import math
import random
import threading
import numpy as np

from opendbc.car import structs
from openpilot.common.params import Params
from openpilot.common.conversions import Conversions as CV
from openpilot.selfdrive.car.cruise import (V_CRUISE_MIN, V_CRUISE_MAX, V_CRUISE_UNSET, V_CRUISE_INITIAL, V_CRUISE_INITIAL_EXPERIMENTAL_MODE,
                                            CRUISE_LONG_PRESS, IMPERIAL_INCREMENT)
from opendbc.car.hyundai.values import Buttons, HyundaiFlags
from openpilot.selfdrive.modeld.constants import ModelConstants
from openpilot.selfdrive.controls.neokii.navi_controller import SpeedLimiter

"""
MPH_TO_KPH = 1.609344
KPH_TO_MPH = 1. / MPH_TO_KPH
MS_TO_KPH = 3.6
KPH_TO_MS = 1. / MS_TO_KPH
MS_TO_MPH = MS_TO_KPH * KPH_TO_MPH
MPH_TO_MS = MPH_TO_KPH * KPH_TO_MS

V_CRUISE_MIN = 10
V_CRUISE_MAX = 145
V_CRUISE_UNSET = 255
V_CRUISE_INITIAL = 30
V_CRUISE_INITIAL_EXPERIMENTAL_MODE = 105
CRUISE_LONG_PRESS = 50
"""

NO_LIMIT_SPEED = 255

ButtonType = structs.CarState.ButtonEvent.Type
GearShifter = structs.CarState.GearShifter


class UnitConverter:
  def __init__(self):
    self.params = Params()
    self.is_metric = self.params.get_bool('IsMetric')

  def to_ms(self, speed: float) -> float:
    return speed * CV.KPH_TO_MS if self.is_metric else speed * CV.MPH_TO_MS

  def to_clu(self, speed: float) -> float:
    return speed * CV.MS_TO_KPH if self.is_metric else speed * CV.MS_TO_MPH

  def to_current_unit(self, speed_kph: float) -> float:
    return speed_kph if self.is_metric else speed_kph * CV.KPH_TO_MPH


class CruiseButtonHandler:
  def __init__(self):
    self.btn_count = 0
    self.prev_btn = ButtonType.unknown
    self.btn_long_pressed = False

  def update(self, btn_events):
    btn = ButtonType.unknown

    if self.btn_count > 0:
      self.btn_count += 1

    for b in btn_events:
      if b.pressed and self.btn_count == 0 and b.type in [
          ButtonType.accelCruise,
          ButtonType.decelCruise,
          ButtonType.gapAdjustCruise,
          ButtonType.cancel,
          ButtonType.lfaButton
        ]:
        self.btn_count = 1
        self.prev_btn = b.type
      elif not b.pressed and self.btn_count > 0:
        if not self.btn_long_pressed:
          btn = b.type
        self.btn_long_pressed = False
        self.btn_count = 0

    if self.btn_count > CRUISE_LONG_PRESS:
      self.btn_long_pressed = True
      btn = self.prev_btn
      self.btn_count %= CRUISE_LONG_PRESS

    return btn, self.btn_long_pressed


class CruiseController:
  def __init__(self, CP, CI):
    self.CP = CP
    self.CI = CI

    self.params = Params()
    self.conv = UnitConverter()
    self.btn_handler = CruiseButtonHandler()

    self.experimental_mode = self.params.get_bool("ExperimentalMode")

    self.min_set_speed_clu = self.conv.to_current_unit(V_CRUISE_MIN) if CruiseStateManager.instance().cruise_state_control else self.conv.to_current_unit(V_CRUISE_INITIAL)
    self.max_set_speed_clu = self.conv.to_current_unit(V_CRUISE_MAX)

    self.btn = Buttons.NONE
    self.target_speed_clu = 0.
    self.max_speed_clu = 0.
    self.curve_speed_clu = 0.
    self.cruise_speed_kph = 0.
    self.real_set_speed_kph = 0.
    self.v_cruise_kph = V_CRUISE_UNSET
    self.v_cruise_cluster_kph = V_CRUISE_UNSET
    self.v_cruise_kph_last = 0
    self.prev_cruise_enabled = False

    self.wait_timer = 0
    self.alive_timer = 0
    self.alive_index = 0
    self.wait_index = 0
    self.alive_count = 0
    self.wait_count_list = [16] if CP.flags & HyundaiFlags.CANFD else [16, 20]
    self.alive_count_list = [20] if CP.flags & HyundaiFlags.CANFD else [12, 14, 16, 18]
    random.shuffle(self.wait_count_list)
    random.shuffle(self.alive_count_list)

  def _get_alive_count(self):
    count = self.alive_count_list[self.alive_index]
    self.alive_index = (self.alive_index + 1) % len(self.alive_count_list)
    return count

  def _get_wait_count(self):
    count = self.wait_count_list[self.wait_index]
    self.wait_index = (self.wait_index + 1) % len(self.wait_count_list)
    return count

  def reset(self):
    self.btn = Buttons.NONE
    self.wait_timer = 0
    self.alive_timer = 0
    self.target_speed_clu = 0.
    self.max_speed_clu = 0.
    self.curve_speed_clu = 0.

  def _cal_max_speed(self, CS, sm, cluster_speed: float, v_cruise_kph: float):
    apply_limit_speed, is_limit_zone = SpeedLimiter.instance().get_max_speed(cluster_speed, self.conv.is_metric)
    road_limit_speed_nda = SpeedLimiter.instance().get_road_limit_speed()
    road_limit_speed_stock = CS.exState.navLimitSpeed
    road_limit_speed = next((s for s in [road_limit_speed_nda, road_limit_speed_stock] if s is not None and s > 0), None)
    ratio = np.interp(road_limit_speed, [self.conv.to_current_unit(10.0), self.conv.to_current_unit(100.0)], [1.30, 1.10])

    # init current speed
    current_max_speed_clu = self.conv.to_current_unit(v_cruise_kph)

    # 1. Road limit speed
    road_limit_speed_clu = road_limit_speed * ratio if road_limit_speed else NO_LIMIT_SPEED

    # 2. Apply limit speed
    apply_limit_speed_clu = apply_limit_speed if apply_limit_speed >= self.min_set_speed_clu else NO_LIMIT_SPEED

    # 3. Lead limit speed
    lead = sm['radarState'].leadOne
    lead_speed = self._cal_lead_speed(lead, cluster_speed)
    lead_limit_speed_clu = lead_speed if self.CP.openpilotLongitudinalControl and lead.status else NO_LIMIT_SPEED

    # 4. Curve limit speed
    model = sm['modelV2']
    curve_speed = self._cal_curve_speed(model, CS.vEgo, v_cruise_kph)
    curve_limit_speed_clu = curve_speed if sm.frame % 20 == 0 else NO_LIMIT_SPEED

    # 5. Steering angle based limit speed
    steer_limit_speed_clu = self._cal_steer_based_speed(CS.vEgo, CS.steeringAngleDeg)

    speed_candidates = [
      current_max_speed_clu,
      road_limit_speed_clu,
      apply_limit_speed_clu,
      lead_limit_speed_clu,
      curve_limit_speed_clu,
      steer_limit_speed_clu
    ]
    calculated_max_speed_clu = min(s for s in speed_candidates if s >= self.min_set_speed_clu and s != NO_LIMIT_SPEED)

    if not self.CP.openpilotLongitudinalControl or self.max_speed_clu <= 0 or is_limit_zone:
      self.max_speed_clu = calculated_max_speed_clu
    else:
      error = calculated_max_speed_clu - self.max_speed_clu
      kp = 0.01
      self.max_speed_clu += error * kp

  def _cal_lead_speed(self, lead, cluster_speed_clu: float):
    lead_distance_buffer = 5.
    distance = lead.dRel - lead_distance_buffer
    relative_speed = lead.vRel
    lead_decay_factor = 22.
    lead_accel_gain = 1.2
    min_relative_speed = -1.0

    is_valid_deceleration = (
      0 < distance < -relative_speed * lead_decay_factor and
      relative_speed < min_relative_speed
    )

    if not is_valid_deceleration:
      return NO_LIMIT_SPEED

    time = distance / relative_speed if abs(relative_speed) > 1e-3 else 0.1
    deceleration_ms = -relative_speed / time
    speed_delta_clu = self.conv.to_clu(deceleration_ms) * lead_accel_gain
    new_speed_clu = cluster_speed_clu + speed_delta_clu
    lead_speed_clu = max(new_speed_clu, self.min_set_speed_clu)

    return lead_speed_clu

  def _cal_curve_speed(self, model, current_speed_ms: float, v_cruise_kph: float):
    trajectory_size = ModelConstants.IDX_N

    if len(model.position.x) != trajectory_size or len(model.position.y) != trajectory_size:
      return NO_LIMIT_SPEED

    x = model.position.x
    y = model.position.y
    dy = np.gradient(y, x)
    d2y = np.gradient(dy, x)
    curv = d2y / (1 + dy ** 2) ** 1.5
    curv_segment = curv[-10:]
    curv_segment_abs = np.abs(curv_segment)

    a_y_max = 2.975 - current_speed_ms * 0.0375
    v_curvature = np.sqrt(a_y_max / np.clip(curv_segment_abs, 1e-4, None))
    model_speed_ms = float(np.mean(v_curvature)) * 0.85

    min_curve_speed_ms = np.interp(current_speed_ms, [0.0, self.conv.to_ms(60.0)], [self.conv.to_ms(30.0), self.conv.to_ms(50.0)])
    model_based_speed_ms = float(max(model_speed_ms, min_curve_speed_ms)) \
    if not math.isnan(model_speed_ms) and model_speed_ms < current_speed_ms else NO_LIMIT_SPEED

    orientation_rate = np.array(model.orientationRate.z)
    velocity = np.array(model.velocity.x)
    predicted_lat_acc = float(np.max(np.abs(orientation_rate * velocity)))
    acc_based_curvature = predicted_lat_acc / max(current_speed_ms, 1.0) ** 2

    acc_based_speed_ms = NO_LIMIT_SPEED
    if acc_based_curvature > 1e-4:
      temp_acc_speed_ms = np.sqrt(a_y_max / acc_based_curvature) * 0.85
      acc_based_speed_ms = float(max(temp_acc_speed_ms, min_curve_speed_ms)) \
        if temp_acc_speed_ms < current_speed_ms else NO_LIMIT_SPEED

    candidates_ms = [s for s in [model_based_speed_ms, acc_based_speed_ms] if s != NO_LIMIT_SPEED]
    calculated_curve_speed_ms = min(candidates_ms) if candidates_ms else NO_LIMIT_SPEED
    curve_speed_ms = min(calculated_curve_speed_ms, self.conv.to_ms(v_cruise_kph))
    self.curve_speed_clu = self.conv.to_clu(curve_speed_ms)

    return self.curve_speed_clu

  def _cal_steer_based_speed(self, current_speed_ms: float, steering_angle_deg: float):
    start_decel_angle = 45.
    end_decel_angle = 120.
    abs_steer_angle = abs(steering_angle_deg)

    if abs_steer_angle < start_decel_angle:
      return NO_LIMIT_SPEED

    speed_multiplier = np.interp(abs_steer_angle, [start_decel_angle, end_decel_angle], [1.0, 0.7])
    target_speed_ms = current_speed_ms * speed_multiplier
    min_allowed_speed_ms = self.conv.to_ms(V_CRUISE_MIN)
    steer_based_speed_ms = max(target_speed_ms, min_allowed_speed_ms)
    steer_based_speed_clu = self.conv.to_clu(steer_based_speed_ms)

    return steer_based_speed_clu

  def _target_speed(self, CS, cluster_speed_clu: float, v_cruise_kph: float, cruise_btn_pressed: bool):
    syncing = CS.gasPressed and not cruise_btn_pressed
    sync_margin = 3.

    if not self.CP.openpilotLongitudinalControl:
      if syncing and cluster_speed_clu + sync_margin > self.conv.to_current_unit(v_cruise_kph):
        set_speed = np.clip(cluster_speed_clu + sync_margin, V_CRUISE_INITIAL, self.max_set_speed_clu)
        v_cruise_kph = int(round(self.conv.to_current_unit(set_speed)))

      self.target_speed_clu = self.conv.to_current_unit(v_cruise_kph)
      if self.max_speed_clu > V_CRUISE_INITIAL:
        self.target_speed_clu = np.clip(self.target_speed_clu, V_CRUISE_INITIAL, self.max_speed_clu)

    elif CS.cruiseState.enabled:
      if syncing and cluster_speed_clu + sync_margin > self.conv.to_current_unit(v_cruise_kph):
        set_speed = np.clip(cluster_speed_clu + sync_margin, self.min_set_speed_clu, self.max_set_speed_clu)
        self.target_speed_clu = int(round(self.conv.to_current_unit(set_speed)))

        if CruiseStateManager.instance().cruise_state_control:
          CruiseStateManager.instance().speed = self.conv.to_ms(set_speed)

  def _get_button_to_adjust_speed(self, current_set_speed: float) -> Buttons:
    if self.target_speed_clu < V_CRUISE_INITIAL:
      return Buttons.NONE
    error = self.target_speed_clu - current_set_speed
    if abs(error) < 0.9:
      return Buttons.NONE
    return Buttons.RES_ACCEL if error > 0 else Buttons.SET_DECEL

  def _initialize_v_cruise(self, CS):
    initial = V_CRUISE_INITIAL_EXPERIMENTAL_MODE if self.experimental_mode else V_CRUISE_INITIAL

    if any(b.type in (ButtonType.accelCruise, ButtonType.resumeCruise) for b in
           CS.buttonEvents) and self.v_cruise_kph != V_CRUISE_UNSET:
      self.v_cruise_kph = self.v_cruise_kph_last
    else:
      self.v_cruise_kph = int(round(np.clip(self.conv.to_clu(CS.vEgo), initial, V_CRUISE_MAX)))

    self.v_cruise_cluster_kph = self.v_cruise_kph
    return self.v_cruise_kph

  def update_v_cruise(self, CS, sm, enabled: bool):
    self.v_cruise_kph_last = self.v_cruise_kph
    v_cruise_kph = self.v_cruise_kph

    if CS.cruiseState.enabled:
      if not self.CP.openpilotLongitudinalControl or not self.CP.pcmCruise:
        v_cruise_kph = self._update_cruise_button(v_cruise_kph, CS.buttonEvents, enabled)
      else:
        v_cruise_kph = self.conv.to_clu(CS.cruiseState.speed)
        if CS.cruiseState.speed == 0:
          v_cruise_kph = V_CRUISE_UNSET
        elif CS.cruiseState.speed == -1:
          v_cruise_kph = -1
    else:
      v_cruise_kph = V_CRUISE_UNSET

    if self.prev_cruise_enabled != CS.cruiseState.enabled:
      self.prev_cruise_enabled = CS.cruiseState.enabled
      if CS.cruiseState.enabled:
        if not self.CP.pcmCruise:
          v_cruise_kph = self._initialize_v_cruise(CS)
        else:
          v_cruise_kph = self.conv.to_clu(CS.cruiseState.speed)
          if CS.cruiseState.speed == 0:
            v_cruise_kph = V_CRUISE_UNSET
          elif CS.cruiseState.speed == -1:
            v_cruise_kph = -1

    self.real_set_speed_kph = v_cruise_kph
    if CS.cruiseState.enabled and 1 < CS.cruiseState.speed < V_CRUISE_UNSET:
      cluster_speed_clu = self.conv.to_clu(CS.vEgoCluster)
      self._cal_max_speed(CS, sm, cluster_speed_clu, v_cruise_kph)
      self.cruise_speed_kph = float(np.clip(v_cruise_kph, V_CRUISE_MIN, self.conv.to_current_unit(self.max_speed_clu)))
      self._target_speed(CS, cluster_speed_clu, self.real_set_speed_kph, self.CI.CS.cruise_buttons[-1] != Buttons.NONE)

      if CruiseStateManager.instance().cruise_state_control:
        self.cruise_speed_kph = min(self.cruise_speed_kph, max(self.real_set_speed_kph, V_CRUISE_MIN))

    else:
      self.cruise_speed_kph = self.conv.to_clu(CS.vEgoCluster)
      self.reset()

    self.v_cruise_kph = v_cruise_kph
    self._update_message(CS)

  def _update_cruise_button(self, v_cruise_kph, btn_events, enabled):
    v_cruise_delta = 10 if self.conv.is_metric else IMPERIAL_INCREMENT * 5

    if enabled:
      btn, long_pressed = self.btn_handler.update(btn_events)

      if btn != Buttons.NONE:
        if not long_pressed:
          if btn == ButtonType.accelCruise:
            v_cruise_kph += (1 if self.conv.is_metric else IMPERIAL_INCREMENT)
          elif btn == ButtonType.decelCruise:
            v_cruise_kph -= (1 if self.conv.is_metric else IMPERIAL_INCREMENT)
        else:
          if btn == ButtonType.accelCruise:
            v_cruise_kph += (v_cruise_delta - v_cruise_kph % v_cruise_delta)
          elif btn == ButtonType.decelCruise:
            v_cruise_kph -= (v_cruise_delta - (-v_cruise_kph) % v_cruise_delta)

      v_cruise_kph = np.clip(round(v_cruise_kph), V_CRUISE_MIN, V_CRUISE_MAX)

    return v_cruise_kph

  def spam_message(self, CS, can_sends):
    ascc_enabled = CS.cruiseState.enabled and 1 < CS.cruiseState.speed < V_CRUISE_UNSET and not CS.brakePressed
    btn_pressed = self.CI.CS.cruise_buttons[-1] != Buttons.NONE

    if not self.CP.openpilotLongitudinalControl:
      if not ascc_enabled or btn_pressed:
        self.reset()
        self.wait_timer = max(self.alive_count_list) + max(self.wait_count_list)
        return

    if not ascc_enabled:
      self.reset()

    if self.wait_timer > 0:
      self.wait_timer -= 1
    elif ascc_enabled and CS.vEgo > 0.1:
      if self.alive_timer == 0:
        current_set_speed_clu = int(round(self.conv.to_clu(CS.cruiseState.speed)))
        self.btn = self._get_button_to_adjust_speed(current_set_speed_clu)
        self.alive_count = self._get_alive_count()

      if self.btn != Buttons.NONE:
        can = self.CI.create_buttons(self.btn)
        if can is not None:
          can_sends.append(can)

        self.alive_timer += 1
        if self.alive_timer >= self.alive_count:
          self.alive_timer = 0
          self.wait_timer = self._get_wait_count()
          self.btn = Buttons.NONE
      elif self.CP.openpilotLongitudinalControl and self.target_speed_clu >= V_CRUISE_INITIAL:
        self.target_speed_clu = 0.
    elif self.CP.openpilotLongitudinalControl:
      self.target_speed_clu = 0.

  def _update_message(self, CS):
    exState = CS.exState
    exState.vCruiseKph = float(self.v_cruise_kph)
    exState.cruiseMaxSpeed = float(self.real_set_speed_kph)
    exState.applyMaxSpeed = float(self.cruise_speed_kph)
    exState.targetSpeed = float(self.target_speed_clu)
    exState.maxSpeed = float(self.max_speed_clu)
    exState.curveSpeed = float(self.curve_speed_clu)

class CruiseStateManager:
  def __init__(self):
    self.params = Params()
    self.conv = UnitConverter()
    self.btn_handler = CruiseButtonHandler()

    self.cruise_state_control = self.params.get_bool('CruiseStateControl')

    self.available = False
    self.enabled = False
    self.speed = self.conv.to_ms(V_CRUISE_INITIAL)

    self.prev_brake_pressed = False
    self.prev_main_button = False

  @classmethod
  def instance(cls):
    if not hasattr(cls, "_instance"):
      cls._instance = cls()
    return cls._instance

  def _reset_available(self):
    self.available = False
    threading.Timer(3.0, lambda: setattr(self, 'available', True)).start()

  def _reset_speed(self):
    self.enabled = False
    self.speed = self.conv.to_ms(V_CRUISE_INITIAL)

  def update(self, CS, main_buttons):
    btn, long_pressed = self.btn_handler.update(CS.buttonEvents)

    if btn != ButtonType.unknown:
      self._button_press(CS, btn, long_pressed)

    self._main_button_toggle(main_buttons[-1])

    if not self.available:
      self._reset_speed()

    if not self.prev_brake_pressed and CS.brakePressed:
      self._reset_speed()
    self.prev_brake_pressed = CS.brakePressed

    if CS.gearShifter == GearShifter.park:
      self._reset_speed()

    CS.cruiseState.available = self.available
    CS.cruiseState.enabled = self.enabled
    CS.cruiseState.standstill = False
    CS.cruiseState.speed = float(self.speed)

  def _main_button_toggle(self, current_main_button: bool) -> None:
    if current_main_button != self.prev_main_button and current_main_button:
      self.available = not self.available
    self.prev_main_button = current_main_button

  def _button_press(self, CS, btn, long_pressed):
    v_cruise_delta = 10 if self.conv.is_metric else IMPERIAL_INCREMENT * 5
    v_cruise_kph = int(round(self.conv.to_clu(self.speed)))

    road_limit_speed_nda = SpeedLimiter.instance().get_road_limit_speed()
    road_limit_speed_stock = CS.exState.navLimitSpeed
    road_limit_speed = next((s for s in [road_limit_speed_nda, road_limit_speed_stock] if s is not None and s > 0), None)

    if btn == ButtonType.accelCruise:
      if self.enabled:
        if not long_pressed:
          v_cruise_kph += (1 if self.conv.is_metric else IMPERIAL_INCREMENT)
        else:
          v_cruise_kph += (v_cruise_delta - v_cruise_kph % v_cruise_delta)
      elif not self.enabled and self.available and CS.gearShifter != GearShifter.park:
        self.enabled = True
        v_cruise_kph = max(np.clip(round(self.conv.to_clu(self.speed)), V_CRUISE_INITIAL, V_CRUISE_MAX),
                           round(self.conv.to_clu(CS.vEgoCluster)))

    if btn == ButtonType.decelCruise:
      if self.enabled:
        if not long_pressed:
          v_cruise_kph -= (1 if self.conv.is_metric else IMPERIAL_INCREMENT)
        else:
          v_cruise_kph -= (v_cruise_delta - (-v_cruise_kph) % v_cruise_delta)
      elif not self.enabled and self.available and CS.gearShifter != GearShifter.park:
        self.enabled = True
        v_cruise_kph = max(np.clip(round(self.conv.to_clu(CS.vEgoCluster)), V_CRUISE_MIN, V_CRUISE_MAX),
                           V_CRUISE_INITIAL)

    if btn == ButtonType.gapAdjustCruise:
      if long_pressed:
        self.params.put_bool("ExperimentalMode", not self.params.get_bool("ExperimentalMode"))

    if btn == ButtonType.cancel:
      if not long_pressed:
        self._reset_speed()
      else:
        self._reset_available()
        self._reset_speed()

    if btn == ButtonType.lfaButton:
      if not long_pressed:
        if road_limit_speed is not None:
          if self.enabled:
            v_cruise_kph = road_limit_speed
          elif not self.enabled and self.available and CS.gearShifter != GearShifter.park:
            self.enabled = True
            v_cruise_kph = road_limit_speed
      else:
        self._reset_available()
        self._reset_speed()

    v_cruise_kph = np.clip(round(v_cruise_kph), V_CRUISE_MIN, V_CRUISE_MAX)
    self.speed = self.conv.to_ms(v_cruise_kph)
