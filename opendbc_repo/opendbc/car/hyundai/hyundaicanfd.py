from opendbc.car import CanBusBase
from opendbc.car.common.numpy_fast import clip
from opendbc.car.hyundai.values import HyundaiFlags, HyundaiExFlags
from openpilot.common.params import Params
from openpilot.selfdrive.controls.neokii.navi_controller import SpeedLimiter

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
  values["SET_ME_2"] = 4
  #values["SET_ME_3"] = 3  # objRelsped와 충돌
  values["SET_ME_TMP_64"] = 100
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
    "SET_ME_2": 4,
    #"SET_ME_3": 3,
    "SET_ME_TMP_64": 100,
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
      'AEB_SETTING': 1,  # show AEB disabled icon
      'SET_ME_2': 2,
      'SET_ME_FF': 255,
      'SET_ME_FC': 252,
      'SET_ME_9': 9,
      #'DATA102': 1,
    }
    ret.append(packer.make_can_msg("ADRV_0x160", CAN.ECAN, values))
  return ret


def create_adrv_messages(CP, packer, CAN, frame, CC, CS, hud_control, disp_angle, left_lane_warning, right_lane_warning):
  # messages needed to car happy after disabling
  # the ADAS Driving ECU to do longitudinal control

  ret = []
  values = {}
  if CP.flags & HyundaiFlags.CAMERA_SCC.value:
    if frame % 5 == 0:
      if CP.exFlags & HyundaiExFlags.CCNC.value:
        if CS.adrv_info_161 is not None:
          main_enabled = CS.out.cruiseState.available
          cruise_enabled = CC.enabled
          values = CS.adrv_info_161

          values["vSetDis"] = int(hud_control.setSpeed * 3.6 + 0.5)
          values["GAP_DIST_SET"] = hud_control.leadDistanceBars

          #values["SETSPEED"] = 3 if cruise_enabled else 1
          #values["SETSPEED_HUD"] = 2 if cruise_enabled else 1
          #values["SETSPEED_SPEED"] = 25 if (s := round(CS.out.vCruiseCluster * CV.KPH_TO_MPH)) > 100 else s

          values["TARGET"] = 1 if cruise_enabled else 0
          values["TARGET_POSITION"] = int(hud_control.leadDistance)

          values["DISTANCE_SPACING"] = 1 if cruise_enabled else 0
          values["DISTANCE_LEAD"] = 1 if cruise_enabled and (hud_control.leadRelSpeed > -0.2 or hud_control.leadVisible) else 0
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

          values["BACKGROUND"] = 1 if cruise_enabled else 7
          # BACKGROUND 0 "HIDDEN" 1 "BLUE" 3 "ORANGE" 4 "FLASHING ORANGE" 6 "FLASHING RED" 7 "GRAY";

          nav_active = SpeedLimiter.instance().get_active()
          values["LFA_ICON"] = 2 if CC.latActive else 1
          values["LKA_ICON"] = 4 if CC.latActive else 3
          values["NAV_ICON"] = 2 if nav_active else 0
          values["HDA_ICON"] = 2 if CS.out.accBtn else 0
          #values["FCA_ICON"] = 0
          #values["FCA_ALT_ICON"] = 0
          #values["FCA_IMAGE"] = 0
          # LFA_ICON 0 "HIDDEN" 1 "GRAY" 2 "GREEN" 3 "WHITE" 5 "CYAN";
          # LKA_ICON 0 "HIDDEN" 1 "ORANGE" 3 "GRAY" 4 "GREEN";
          # NAV_ICON 0 "HIDDEN" 1 "GRAY" 2 "GREEN" 4 "WHITE";
          # HDA_ICON 0 "HIDDEN" 1 "GRAY" 2 "GREEN" 3 "WHITE" 5 "CYAN HDP";
          # FCA_ICON 0 "HIDDEN" 1 "ORANGE" 2 "RED";
          # FCA_ALT_ICON 0 "HIDDEN" 1 "ORANGE" 3 "RED";
          # FCA_IMAGE 0 "HIDDEN" 2 "VISIBLE";

          #values["ALERTS_1"] = 0
          # ALERTS_1 0 "HIDDEN" 1 "WARNING_ONLY_CAR_CENTER" 2 "WARNING_ONLY_CAR_LEFT" 3 "WARNING_ONLY_CAR_RIGHT" 4 "WARNING_ONLY_LEFT" 5 "WARNING_ONLY_RIGHT" 11 "EMERGENCY_BRAKING_CAR_CENTER" 12 "EMERGENCY_BRAKING_CAR_LEFT" 13 "EMERGENCY_BRAKING_CAR_RIGHT" 14 "EMERGENCY_BRAKING_LEFT" 15 "EMERGENCY_BRAKING_RIGHT" 21 "EMERGENCY_STEERING_CAR_LEFT" 22 "EMERGENCY_STEERING_CAR_RIGHT" 23 "EMERGENCY_STEERING_CAR_LEFT_AWAY" 24 "EMERGENCY_STEERING_CAR_RIGHT_AWAY" 25 "EMERGENCY_STEERING_REAR_LEFT" 26 "EMERGENCY_STEERING_REAR_RIGHT" 33 "DRIVE_CAREFULLY";
          if values["ALERTS_2"] == 5:
            values["ALERTS_2"] = 0
            values["SOUNDS_2"] = 0
            values["DAW_ICON"] = 0
          # ALERTS_2 0 "HIDDEN" 1 "KEEP_HANDS_ON_STEERING_WHEEL" 2 "KEEP_HANDS_ON_STEERING_WHEEL_RED" 3 "LANE_FOLLOWING_ASSIST_DEACTIVATED" 4 "HIGHWAY_DRIVING_ASSIST_DEACTIVATED" 5 "CONSIDER_TAKING_A_BREAK" 6 "PRESS_OK_BUTTON_TO_ENABLE_LANE_CHANGE_ASSIST" 7 "COLLISION_RISK_VEHICLE_TAKING_EMERGENCY_CONTROL" 8 "TAKE_CONTROL_OF_THE_VEHICLE_IMMEDIATELY_VEHICLE_IS_STOPPING" 9 "TAKE_CONTROL_OF_THE_VEHICLE_IMMEDIATELY" 11 "HIGHWAY_DRIVING_PILOT_SYSTEM_DEACTIVATED_AUDIBLE" 12 "KEEP_YOUR_EYES_ON_THE_ROAD" 13 "HIGHWAY_DRIVING_PILOT_CONDITIONS_NOT_MET_AUDIBLE" 14 "COLLISION_RISK_VEHICLE_TAKING_EMERGENCY_CONTROL" 15 "SET_THE_WIPER_AND_LIGHT_CONTROLS_TO_AUTO" 16 "BE_PREPARED_TO_TAKE_CONTROL_OF_THE_VEHICLE_AT_ANY_TIME" 21 "TAKE_CONTROL_OF_THE_VEHICLE_IMMEDIATELY_VEHICLE_IS_STOPPING" 10 "TAKE_CONTROL_OF_THE_VEHICLE_IMMEDIATELY";
          # DAW_ICON 0 "HIDDEN" 1 "ORANGE";

          if values["ALERTS_3"] == 17:
            values["ALERTS_3"] = 0
          # ALERTS_3 0 "HIDDEN" 1 "AUTOMATICALLY_ADJUSTING_TO_THE_POSTED_SPEED_LIMIT" 2 "SET_SPEED_CHANGED" 3 "AUTOMATICALLY_ADJUSTING_TO_THE_POSTED_SPEED_LIMIT" 4 "SET_SPEED_CHANGED" 7 "DISTANCE_1" 8 "DISTANCE_2" 9 "DISTANCE_3" 10 "DISTANCE_4" 17 "DRIVE_CAREFULLY" 18 "CHECK_SURROUNDINGS" 19 "CONDITIONS_NOT_MET" 20 "LANES_NOT_DETECTED" 21 "CURVE_TOO_SHARP" 22 "LANE_TOO_NARROW" 23 "ROAD_TYPE_NOT_SUPPORTED" 24 "UNAVAILABLE_WITH_HAZARD_LIGHTS_ON" 25 "VEHICLE_SPEED_IS_TOO_LOW" 26 "KEEP_HANDS_ON_STEERING_WHEEL" 27 "LANE_TYPE_NOT_SUPPORTED" 28 "LANE_ASSIST_CANCELED_STEERING_INPUT_DETECTED";
          #values["ALERTS_4"] = 0
          # ALERTS_4 0 "HIDDEN" 1 "TAKE_FOOT_OFF_THE_ACCELERATOR_PEDAL" 2 "TAKE_FOOT_OFF_THE_BRAKE_PEDAL" 3 "UNAVAILABLE_WHILE_HIGHWAY_DRIVING_PILOT_SYSTEM_IS_ACTIVE" 4 "TO_EXIT_HDP_GRASP_THE_STEERING_WHEEL_THEN_PRESS_AND_HOLD_THE_HDP_BUTTON" 5 "ACCELERATOR_PEDAL_OPERATION_LIMITED_FOR_SAFETY" 6 "TURN_OFF_HAZARD_WARNING_LIGHTS_AND_TURN_SIGNAL" 7 "KEEP_THE_DRIVERS_SEAT_IN_A_SAFE_DRIVING_POSITION" 16 "SET_SPEED_CHANGED" 17 "ACTIVATING_WINDSHIELD_DEFOG_TO_MAINTAIN_THE_DRIVERS_VIEW" 18 "SET_THE_WIPER_AND_LIGHT_CONTROLS_TO_AUTO" 19 "VEHICLE_SPEED_REDUCED_FOR_SAFETY_MERGING_LANES_AHEAD" 20 "SPEED_REDUCED_FOR_SAFETY_CONSTRUCTION_ZONE_DETECTED" 21 "VEHICLE_SPEED_LIMITED_SENSOR_DETECTION_RANGE_LIMITED" 22 "PREPARE_TO_TAKE_CONTROL_UNSUPPORTED_ROAD_TYPE_AHEAD" 23 "PREPARE_TO_TAKE_CONTROL_ENTRANCE_AND_EXIT_RAMPS_AHEAD" 24 "PREPARE_TO_TAKE_CONTROL_TOLLGATE_AHEAD" 25 "PREPARE_TO_TAKE_CONTROL_ROAD_EVENT_AHEAD" 26 "CLEARING_PATH_FOR_EMERGENCY_VEHICLE" 27 "VEHICLE_IS_TOO_SLOW_COMPARED_TO_TRAFFIC_FLOW" 28 "AFTER_SUNSET_HDP_IS_AVAILABLE_IN_AN_INSIDE_LANE_BEHIND_A_LEADING_VEHICLE" 29 "VEHICLE_SPEED_LIMITED_MERGING_LANES_AHEAD" 30 "VEHICLE_SPEED_LIMITED_CONSTRUCTION_ZONE_DETECTED" 31 "VEHICLE_SPEED_TEMPORARILY_LIMITED_FOR_SAFETY" 32 "PRESS_AND_HOLD_THE_BUTTON_TO_ACTIVATE_HIGHWAY_DRIVING_PILOT" 40 "HIGHWAY_DRIVING_PILOT_SYSTEM_IS_AVAILABLE" 64 "RESTART_VEHICLE_AFTER_EMERGENCY_STOP" 65 "CONNECTED_SERVICES_UNAVAILABLE" 66 "AVAILABLE_AFTER_VEHICLE_SOFTWARE_IS_UPDATED" 67 "ROAD_TYPE_NOT_SUPPORTED" 68 "ONLY_AVAILABLE_WHILE_DRIVING_ON_HIGHWAY_LANES" 69 "UNAVAILABLE_WHILE_OTHER_WARNINGS_ARE_ACTIVE" 70 "CANNOT_ACTIVATE_AT_ENTRANCE_EXIT_RAMPS" 71 "LANE_UNSUPPORTED" 72 "NOT_AVAILABLE_IN_THIS_COUNTRY" 79 "CHECKING_THE_DETECTION_RANGE_OF_THE_SENSOR" 80 "SHIFT_TO_D" 81 "ENGINE_STOPPED_BY_AUTO_STOP" 82 "INCREASE_DISTANCE_FROM_VEHICLE_AHEAD" 83 "VEHICLE_SPEED_IS_TOO_HIGH" 84 "CENTER_VEHICLE_IN_THE_LANE" 85 "PARKING_ASSIST_IS_ACTIVE" 86 "ESC_ACTIVIATION_REQUIRED" 87 "UNFOLD_SIDE_VIEW_MIRRORS" 88 "UNAVAILABLE_IN_THE_OUTER_LANE_AFTER_SUNSET" 89 "VEHICLE_SPEED_LIMITED_AFTER_SUNSET_FOR_SAFETY" 90 "LEADING_VEHICLE_NOT_DETECTED" 104 "AGGRESSIVE_BRAKING_OR_STEERING_DETECTED" 110 "SENSOR_AUTO_CALIBRATION_IN_PROGRESS_THIS_MAY_TAKE_SEVERAL_MINUTES" 111 "HIGHWAY_DRIVING_PILOT_WILL_BE_AVAILABLE_SHORTLY" 112 "IF_STEERING_WHEEL_IS_USED_HDP_WILL_BE_DEACTIVATED" 120 "IMPACT_DETECTED" 128 "UNSUITABLE_USE_OF_ACCELERATOR_PEDAL_DETECTED" 129 "GEAR_SHIFTER_USE_DETECTED" 130 "UNSUITABLE_BRAKE_PEDAL_USE_DETECTED" 131 "VEHICLE_START_BUTTON_PRESSED" 132 "VEHICLE_HAS_BEEN_STOPPED_FOR_TOO_LONG" 141 "TRAFFIC_CONGESTION_HAS_CLEARED" 142 "ENTRANCE_AND_EXIT_RAMPS_AHEAD" 143 "UNSUPPORTED_LANE_AHEAD" 144 "UNSUPPORTED_ROAD_TYPE_AHEAD" 145 "LANE_DEPARTURE_DETECTED" 146 "MAXIMUM_SPEED_EXCEEDED" 147 "HIGHWAY_DRIVING_PILOT_LIMITED_ABNORMAL_VEHICLE_CONTROLLER_STATUS" 148 "WIPER_LIGHT_CONTROL_SETTINGS_ARE_UNSUITABLE_FOR_USE_WITH_HDP" 149 "WINDSHIELD_DEFOG_SYSTEM_STATUS_IS_UNSUITABLE_FOR_USE_WITH_HDP" 150 "HAZARD_WARNING_LIGHTS_OR_TURN_SIGNAL_OPERATION_DETECTED" 151 "PERFORMING_EVASIVE_STEERING_OBSTACLES_DETECTED_AHEAD" 152 "HIGHWAY_DRIVING_PILOT_LIMITED_SENSOR_DETECTION_RANGE_LIMITED" 160 "CHECK_HIGHWAY_DRIVING_PILOT_SYSTEM" 161 "SAFETY_FUNCTION_ACTIVATED" 176 "CAMERA_OBSCURED" 177 "RADAR_BLOCKED" 178 "LIDAR_BLOCKED" 179 "AIRBAG_WARNING_LIGHT_IS_ON" 180 "ATTACHED_TRAILED_DETECTED" 181 "HIGH_OUTSIDE_TEMPERATURE" 182 "LOW_OUTSIDE_TEMPERATURE" 190 "UNAVAILABLE_DUE_TO_THE_ROAD_EVENT_INFORMATION_RECEIVED" 191 "UNAVAILABLE_NEAR_TOLLGATES" 192 "DRIVERS_SEAT_IS_NOT_IN_A_SAFE_DRIVING_POSITION" 193 "VEHICLE_DRIVING_THE_WRONG_WAY_DETECTED_AHEAD" 194 "EMERGENCY_VEHICLE_DETECTED" 195 "OBSTACLE_DETECTED_AHEAD" 196 "SENSOR_BLOCKED_DUE_TO_RAIN_SNOW_OR_ROAD_DEBRIS" 197 "SLIPPERY_ROAD_SURFACE_DETECTED" 198 "CONSTRUCTION_ZONE_DETECTED_AHEAD" 199 "PEDESTRIAN_DETECTED_AHEAD" 200 "UNSUITABLE_DRIVERS_SEAT_POSITION_DETECTED" 201 "FOLDED_SIDE_VIEW_MIRRORS_DETECTED" 208 "VEHICLE_POSITION_NOT_DETECTED" 209 "LANE_NOT_DETECTED" 210 "DRIVER_NOT_DETECTED" 211 "KEEP_YOUR_EYES_ON_THE_ROAD" 212 "LEADING_VEHICLE_REQUIRED_AFTER_SUNSET" 213 "TBD" 240 "LOW_FUEL" 241 "LOW_TIRE_PRESSURE" 242 "DOOR_OPEN" 243 "TRUNK_OPEN" 244 "HOOD_OPEN" 245 "SEAT_BELT_NOT_FASTENED" 246 "PARKING_BRAKE_ACTIVATED" 247 "LOW_EV_BATTERY" 248 "HDP_DEACTIVATION_DELAYED_RISK_OF_COLLISION_DETECTED" 249 "LIFTGATE_OPENED";
          if values["ALERTS_5"] == 5:
            values["ALERTS_5"] = 0
          # ALERTS_5 0 "HIDDEN" 1 "DRIVERS_GRASP_NOT_DETECTED_DRIVING_SPEED_WILL_BE_LIMITED" 2 "WATCH_FOR_SURROUNDING_VEHICLES" 3 "SMART_CRUISE_CONTROL_DEACTIVATED" 4 "SMART_CRUISE_CONTROL_CONDITIONS_NOT_MET" 5 "USE_SWITCH_OR_PEDAL_TO_ACCELERATE" 6 "DRIVER_ASSISTNCE_SYSTEM_LIMITED_TRAILER_ATTACHED" 7 "DRIVER_ASSISTNCE_SYSTEM_LIMITED_DRIVER_FULL_FACE_NOT_VISIBLE" 11 "LEADING_VEHICLE_IS_DRIVING_AWAY" 12 "STOP_VEHICLE_THEN_TRY_AGAIN" 19 "ACTIVATING_HIGHWAY_DRIVING_PILOT_SYSTEM" 20 "CONTINUING_USE_OF_HIGHWAY_DRIVING_PILOT_WILL_RESULT_IN_DEVIATION_FROM_THE_NAVIGATION_ROUTE" 21 "HIGHWAY_DRIVING_PILOT_SYSTEM_DEACTIVATED_SILENT" 22 "HIGHWAY_DRIVING_PILOT_SYSTEM_NOT_APPLIED" 23 "HIGHWAY_DRIVING_PILOT_CONDITIONS_NOT_MET_SILENT";
          values["CENTERLINE"] = 1 if CC.latActive else 0

          curvature = {
            i: (31 if i == -1 else 13 - abs(i + 15)) if i < 0 else 15 + i
            for i in range(-15, 16)
          }
          values["LANELINE_CURVATURE"] = curvature.get(max(-15, min(int(disp_angle / 3), 15)), 14) if main_enabled else 15

          if hud_control.leftLaneDepart:
            values["LANELINE_LEFT"] = 4 if (frame // 50) % 2 == 0 else 1
          else:
            values["LANELINE_LEFT"] = 2 if hud_control.leftLaneVisible else 0

          if hud_control.rightLaneDepart:
            values["LANELINE_RIGHT"] = 4 if (frame // 50) % 2 == 0 else 1
          else:
            values["LANELINE_RIGHT"] = 2 if hud_control.rightLaneVisible else 0

          values["LANELINE_LEFT_POSITION"] = 15
          values["LANELINE_RIGHT_POSITION"] = 15

          speed_below_threshold = CS.out.vEgo < 8.94
          values["LCA_LEFT_ICON"] = 0 if CS.out.leftBlindspot or speed_below_threshold else 2 if CS.out.leftBlinker else 1
          values["LCA_RIGHT_ICON"] = 0 if CS.out.rightBlindspot or speed_below_threshold else 2 if CS.out.rightBlinker else 1
          values["LCA_LEFT_ARROW"] = 2 if CS.out.leftBlinker else 0
          values["LCA_RIGHT_ARROW"] = 2 if CS.out.rightBlinker else 0

          ret.append(packer.make_can_msg("ADRV_0x161", CAN.ECAN, values))

        if CS.adrv_info_162 is not None:
          values = CS.adrv_info_162
          values["FAULT_FSS"] = 0
          values["FAULT_FCA"] = 0

          values["FAULT_LSS"] = 0
          values["FAULT_HDA"] = 0
          values["FAULT_DAS"] = 0

          values["FAULT_SLA"] = 0
          values["FAULT_DAW"] = 0
          values["FAULT_SCC"] = 0
          values["FAULT_LFA"] = 0
          values["FAULT_LCA"] = 0

          # FAULT_FSS 0 "HIDDEN" 1 "CHECK_FORWARD_SAFETY_SYSTEM" 2 "FORWARD_SAFETY_SYSTEM_LIMITED_CAMERA_OBSCURED" 3 "FORWARD_SAFETY_SYSTEM_LIMITED_RADAR_BLOCKED";
          # FAULT_FCA 0 "HIDDEN" 1 "CHECK_FORWARD_SIDE_SAFETY_SYSTEM" 2 "FORWARD_SIDE_SAFETY_SYSTEM_LIMITED_CAMERA_OBSCURED" 3 "FORWARD_SIDE_SAFETY_SYSTEM_LIMITED_RADAR_BLOCKED";
          # FAULT_LSS 0 "HIDDEN" 1 "CHECK_LANE_SAFETY_SYSTEM" 2 "LANE_SAFETY_SYSTEM_DISABLED_CAMERA_OBSCURED";
          # FAULT_HDA 0 "HIDDEN" 1 "CHECK_HIGHWAY_DRIVING_ASSIST_SYSTEM";
          # FAULT_DAS 0 "HIDDEN" 1 "CHECK_DRIVER_ASSISTANCE_SYSTEM" 2 "DRIVER_ASSISTANCE_SYSTEM_LIMITED_CAMERA_OBSCURED" 3 "DRIVER_ASSISTANCE_SYSTEM_LIMITED_RADAR_BLOCKED" 4 "DRIVER_ASSISTANCE_SYSTEM_LIMITED_CAMERA_OBSCURED_AND_RADAR_BLOCKED";
          # FAULT_SLA 0 "HIDDEN" 1 "CHECK_SPEED_LIMIT_SYSTEM" 2 "SPEED_LIMIT_SYSTEM_DISABLED_CAMERA_OBSCURED";
          # FAULT_DAW 0 "HIDDEN" 1 "CHECK_INATTENTIVE_DRIVING_WARNING_SYSTEM" 2 "INATTENTIVE_DRIVING_WARNING_SYSTEM_DISABLED_CAMERA_OBSCURED";
          # FAULT_SCC 0 "HIDDEN" 1 "CHECK_SMART_CRUISE_CONTROL_SYSTEM" 2 "SMART_CRUISE_CONTROL_DISABLED_RADAR_BLOCKED";
          # FAULT_LFA 0 "HIDDEN" 1 "CHECK_LANE_FOLLOWING_SYSTEM_ASSIST_SYSTEM";
          # FAULT_LCA 0 "HIDDEN" 1 "CHECK_LANE_CHANGE_ASSIST_FUNCTION" 2 "LANE_CHANGE_ASSIST_FUNCTION_DISABLED_CAMERA_OBSCURED" 3 "LANE_CHANGE_ASSIST_FUNCTION_DISABLED_RADAR_BLOCKED";
          if left_lane_warning or right_lane_warning:
            values["HAPTIC_VIBRATE"] = 1
          ret.append(packer.make_can_msg("ADRV_0x162", CAN.ECAN, values))

      if CS.adrv_info_160 is not None:
        values = CS.adrv_info_160
        values["NEW_SIGNAL_1"] = 0  # steer_temp관련없음, 계기판에러
        values["SET_ME_9"] = 17  # steer_temp관련없음, 계기판에러
        values["SET_ME_2"] = 0  # 커멘트해도 steer_temp에러남, 2값은 콤마에서 찾은거니...
        values["DATA102"] = 0  # steer_temp관련없음
        ret.append(packer.make_can_msg("ADRV_0x160", CAN.ECAN, values))

      if CS.adrv_info_200 is not None:
        values = CS.adrv_info_200
        values["TauGapSet"] = hud_control.leadDistanceBars
        ret.append(packer.make_can_msg("ADRV_0x200", CAN.ECAN, values))

      if CS.adrv_info_1ea is not None:
        values = CS.adrv_info_1ea
        values["HDA_MODE1"] = 8
        values["HDA_MODE2"] = 1
        ret.append(packer.make_can_msg("ADRV_0x1ea", CAN.ECAN, values))

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
        ret.append(packer.make_can_msg("HDA_INFO_0x4a3", CAN.CAM, values))
    if frame % 10 == 0:
      if CS.hda_info_4b4 is not None: #G80 HDA2개조차량은 안나옴...
        values = CS.hda_info_4b4
        values["NEW_SIGNAL_1"] = 8
        values["NEW_SIGNAL_3"] = (frame / 100) % 10
        values["NEW_SIGNAL_4"] = 146
        values["NEW_SIGNAL_5"] = 68
        values["NEW_SIGNAL_6"] = 76
        ret.append(packer.make_can_msg("HDA_INFO_0x4b4", CAN.CAM, values))
        """
    return ret
  else:
    values = {}

    ret.extend(create_fca_warning_light(CP, packer, CAN, frame))
    if frame % 5 == 0:
      values = {
        'HDA_MODE1': 8,
        'HDA_MODE2': 1,
        #'SET_ME_1C': 28,
        'SET_ME_FF': 255,
        #'SET_ME_TMP_F': 15,
        #'SET_ME_TMP_F_2': 15,
        #'DATA26': 1,
        #'DATA32': 5,
      }
      ret.append(packer.make_can_msg("ADRV_0x1ea", CAN.ECAN, values))

      values = {
        'SET_ME_E1': 225,
        #'SET_ME_3A': 58,
        'TauGapSet' : 1,
        'NEW_SIGNAL_2': 3,
      }
      ret.append(packer.make_can_msg("ADRV_0x200", CAN.ECAN, values))

    if frame % 20 == 0:
      values = {
        'SET_ME_15': 21,
      }
      ret.append(packer.make_can_msg("ADRV_0x345", CAN.ECAN, values))

    if frame % 100 == 0:
      values = {
        'SET_ME_22': 34,
        'SET_ME_41': 65,
      }
      ret.append(packer.make_can_msg("ADRV_0x1da", CAN.ECAN, values))

    return ret
