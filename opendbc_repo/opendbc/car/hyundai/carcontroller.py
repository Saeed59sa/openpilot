from opendbc.can.packer import CANPacker
from opendbc.car import (Bus, DT_CTRL, apply_driver_steer_torque_limits, common_fault_avoidance, make_tester_present_msg, structs,
                                     apply_std_steer_angle_limits)
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.common.numpy_fast import clip, interp
from opendbc.car.hyundai import hyundaicanfd, hyundaican
from opendbc.car.hyundai.carstate import CarState
from opendbc.car.hyundai.hyundaicanfd import CanBus
from opendbc.car.hyundai.values import HyundaiFlags, Buttons, CarControllerParams, CAR, CAN_GEARS
from opendbc.car.interfaces import CarControllerBase, ACCEL_MIN, ACCEL_MAX

from openpilot.selfdrive.controls.neokii.navi_controller import SpeedLimiter
from openpilot.common.params import Params

VisualAlert = structs.CarControl.HUDControl.VisualAlert
LongCtrlState = structs.CarControl.Actuators.LongControlState

# EPS faults if you apply torque while the steering angle is above 90 degrees for more than 1 second
# All slightly below EPS thresholds to avoid fault
MAX_ANGLE = 85
MAX_ANGLE_FRAMES = 89
MAX_ANGLE_CONSECUTIVE_FRAMES = 2


def process_hud_alert(enabled, fingerprint, hud_control):
  sys_warning = (hud_control.visualAlert in (VisualAlert.steerRequired, VisualAlert.ldw))

  # initialize to no line visible
  # TODO: this is not accurate for all cars
  sys_state = 1
  if hud_control.leftLaneVisible and hud_control.rightLaneVisible or sys_warning:  # HUD alert only display when LKAS status is active
    sys_state = 3 if enabled or sys_warning else 4
  elif hud_control.leftLaneVisible:
    sys_state = 5
  elif hud_control.rightLaneVisible:
    sys_state = 6

  # initialize to no warnings
  left_lane_warning = 0
  right_lane_warning = 0
  if hud_control.leftLaneDepart:
    left_lane_warning = 1 if fingerprint in (CAR.GENESIS_G90, CAR.GENESIS_G80) else 2
  if hud_control.rightLaneDepart:
    right_lane_warning = 1 if fingerprint in (CAR.GENESIS_G90, CAR.GENESIS_G80) else 2

  return sys_warning, sys_state, left_lane_warning, right_lane_warning


class CarController(CarControllerBase):
  def __init__(self, dbc_names, CP):
    super().__init__(dbc_names, CP)
    self.CAN = CanBus(CP)
    self.params = CarControllerParams(CP)
    self.packer = CANPacker(dbc_names[Bus.pt])
    self.angle_limit_counter = 0

    self.accel_last = 0
    self.apply_steer_last = 0
    self.car_fingerprint = CP.carFingerprint
    self.last_button_frame = 0
    self.apply_angle_last = 0
    self.lkas_max_torque = 0
    self.torque_reducer = 0
    self.turningSignalTimer = 0

    self.hyundai_jerk = HyundaiJerk()

  def update(self, CC, CS, now_nanos):
    actuators = CC.actuators
    hud_control = CC.hudControl

    # steering torque
    new_steer = int(round(actuators.steer * self.params.STEER_MAX))
    apply_steer = apply_driver_steer_torque_limits(new_steer, self.apply_steer_last, CS.out.steeringTorque, self.params)

    # >90 degree steering fault prevention
    self.angle_limit_counter, apply_steer_req = common_fault_avoidance(abs(CS.out.steeringAngleDeg) >= MAX_ANGLE, CC.latActive,
                                                                       self.angle_limit_counter, MAX_ANGLE_FRAMES,
                                                                       MAX_ANGLE_CONSECUTIVE_FRAMES)

    apply_angle = apply_std_steer_angle_limits(actuators.steeringAngleDeg, self.apply_angle_last, CS.out.vEgoRaw, self.params)

    if abs(CS.out.steeringTorqueEps) >= 100.0: # carrot. fault avoidance, test code
      apply_angle = CS.out.steeringAngleDeg

    # Constants
    TORQUE_THRESHOLD = 200  # Driver-applied torque threshold (absolute value)
    TORQUE_MAX = 240  # Maximum allowable torque for LKAS
    REDUCER_MIN = 30  # Minimum torque reducer value
    REDUCER_MAX = 150  # Maximum torque reducer value

    # Clamp vEgoCluster between 0 and 20
    vEgo_safe = max(0, min(CS.out.vEgoCluster, 20))

    # Interpolate a weight based on vehicle speed (vEgoCluster in m/s)
    speed_weight = interp(vEgo_safe, [0, 5, 10, 20], [0.2, 0.3, 0.5, 1.0])

    # Adjust the torque reducer directly in this block
    if abs(CS.out.steeringTorque) > TORQUE_THRESHOLD:
      # Driver is applying torque: reduce the reducer value to fight less
      self.torque_reducer = max(REDUCER_MIN, self.torque_reducer - 1)
    else:
      # Driver is not applying torque: gradually restore the reducer value
      self.torque_reducer = min(REDUCER_MAX, self.torque_reducer + 1)

    # Calculate max torque to apply
    reducer_ratio = self.torque_reducer / REDUCER_MAX
    self.lkas_max_torque = int(round(TORQUE_MAX * speed_weight * reducer_ratio))

    # Disable steering while turning blinker on and speed below 60 kph
    if CS.out.leftBlinker or CS.out.rightBlinker:
      self.turningSignalTimer = 0.5 / DT_CTRL  # Disable for 0.5 Seconds after blinker turned off
    if self.turningSignalTimer > 0:
      self.turningSignalTimer -= 1

    if not CC.latActive:
      apply_angle = CS.out.steeringAngleDeg
      apply_steer = 0
      self.lkas_max_torque = 0

    self.apply_angle_last = apply_angle
    self.apply_steer_last = apply_steer

    # Hold torque with induced temporary fault when cutting the actuation bit
    torque_fault = CC.latActive and not apply_steer_req

    # accel + longitudinal
    accel = clip(actuators.accel, ACCEL_MIN, ACCEL_MAX)
    stopping = actuators.longControlState == LongCtrlState.stopping
    set_speed_in_units = hud_control.setSpeed * (CV.MS_TO_KPH if CS.is_metric else CV.MS_TO_MPH)

    # HUD messages
    sys_warning, sys_state, left_lane_warning, right_lane_warning = process_hud_alert(CC.enabled, self.car_fingerprint, hud_control)

    can_sends = []

    # *** common hyundai stuff ***

    # tester present - w/ no response (keeps relevant ECU disabled)
    if self.frame % 100 == 0 and not (self.CP.flags & HyundaiFlags.CANFD_CAMERA_SCC) and self.CP.openpilotLongitudinalControl:
      # for longitudinal control, either radar or ADAS driving ECU
      addr, bus = 0x7d0, self.CAN.ECAN if self.CP.flags & HyundaiFlags.CANFD else 0
      if self.CP.flags & HyundaiFlags.CANFD_HDA2.value:
        addr, bus = 0x730, self.CAN.ECAN
      can_sends.append(make_tester_present_msg(addr, bus, suppress_response=True))

      # for blinkers
      if self.CP.flags & HyundaiFlags.ENABLE_BLINKERS:
        can_sends.append(make_tester_present_msg(0x7b1, self.CAN.ECAN, suppress_response=True))

    camera_scc = self.CP.flags & HyundaiFlags.CAMERA_SCC
    # CAN-FD platforms
    if self.CP.flags & HyundaiFlags.CANFD:
      hda2 = self.CP.flags & HyundaiFlags.CANFD_HDA2 or Params().get_bool("IsHda2")
      hda2_long = hda2 and self.CP.openpilotLongitudinalControl

      # steering control
      angle_control = self.CP.flags & HyundaiFlags.ANGLE_CONTROL

      can_sends.extend(hyundaicanfd.create_steering_messages(self.packer, self.CP, CC, CS, self.CAN, self.lkas_max_torque,
                                                             apply_steer_req, apply_steer, apply_angle, angle_control))

      # prevent LFA from activating on HDA2 by sending "no lane lines detected" to ADAS ECU
      if self.frame % 5 == 0 and (hda2 and not camera_scc):
        can_sends.append(hyundaicanfd.create_suppress_lfa(self.packer, self.CP, CC, CS, self.CAN))

      # LFA and HDA icons
      if self.frame % 5 == 0 and (not hda2 or hda2_long):
        can_sends.append(hyundaicanfd.create_lfahda_cluster(self.packer, CC, self.CAN))

      # blinkers
      if hda2 and self.CP.flags & HyundaiFlags.ENABLE_BLINKERS:
        can_sends.extend(hyundaicanfd.create_spas_messages(self.packer, CC, self.CAN))

      if self.CP.openpilotLongitudinalControl:
        self.hyundai_jerk.make_jerk(self.CP, CS, accel, actuators, hud_control)

        if hda2:
          can_sends.extend(hyundaicanfd.create_adrv_messages(self.packer, self.CP, CC, CS, self.CAN, self.frame,
                                                             hud_control, apply_angle, left_lane_warning, right_lane_warning))
        else:
          can_sends.extend(hyundaicanfd.create_fca_warning_light(self.packer, self.CP, self.CAN, self.frame))
        if self.frame % 2 == 0:
          can_sends.append(hyundaicanfd.create_acc_control(self.packer, self.CP, CC, CS, self.CAN, self.accel_last,
                                                           accel, stopping, set_speed_in_units, hud_control,
                                                           self.hyundai_jerk.jerk_u, self.hyundai_jerk.jerk_l))
          self.accel_last = accel
      else:
        # button presses
        can_sends.extend(self.create_button_messages(CC, CS, use_clu11=False))
    else:
      send_lfa = self.CP.flags & HyundaiFlags.SEND_LFA.value
      use_fca = self.CP.flags & HyundaiFlags.USE_FCA.value

      can_sends.append(hyundaican.create_lkas11(self.packer, self.frame, self.CP, apply_steer, apply_steer_req, torque_fault, sys_warning, sys_state, CC.enabled,
                                                hud_control.leftLaneVisible, hud_control.rightLaneVisible, left_lane_warning, right_lane_warning, CS.lkas11))

      if not self.CP.openpilotLongitudinalControl:
        can_sends.extend(self.create_button_messages(CC, CS, use_clu11=True))

      if self.frame % 2 == 0 and self.CP.openpilotLongitudinalControl:
        # TODO: unclear if this is needed
        jerk = 3.0 if actuators.longControlState == LongCtrlState.pid else 1.0
        can_sends.extend(hyundaican.create_scc_commands(self.packer, accel, jerk, int(self.frame / 2),
                                                        hud_control, set_speed_in_units, stopping, CC, CS, use_fca))

      # 20 Hz LFA MFA message
      if self.frame % 5 == 0 and send_lfa:
        can_sends.append(hyundaican.create_lfahda_mfc(self.packer, CC.enabled, SpeedLimiter.instance().get_active()))

      # 5 Hz ACC options
      if self.frame % 20 == 0 and self.CP.openpilotLongitudinalControl and not camera_scc:
        can_sends.extend(hyundaican.create_acc_opt(self.packer, use_fca))
      elif CS.scc13 is not None:
        can_sends.append(hyundaican.create_acc_opt_none(self.packer, CS))

      if self.CP.carFingerprint in CAN_GEARS["send_mdps12"]:  # send mdps12 to LKAS to prevent LKAS error
        can_sends.append(hyundaican.create_mdps12(self.packer, self.frame, CS.mdps12))

      # 2 Hz front radar options
      if self.frame % 50 == 0 and self.CP.openpilotLongitudinalControl and not camera_scc:
        can_sends.append(hyundaican.create_frt_radar_opt(self.packer))

      # car signal status
      if self.frame % 1000 == 0:
        print(f'scc11 = {bool(CS.scc11)}  scc12 = {bool(CS.scc12)}  scc13 = {bool(CS.scc13)}  scc14 = {bool(CS.scc14)}  mdps12 = {bool(CS.mdps12)}')

    new_actuators = actuators.as_builder()
    new_actuators.steer = apply_steer / self.params.STEER_MAX
    new_actuators.steerOutputCan = apply_steer
    new_actuators.steeringAngleDeg = apply_angle
    new_actuators.accel = accel

    self.frame += 1
    return new_actuators, can_sends

  def create_button_messages(self, CC: structs.CarControl, CS: CarState, use_clu11: bool):
    can_sends = []
    if use_clu11:
      if CC.cruiseControl.cancel:
        can_sends.append(hyundaican.create_clu11(self.packer, self.frame, self.CP, Buttons.CANCEL, CS.clu11))
      elif CC.cruiseControl.resume:
        # send resume at a max freq of 10Hz
        if (self.frame - self.last_button_frame) * DT_CTRL > 0.1:
          # send 25 messages at a time to increases the likelihood of resume being accepted
          can_sends.extend([hyundaican.create_clu11(self.packer, self.frame, self.CP, Buttons.RES_ACCEL, CS.clu11)] * 25)
          if (self.frame - self.last_button_frame) * DT_CTRL >= 0.15:
            self.last_button_frame = self.frame
    else:
      if (self.frame - self.last_button_frame) * DT_CTRL > 0.25:
        # cruise cancel
        if CC.cruiseControl.cancel:
          if self.CP.flags & HyundaiFlags.CANFD_ALT_BUTTONS:
            can_sends.append(hyundaicanfd.create_acc_cancel(self.packer, self.CP, CS, self.CAN))
            self.last_button_frame = self.frame
          else:
            for _ in range(20):
              can_sends.append(hyundaicanfd.create_buttons(self.packer, self.CP, self.CAN, CS.buttons_counter+1, Buttons.CANCEL))
            self.last_button_frame = self.frame

        # cruise standstill resume
        elif CC.cruiseControl.resume:
          for _ in range(20):
            can_sends.append(hyundaicanfd.create_buttons(self.packer, self.CP, self.CAN, CS.buttons_counter+1, Buttons.RES_ACCEL))
          self.last_button_frame = self.frame

    return can_sends


class HyundaiJerk:
  def __init__(self):
    self.jerk = 0.0
    self.jerk_u = self.jerk_l = 0.0
    self.cb_upper = self.cb_lower = 0.0
    self.jerk_u_min = 0.5

  def make_jerk(self, CP, CS, accel, actuators, hud_control):
    if actuators.longControlState == LongCtrlState.stopping:
      self.jerk = self.jerk_u_min / 2 - CS.out.aEgo
    else:
      jerk = actuators.jerk if actuators.longControlState == LongCtrlState.pid else 0.0
      #a_error = actuators.aTargetNow - CS.out.aEgo
      self.jerk = jerk #+ a_error

    jerk_max_l = 5.0
    jerk_max_u = jerk_max_l
    if actuators.longControlState == LongCtrlState.off:
      self.jerk_u = jerk_max_u
      self.jerk_l = jerk_max_l
      self.cb_upper = self.cb_lower = 0.0
    else:
      if CP.flags & HyundaiFlags.CANFD:
        self.jerk_u = min(max(self.jerk_u_min, self.jerk * 2.0), jerk_max_u)
        self.jerk_l = min(max(1.0, -self.jerk * 4.0), jerk_max_l)
        self.cb_upper = self.cb_lower = 0.0
      else:
        self.jerk_u = min(max(self.jerk_u_min, self.jerk * 2.0), jerk_max_u)
        self.jerk_l = min(max(1.0, -self.jerk * 2.0), jerk_max_l)
        self.cb_upper = clip(0.9 + accel * 0.2, 0, 1.2)
        self.cb_lower = clip(0.8 + accel * 0.2, 0, 1.2)
