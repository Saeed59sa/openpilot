from opendbc.car import CanBusBase
from opendbc.car.common.numpy_fast import clip
from opendbc.car.hyundai.values import HyundaiFlags, HyundaiExFlags
from openpilot.common.params import Params

class CanBus(CanBusBase):
  def __init__(self, CP, fingerprint=None, hda2=None) -> None:
    super().__init__(CP, fingerprint)

    if hda2 is None:
      hda2 = CP.flags & HyundaiFlags.CANFD_HDA2.value if CP is not None else False

    # On the CAN-FD platforms, the LKAS camera is on both A-CAN and E-CAN. HDA2 cars
    # have a different harness than the HDA1 and non-HDA variants in order to split
    # a different bus, since the steering is done by different ECUs.
    self._a, self._e = 1, 0
    if hda2 and not Params().get_bool("HyundaiCameraSCC"):  #배선개조는 무조건 Bus0가 ECAN임.
      self._a, self._e = 0, 1

    self._a += self.offset
    self._e += self.offset
    self._cam = 2 + self.offset

  @property
  def ECAN(self):
    return self._e

  @property
  def ACAN(self):
    return self._a

  @property
  def CAM(self):
    return self._cam


def create_steering_messages_camera_scc(packer, CP, CS, CAN, enabled, lat_active, apply_steer, apply_angle, max_torque, angle_control):
  ret = []

  if angle_control:
    apply_angle = clip(apply_angle, -119, 119)

    values = {
      "LKAS_ANGLE_ACTIVE": 2 if abs(CS.out.steeringAngleDeg) < 110.0 and lat_active else 1,
      "LKAS_ANGLE_CMD": -apply_angle,
      "LKAS_ANGLE_MAX_TORQUE": max_torque if lat_active else 0,
    }
    ret.append(packer.make_can_msg("LFA_ANGLE_MAYBE_CB", CAN.ECAN, values))

    values = CS.lfa_info
    values["LKA_MODE"] = 0
    values["LKA_ICON"] = 2 if enabled else 1
    values["TORQUE_REQUEST"] = -1024  # apply_steer,
    values["LKA_ASSIST"] = 0
    values["STEER_REQ"] = 0  # 1 if lat_active else 0,
    values["HAS_LANE_SAFETY"] = 0  # hide LKAS settings
    values["LKA_ACTIVE"] = 3 if lat_active else 0  # this changes sometimes, 3 seems to indicate engaged
    values["STEER_MODE"] = 0
    values["LKAS_ANGLE_CMD"] = -25.6  # -apply_angle if lat_active else 0,
    values["LKAS_ANGLE_ACTIVE"] = 0  # 2 if lat_active else 1,
    values["LKAS_ANGLE_MAX_TORQUE"] = 0  # max_torque if lat_active else 0,
    values["LKAS_SIGNAL_1"] = 10
    values["NEW_SIGNAL_3"] = 9
    values["LKAS_SIGNAL_2"] = 1
    values["LKAS_SIGNAL_3"] = 1
    values["LKAS_SIGNAL_4"] = 1
    values["LKAS_SIGNAL_5"] = 1
  else:
    values = {}
    values["LKA_MODE"] = 2
    values["LKA_ICON"] = 2 if lat_active else 1
    values["TORQUE_REQUEST"] = apply_steer
    values["STEER_REQ"] = 1 if lat_active else 0
    values["STEER_MODE"] = 0
    values["HAS_LANE_SAFETY"] = 0
    values["LKA_ACTIVE"] = 0  # NEW_SIGNAL_1
    #values["LKA_ASSIST"] = 0
    #values["VALUE104"] = 3 if lat_active else 100
    #values["VALUE82_SET256"] = 0

  ret.append(packer.make_can_msg("LFA", CAN.ECAN, values))
  return ret


def create_steering_messages(packer, CP, CAN, enabled, lat_active, apply_steer, apply_angle, max_torque, angle_control):
  ret = []

  if angle_control:
    values = {
      "LKA_MODE": 0,
      "LKA_ICON": 2 if enabled else 1,
      "TORQUE_REQUEST": 0,  # apply_steer,
      "LKA_ASSIST": 0,
      "STEER_REQ": 0,  # 1 if lat_active else 0,
      "STEER_MODE": 0,
      "HAS_LANE_SAFETY": 0,  # hide LKAS settings
      "LKA_ACTIVE": 3 if lat_active else 0,  # this changes sometimes, 3 seems to indicate engaged
      "LKAS_ANGLE_CMD": -apply_angle if lat_active else 0,
      "LKAS_ANGLE_ACTIVE": 2 if lat_active else 1,
      "LKAS_ANGLE_MAX_TORQUE": max_torque if lat_active else 0,
      "LKAS_SIGNAL_1": 10,
      "NEW_SIGNAL_3": 9,
      "LKAS_SIGNAL_2": 1,
      "LKAS_SIGNAL_3": 1,
      "LKAS_SIGNAL_4": 1,
      "LKAS_SIGNAL_5": 1,
    }
  else:
    values = {
      "LKA_MODE": 2,
      "LKA_ICON": 2 if enabled else 1,
      "TORQUE_REQUEST": apply_steer,
      "LKA_ASSIST": 0,
      "STEER_REQ": 1 if lat_active else 0,
      "STEER_MODE": 0,
      "HAS_LANE_SAFETY": 0,  # hide LKAS settings
    }

  if CP.flags & HyundaiFlags.CANFD_HDA2:
    hda2_lkas_msg = "LKAS_ALT" if CP.flags & HyundaiFlags.CANFD_HDA2_ALT_STEERING else "LKAS"
    if CP.openpilotLongitudinalControl:
      ret.append(packer.make_can_msg("LFA", CAN.ECAN, values))
    if not (CP.flags & HyundaiFlags.CAMERA_SCC.value):
      ret.append(packer.make_can_msg(hda2_lkas_msg, CAN.ACAN, values))
  else:
    ret.append(packer.make_can_msg("LFA", CAN.ECAN, values))

  return ret

def create_suppress_lfa(packer, CAN, hda2_lfa_block_msg, hda2_alt_steering, enabled):
  suppress_msg = "CAM_0x362" if hda2_alt_steering else "CAM_0x2a4"
  msg_bytes = 32 if hda2_alt_steering else 24

  values = {f"BYTE{i}": hda2_lfa_block_msg[f"BYTE{i}"] for i in range(3, msg_bytes) if i != 7}
  values["COUNTER"] = hda2_lfa_block_msg["COUNTER"]
  values["SET_ME_0"] = 0
  values["SET_ME_0_2"] = 0
  values["LEFT_LANE_LINE"] = 0 if enabled else 3
  values["RIGHT_LANE_LINE"] = 0 if enabled else 3
  return packer.make_can_msg(suppress_msg, CAN.ACAN, values)

def create_buttons(packer, CP, CAN, cnt, btn):
  canfd_msg = "CRUISE_BUTTONS_ALT" if CP.flags & HyundaiFlags.CANFD_ALT_BUTTONS else \
              "CRUISE_BUTTONS"
  # If we discover cars use different values for this in the future, echoing back
  # what's read from the car would also work.
  SET_ME_2 = 6
  values = {
    "COUNTER": cnt,
    "CRUISE_BUTTONS": btn,
    "SET_ME_1": 1,
  } | {"SET_ME_2": SET_ME_2} if CP.flags & HyundaiFlags.CANFD_ALT_BUTTONS else {}

  bus = CAN.ECAN if CP.flags & HyundaiFlags.CANFD_HDA2 else CAN.CAM
  return packer.make_can_msg(canfd_msg, bus, values)

def create_acc_cancel(packer, CP, CAN, cruise_info_copy):
  # TODO: why do we copy different values here?
  if CP.flags & HyundaiFlags.CANFD_CAMERA_SCC.value:
    values = {s: cruise_info_copy[s] for s in [
      "COUNTER",
      "CHECKSUM",
      "NEW_SIGNAL_1",
      "MainMode_ACC",
      "ACCMode",
      "ZEROS_9",
      "CRUISE_STANDSTILL",
      "ZEROS_5",
      "DISTANCE_SETTING",
      "VSetDis",
    ]}
  else:
    values = {s: cruise_info_copy[s] for s in [
      "COUNTER",
      "CHECKSUM",
      "ACCMode",
      "VSetDis",
      "CRUISE_STANDSTILL",
    ]}
  values.update({
    "ACCMode": 4,
    "aReqRaw": 0.0,
    "aReqValue": 0.0,
  })
  return packer.make_can_msg("SCC_CONTROL", CAN.ECAN, values)

def create_lfahda_cluster(packer, CAN, enabled):
  values = {
    "HDA_ICON": 1 if enabled else 0,
    "LFA_ICON": 2 if enabled else 0,
  }
  return packer.make_can_msg("LFAHDA_CLUSTER", CAN.ECAN, values)


def create_acc_control_scc2(packer, CAN, enabled, accel_last, accel, stopping, gas_override, set_speed, hud_control, jerk_u, jerk_l, CS):
  jerk = 5
  jn = jerk / 50
  if not enabled or gas_override:
    a_val, a_raw = 0, 0
  else:
    a_raw = accel
    a_val = clip(accel, accel_last - jn, accel_last + jn)

  values = CS.cruise_info
  values["ACCMode"] = 0 if not enabled else (2 if gas_override else 1)
  values["MainMode_ACC"] = 1
  values["StopReq"] = 1 if stopping else 0
  values["aReqValue"] = a_val
  values["aReqRaw"] = a_raw
  values["VSetDis"] = set_speed
  #values["JerkLowerLimit"] = jerk if enabled else 1
  #values["JerkUpperLimit"] = 3.0
  values["JerkLowerLimit"] = jerk_l if enabled else 1
  values["JerkUpperLimit"] = jerk_u
  values["DISTANCE_SETTING"] = hud_control.leadDistanceBars # + 5

  #values["ACC_ObjDist"] = 1
  #values["ObjValid"] = 0
  #values["OBJ_STATUS"] =  2
  values["SET_ME_2"] = 0x4
  #values["SET_ME_3"] = 0x3  # objRelsped와 충돌
  values["SET_ME_TMP_64"] = 0x64
  values["NEW_SIGNAL_3"] = 1 if hud_control.leadVisible else 0 #0  # 1이되면 차선이탈방지 알람이 뜬다고...  => 앞에 차가 있으면, 1또는 2가 됨. 전방두부?
  #values["NEW_SIGNAL_4"] = 2
  values["ZEROS_5"] = 0
  values["NEW_SIGNAL_15_DESIRE_DIST"] = CS.out.vEgo * 1.0 + 4.0
  values["CRUISE_STANDSTILL"] = 1 if stopping and CS.out.aEgo > -0.1 else 0
  values["NEW_SIGNAL_2"] = 0  # 이것이 켜지면 가속을 안하는듯함.
  #values["NEW_SIGNAL_4"] = 0  # signal2와 조합하여.. 앞차와 깜박이등이 인식되는것 같음..

  return packer.make_can_msg("SCC_CONTROL", CAN.ECAN, values)


def create_acc_control(packer, CAN, enabled, accel_last, accel, stopping, gas_override, set_speed, hud_control, jerk_u, jerk_l, CS):
  jerk = 5
  jn = jerk / 50
  if not enabled or gas_override:
    a_val, a_raw = 0, 0
  else:
    a_raw = accel
    a_val = clip(accel, accel_last - jn, accel_last + jn)

  values = {
    "ACCMode": 0 if not enabled else (2 if gas_override else 1),
    "MainMode_ACC": 1,
    "StopReq": 1 if stopping else 0,
    "aReqValue": a_val,
    "aReqRaw": a_raw,
    "VSetDis": set_speed,
    #"JerkLowerLimit": jerk if enabled else 1,
    #"JerkUpperLimit": 3.0,
    "JerkLowerLimit": jerk_l if enabled else 1,
    "JerkUpperLimit": jerk_u,

    "ACC_ObjDist": 1,
    #"ObjValid": 0,
    #"OBJ_STATUS": 2,
    "SET_ME_2": 0x4,
    #"SET_ME_3": 0x3,
    "SET_ME_TMP_64": 0x64,
    "DISTANCE_SETTING": hud_control.leadDistanceBars, # + 5,
    "CRUISE_STANDSTILL": 1 if stopping and CS.out.cruiseState.standstill else 0,
  }

  return packer.make_can_msg("SCC_CONTROL", CAN.ECAN, values)


def create_spas_messages(packer, CAN, frame, left_blink, right_blink):
  ret = []

  values = {
  }
  ret.append(packer.make_can_msg("SPAS1", CAN.ECAN, values))

  blink = 0
  if left_blink:
    blink = 3
  elif right_blink:
    blink = 4
  values = {
    "BLINKER_CONTROL": blink,
  }
  ret.append(packer.make_can_msg("SPAS2", CAN.ECAN, values))

  return ret


def create_fca_warning_light(CP, packer, CAN, frame):
  ret = []
  if CP.flags & HyundaiFlags.CAMERA_SCC.value:
    return ret

  if frame % 2 == 0:
    values = {
      'AEB_SETTING': 0x1,  # show AEB disabled icon
      'SET_ME_2': 0x2,
      'SET_ME_FF': 0xff,
      'SET_ME_FC': 0xfc,
      'SET_ME_9': 0x9,
      #'DATA102': 1,
    }
    ret.append(packer.make_can_msg("ADRV_0x160", CAN.ECAN, values))
  return ret


def create_adrv_messages(CP, packer, CAN, frame, CC, CS, hud_control):
  # messages needed to car happy after disabling
  # the ADAS Driving ECU to do longitudinal control

  ret = []
  values = {}
  if CP.flags & HyundaiFlags.CAMERA_SCC.value:
    if frame % 5 == 0:
      if CP.exFlags & HyundaiExFlags.CANFD_161.value:
        if CS.adrv_info_161 is not None:
          main_enabled = CS.out.cruiseState.available
          cruise_enabled = CC.enabled
          values = CS.adrv_info_161
          #print("adrv_info_161 = ", CS.adrv_info_161)
          values["vSetDis"] = int(hud_control.setSpeed * 3.6 + 0.5)
          values["GAP_DIST_SET"] = hud_control.leadDistanceBars

          #values["SETSPEED"] = 3 if cruise_enabled else 1
          values["SETSPEED_HUD"] = 2 if cruise_enabled else 1
          #values["SETSPEED_SPEED"] = 25 if (s := round(CS.out.vCruiseCluster * CV.KPH_TO_MPH)) > 100 else s

          values["DISTANCE_SPACING"] = 1 if cruise_enabled else 0
          depart = hud_control.leadRelSpeed > -0.2 and cruise_enabled
          values["DISTANCE_LEAD"] = 1 if depart else 1 if cruise_enabled and hud_control.leadVisible else 0
          values["DISTANCE_CAR"] = 2 if cruise_enabled else 1

          """
          # DISTANCE
          if 1 <= hud_control.leadDistanceBars <= 3:
            values["DISTANCE"] = hud_control.leadDistanceBars
            values["DISTANCE_SPACING"] = 1 if cruise_enabled else 0
            values["DISTANCE_LEAD"] = 2 if cruise_enabled and hud_control.leadVisible else 1 if cruise_enabled else 0
            values["DISTANCE_CAR"] = 2 if cruise_enabled else 1
            values["ALERTS_3"] = hud_control.leadDistanceBars + 6
          else:
            values["DISTANCE"] = 0
            values["DISTANCE_SPACING"] = 0
            values["DISTANCE_LEAD"] = 0
            values["DISTANCE_CAR"] = 0
          """

          # BACKGROUND
          values["BACKGROUND"] = 1 if cruise_enabled else 7

          values["LFA_ICON"] = 1 if CC.latActive else 0
          values["LKA_ICON"] = 4 if CC.latActive else 3
          values["NAV_ICON"] = 2 if CC.latActive else 0
          values["HDA_ICON"] = 2 if main_enabled else 0

          if values["MUTE"] == 3:
            values["MUTE"] = 0
          values["ALERTS_1"] = 0
          values["ALERTS_2"] = 0
          values["ALERTS_3"] = 0
          values["ALERTS_4"] = 0
          values["ALERTS_5"] = 0
          values["SOUNDS_2"] = 0
          values["FCA_ALT_ICON"] = 0
          values["CENTERLINE"] = 1 if CC.latActive else 0
          curvature = {
            i: (31 if i == -1 else 13 - abs(i + 15)) if i < 0 else 15 + i
            for i in range(-15, 16)
          }
          values["LANELINE_CURVATURE"] = curvature.get(max(-15, min(int(CS.out.steeringAngleDeg / 3), 15)), 14) if main_enabled else 15
          if hud_control.leftLaneDepart:
            values["LANELINE_LEFT"] = 4 if (frame // 50) % 2 == 0 else 1
          else:
            values["LANELINE_LEFT"] = 2 if hud_control.leftLaneVisible else 0
          if hud_control.rightLaneDepart:
            values["LANELINE_RIGHT"] = 4 if (frame // 50) % 2 == 0 else 1
          else:
            values["LANELINE_RIGHT"] =  2 if hud_control.rightLaneVisible else 0
          values["LANELINE_LEFT_POSITION"] = 15
          values["LANELINE_RIGHT_POSITION"] = 15

          if main_enabled or True:
            speed_below_threshold = CS.out.vEgo < 8.94
            values["LCA_LEFT_ICON"] = 0 if CS.out.leftBlindspot or speed_below_threshold else 2 if CS.out.leftBlinker else 1
            values["LCA_RIGHT_ICON"] = 0 if CS.out.rightBlindspot or speed_below_threshold else 2 if CS.out.rightBlinker else 1
            values["LCA_LEFT_ARROW"] = 2 if CS.out.leftBlinker else 0
            values["LCA_RIGHT_ARROW"] = 2 if CS.out.rightBlinker else 0

          ret.append(packer.make_can_msg("ADRV_0x161", CAN.ECAN, values))
        else:
          print("no adrv_info_161")

      if CS.adrv_info_200 is not None:
        values = CS.adrv_info_200
        values["TauGapSet"] = hud_control.leadDistanceBars
        ret.append(packer.make_can_msg("ADRV_0x200", CAN.ECAN, values))

      if CS.adrv_info_1ea is not None:
        values = CS.adrv_info_1ea
        values["HDA_MODE1"] = 8
        values["HDA_MODE2"] = 1
        ret.append(packer.make_can_msg("ADRV_0x1ea", CAN.ECAN, values))

      if CS.adrv_info_160 is not None:
        values = CS.adrv_info_160
        values["NEW_SIGNAL_1"] = 0 # steer_temp관련없음, 계기판에러
        values["SET_ME_9"] = 17 # steer_temp관련없음, 계기판에러
        values["SET_ME_2"] = 0   #커멘트해도 steer_temp에러남, 2값은 콤마에서 찾은거니...
        values["DATA102"] = 0  # steer_temp관련없음
        ret.append(packer.make_can_msg("ADRV_0x160", CAN.ECAN, values))

      if CS.adrv_info_162 is not None:
        values = CS.adrv_info_162
        values["FAULT_FSS"] = 0
        values["FAULT_LSS"] = 0
        values["FAULT_SLA"] = 0
        values["FAULT_LFA"] = 0
        values["FAULT_HDA"] = 0
        values["FAULT_LCA"] = 0
        values["FAULT_HDP"] = 0
        values["FAULT_SCC"] = 0
        values["FAULT_DAS"] = 0
        ret.append(packer.make_can_msg("ADRV_0x162", CAN.ECAN, values))

    """
    if frame % 20 == 0:
      if CS.hda_info_4a3 is not None:
        values = CS.hda_info_4a3
        # SIGNAL_4: 7, SIGNAL_0: 0 으로 해도 .. 옆두부는 나오기도 함.. 아오5
        values["SIGNAL_4"] = 10 if CC.enabled else 0   # 0, 5(고속도로진입), 10(고속도로), 7,5(국도에서 간혹), 0,10(카니발)      , 5(고속도로진입,EV6), 11(고속도로,EV6)
        values["SIGNAL_0"] = 5 if CC.enabled else 0  # 0, 2(고속도로진입), 1(고속도로),                      5(카니발은 항상)  , 2(고속도로진입,EV6), 1(고속도로,EV6)
        values["NEW_SIGNAL_1"] = 4
        values["NEW_SIGNAL_2"] = 0
        values["NEW_SIGNAL_3"] = 154
        values["NEW_SIGNAL_4"] = 9
        values["NEW_SIGNAL_5"] = 0
        values["NEW_SIGNAL_6"] = 256
        values["NEW_SIGNAL_7"] = 0
        ret.append(packer.make_can_msg("HDA_INFO_4A3", CAN.CAM, values))
    if frame % 10 == 0:
      if CS.new_msg_4b4 is not None: #G80 HDA2개조차량은 안나옴...
        values = CS.new_msg_4b4
        values["NEW_SIGNAL_1"] = 8
        values["NEW_SIGNAL_3"] = (frame / 100) % 10
        values["NEW_SIGNAL_4"] = 146
        values["NEW_SIGNAL_5"] = 68
        values["NEW_SIGNAL_6"] = 76
        ret.append(packer.make_can_msg("NEW_MSG_4B4", CAN.CAM, values))
    """
    return ret
  else:
    values = {}

    ret.extend(create_fca_warning_light(CP, packer, CAN, frame))
    if frame % 5 == 0:
      values = {
        'HDA_MODE1': 0x8,
        'HDA_MODE2': 0x1,
        #'SET_ME_1C': 0x1c,
        'SET_ME_FF': 0xff,
        #'SET_ME_TMP_F': 0xf,
        #'SET_ME_TMP_F_2': 0xf,
        #'DATA26': 1,  #1
        #'DATA32': 5,  #5
      }
      ret.append(packer.make_can_msg("ADRV_0x1ea", CAN.ECAN, values))

      values = {
        'SET_ME_E1': 0xe1,
        #'SET_ME_3A': 0x3a,
        'TauGapSet' : 1,
        'NEW_SIGNAL_2': 3,
      }
      ret.append(packer.make_can_msg("ADRV_0x200", CAN.ECAN, values))

    if frame % 20 == 0:
      values = {
        'SET_ME_15': 0x15,
      }
      ret.append(packer.make_can_msg("ADRV_0x345", CAN.ECAN, values))

    if frame % 100 == 0:
      values = {
        'SET_ME_22': 0x22,
        'SET_ME_41': 0x41,
      }
      ret.append(packer.make_can_msg("ADRV_0x1da", CAN.ECAN, values))

    return ret
