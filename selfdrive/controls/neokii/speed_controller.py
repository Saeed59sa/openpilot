import random
import numpy as np
import math

from opendbc.car.hyundai.values import Buttons, HyundaiFlags
from openpilot.common.conversions import Conversions as CV
from openpilot.common.params import Params
from openpilot.selfdrive.car.cruise import V_CRUISE_MIN, V_CRUISE_MAX, V_CRUISE_INITIAL, V_CRUISE_UNSET, VCruiseHelper
from openpilot.selfdrive.controls.neokii.cruise_state_manager import CruiseStateManager
from openpilot.selfdrive.controls.neokii.navi_controller import SpeedLimiter
from openpilot.selfdrive.modeld.constants import ModelConstants

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
"""

class SpeedController:
  def __init__(self, CP, CI):
    self.CP = CP
    self.CI = CI
    self.long_control = CP.openpilotLongitudinalControl
    self.params = Params()

    self.is_metric = self.params.get_bool('IsMetric')
    self.experimental_mode = self.params.get_bool("ExperimentalMode") and self.long_control

    self.min_set_speed_clu = self.to_current_unit(V_CRUISE_MIN) if CruiseStateManager.instance().cruise_state_control else self.to_current_unit(V_CRUISE_INITIAL)
    self.max_set_speed_clu = self.to_current_unit(V_CRUISE_MAX)

    self.btn = Buttons.NONE
    self.target_speed_clu = 0.
    self.max_speed_clu = 0.
    self.curve_speed_clu = 255.
    self.cruise_speed_kph = 0.
    self.real_set_speed_kph = 0.

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

    self.v_cruise_helper = VCruiseHelper(self.CP)

  def conv_to_ms(self, speed: float) -> float:
    return speed * CV.KPH_TO_MS if self.is_metric else speed * CV.MPH_TO_MS

  def conv_to_clu(self, speed: float) -> float:
    return speed * CV.MS_TO_KPH if self.is_metric else speed * CV.MS_TO_MPH

  def to_current_unit(self, speed_kph: float) -> float:
    return speed_kph if self.is_metric else speed_kph * CV.KPH_TO_MPH

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
    self.curve_speed_clu = 255.

  def _calculate_max_speed(self, CS, sm, cluster_speed: float, v_cruise_kph: float):
    apply_limit_speed, is_limit_zone = SpeedLimiter.instance().get_max_speed(cluster_speed, self.is_metric)

    road_limit_speed_nda = SpeedLimiter.instance().get_road_limit_speed()
    road_limit_speed_stock = CS.exState.navLimitSpeed

    road_limit_speed = next((s for s in [road_limit_speed_nda, road_limit_speed_stock] if s is not None and s > 0), None)

    current_max_speed_clu = self.to_current_unit(v_cruise_kph)

    # 1. Road limit speed
    if road_limit_speed is not None and self.min_set_speed_clu <= road_limit_speed < current_max_speed_clu:
      limit_ratio = [1.30, 1.10]
      speed_bp = [self.to_current_unit(10.0), self.to_current_unit(100.0)]

      ratio = np.interp(road_limit_speed, speed_bp, limit_ratio)
      road_limit_speed_decel = road_limit_speed * ratio

      prelimit_clu = max(road_limit_speed_decel, self.min_set_speed_clu)
      current_max_speed_clu = min(current_max_speed_clu, prelimit_clu)

    # 2. Apply limit speed
    if apply_limit_speed >= self.min_set_speed_clu:
      current_max_speed_clu = min(current_max_speed_clu, apply_limit_speed)

    # 3. Lead limit speed
    lead_speed_clu = self._get_long_lead_speed(sm['radarState'], cluster_speed)
    if self.min_set_speed_clu <= lead_speed_clu < current_max_speed_clu:
      current_max_speed_clu = min(current_max_speed_clu, lead_speed_clu)

    # 4. Curve limit speed
    if sm.frame % 20 == 0:
      self._calculate_curve_speed(sm['modelV2'], CS.vEgo, v_cruise_kph)
    current_max_speed_clu = min(current_max_speed_clu, self.curve_speed_clu)

    # 5. Steering angle based speed limit
    steer_limited_speed_clu = self._calculate_steer_based_speed(CS.vEgo, CS.steeringAngleDeg)
    current_max_speed_clu = min(current_max_speed_clu, steer_limited_speed_clu)

    self._update_max_speed_clu(int(round(current_max_speed_clu)), is_limit_zone)

  def _get_long_lead_speed(self, radar_state, cluster_speed_clu: float) -> float:
    lead = radar_state.leadOne
    lead_distance_buffer = 5.
    distance = lead.dRel - lead_distance_buffer
    relative_speed = lead.vRel
    lead_decay_factor = 22.
    lead_accel_gain = 1.2
    min_relative_speed = -1.0

    if not self.long_control or not lead.status:
        return 0.

    is_valid_deceleration = (
      0 < distance < -relative_speed * lead_decay_factor and
      relative_speed < min_relative_speed
    )
    if not is_valid_deceleration:
      return 0.

    time = distance / relative_speed if abs(relative_speed) > 1e-3 else 0.1
    deceleration_ms = -relative_speed / time
    speed_delta_clu = self.conv_to_clu(deceleration_ms) * lead_accel_gain
    new_speed_clu = cluster_speed_clu + speed_delta_clu
    lead_speed_clu = max(new_speed_clu, self.min_set_speed_clu)

    return lead_speed_clu

  def _calculate_curve_speed(self, model_msg, current_speed_ms: float, v_cruise_kph: float):
    no_limit_speed = 255.
    trajectory_size = ModelConstants.IDX_N

    if len(model_msg.position.x) != trajectory_size or len(model_msg.position.y) != trajectory_size:
      self.curve_speed_clu = no_limit_speed
      return

    x = model_msg.position.x
    y = model_msg.position.y
    dy = np.gradient(y, x)
    d2y = np.gradient(dy, x)
    curv = d2y / (1 + dy ** 2) ** 1.5

    far_start_ratio = 0.3
    far_end_ratio = 0.95
    start_index = int(trajectory_size * far_start_ratio)
    end_index = int(trajectory_size * far_end_ratio)

    curv_segment = curv[start_index:end_index]
    curv_segment_abs = np.abs(curv_segment)

    curve_a_y_slope = 0.0375
    a_y_max = 2.975 - current_speed_ms * curve_a_y_slope
    v_curvature = np.sqrt(a_y_max / np.clip(curv_segment_abs, 1e-4, None))
    model_speed_ms = float(np.mean(v_curvature))

    min_curve_speed_ms = np.interp(current_speed_ms, [0.0, self.conv_to_ms(60.0)], [self.conv_to_ms(30.0), self.conv_to_ms(50.0)])
    model_based_speed_ms = float(max(model_speed_ms, min_curve_speed_ms)) \
      if not math.isnan(model_speed_ms) and model_speed_ms < current_speed_ms else no_limit_speed

    orientation_rate = np.array(model_msg.orientationRate.z)
    velocity = np.array(model_msg.velocity.x)
    predicted_lat_acc = float(np.max(np.abs(orientation_rate * velocity)))
    acc_based_curvature = predicted_lat_acc / max(current_speed_ms, 1.0) ** 2

    acc_based_speed_ms = no_limit_speed
    if acc_based_curvature > 1e-4:
      temp_acc_speed = np.sqrt(a_y_max / acc_based_curvature)
      acc_based_speed_ms = float(max(temp_acc_speed, min_curve_speed_ms)) \
        if temp_acc_speed < current_speed_ms else no_limit_speed

    candidates_ms = [s for s in [model_based_speed_ms, acc_based_speed_ms] if s != no_limit_speed]
    calculated_curve_speed_ms = min(candidates_ms) if candidates_ms else no_limit_speed
    final_curve_speed_ms = min(calculated_curve_speed_ms, self.conv_to_ms(v_cruise_kph))
    self.curve_speed_clu = self.conv_to_clu(final_curve_speed_ms)

  def _calculate_steer_based_speed(self, current_speed_ms: float, steering_angle_deg: float) -> float:
    steering_decel_angle_deg = 60.0
    no_limit_speed = 255.

    if abs(steering_angle_deg) >= steering_decel_angle_deg * 2:
      steer_based_speed_ms = max(min(current_speed_ms * 0.70, current_speed_ms - 3.0), self.conv_to_ms(V_CRUISE_MIN))
      return self.conv_to_clu(steer_based_speed_ms)
    elif abs(steering_angle_deg) >= steering_decel_angle_deg:
      steer_based_speed_ms = max(min(current_speed_ms * 0.85, current_speed_ms - 3.0), self.conv_to_ms(V_CRUISE_MIN))
      return self.conv_to_clu(steer_based_speed_ms)

    return no_limit_speed

  def _calculate_target_speed(self, CS, cluster_speed_clu: float, v_cruise_kph: float, cruise_btn_pressed: bool):
    syncing = CS.gasPressed and not cruise_btn_pressed
    sync_margin = 3.

    if not self.long_control:
      if syncing and cluster_speed_clu + sync_margin > self.to_current_unit(v_cruise_kph):
        set_speed = np.clip(cluster_speed_clu + sync_margin, V_CRUISE_INITIAL, self.max_set_speed_clu)
        v_cruise_kph = int(round(self.to_current_unit(set_speed)))

      self.target_speed_clu = self.to_current_unit(v_cruise_kph)
      if self.max_speed_clu > V_CRUISE_INITIAL:
        self.target_speed_clu = np.clip(self.target_speed_clu, V_CRUISE_INITIAL, self.max_speed_clu)

    elif CS.cruiseState.enabled and syncing:
      if cluster_speed_clu + sync_margin > self.to_current_unit(v_cruise_kph):
        set_speed = np.clip(cluster_speed_clu + sync_margin, self.min_set_speed_clu, self.max_set_speed_clu)
        self.target_speed_clu = int(round(self.to_current_unit(set_speed)))
        CruiseStateManager.instance().speed = self.conv_to_ms(set_speed)

  def _update_max_speed_clu(self, current_max_speed_clu: float, is_limit_zone: bool):
    if not self.long_control or self.max_speed_clu <= 0 or is_limit_zone:
      self.max_speed_clu = current_max_speed_clu
    else:
      error = current_max_speed_clu - self.max_speed_clu
      kp = 0.01
      self.max_speed_clu += error * kp

  def _get_button_to_adjust_speed(self, current_set_speed: float) -> Buttons:
    if self.target_speed_clu < V_CRUISE_INITIAL:
      return Buttons.NONE
    error = self.target_speed_clu - current_set_speed
    if abs(error) < 0.9:
      return Buttons.NONE
    return Buttons.RES_ACCEL if error > 0 else Buttons.SET_DECEL

  def update_v_cruise(self, CS, sm, enabled: bool):
    if CS.cruiseState.enabled:
      if not self.long_control or not self.CP.pcmCruise:
        v_cruise_kph = self.v_cruise_helper.update_v_cruise(CS, enabled, self.is_metric)
      else:
        v_cruise_kph = self.conv_to_clu(CS.cruiseState.speed)
    else:
      v_cruise_kph = V_CRUISE_UNSET

    if self.prev_cruise_enabled != CS.cruiseState.enabled:
      self.prev_cruise_enabled = CS.cruiseState.enabled
      if CS.cruiseState.enabled:
        if not self.CP.pcmCruise:
          v_cruise_kph = self.v_cruise_helper.initialize_v_cruise(CS, self.experimental_mode)
        else:
          v_cruise_kph = self.conv_to_clu(CS.cruiseState.speed)

    self.real_set_speed_kph = v_cruise_kph
    if CS.cruiseState.enabled and 1 < CS.cruiseState.speed < V_CRUISE_UNSET:
      cluster_speed_clu = self.conv_to_clu(CS.vEgoCluster)
      self._calculate_max_speed(CS, sm, cluster_speed_clu, v_cruise_kph)
      self.cruise_speed_kph = float(np.clip(v_cruise_kph, V_CRUISE_MIN, self.to_current_unit(self.max_speed_clu)))
      self._calculate_target_speed(CS, cluster_speed_clu, self.real_set_speed_kph, self.CI.CS.cruise_buttons[-1] != Buttons.NONE)

      if CruiseStateManager.instance().cruise_state_control:
        self.cruise_speed_kph = min(self.cruise_speed_kph, max(self.real_set_speed_kph, V_CRUISE_MIN))

    else:
      self.reset()

    self.v_cruise_helper.v_cruise_kph = v_cruise_kph

  def spam_message(self, CS, can_sends):
    ascc_enabled = CS.cruiseState.enabled and 1 < CS.cruiseState.speed < V_CRUISE_UNSET and not CS.brakePressed
    btn_pressed = self.CI.CS.cruise_buttons[-1] != Buttons.NONE

    if not self.long_control:
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
        current_set_speed_clu = int(round(self.conv_to_clu(CS.cruiseState.speed)))
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
      elif self.long_control and self.target_speed_clu >= V_CRUISE_INITIAL:
        self.target_speed_clu = 0.
    elif self.long_control:
      self.target_speed_clu = 0.
