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


class SpeedController:
  def __init__(self, CP, CI):
    self.CP = CP
    self.CI = CI

    self.long_control = CP.openpilotLongitudinalControl

    self.is_metric = Params().get_bool('IsMetric')
    self.experimental_mode = Params().get_bool("ExperimentalMode") and self.long_control

    self.speed_conv_to_ms = CV.KPH_TO_MS if self.is_metric else CV.MPH_TO_MS
    self.speed_conv_to_clu = CV.MS_TO_KPH if self.is_metric else CV.MS_TO_MPH

    self.min_set_speed_clu = self._kph_to_clu(V_CRUISE_MIN) if CruiseStateManager.instance().cruise_state_control else self._kph_to_clu(V_CRUISE_INITIAL)
    self.max_set_speed_clu = self._kph_to_clu(V_CRUISE_MAX)

    self.btn = Buttons.NONE
    self.target_speed = 0.
    self.max_speed_clu = 0.
    self.curve_speed_ms = 0.
    self.cruise_speed_kph = 0.
    self.real_set_speed_kph = 0.
    self.v_cruise_kph_last = 0
    self.v_cruise_kph = V_CRUISE_UNSET

    self.active_cam = False
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

  def _kph_to_clu(self, kph):
    return int(kph * CV.KPH_TO_MS * self.speed_conv_to_clu)

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
    self.target_speed = 0.
    self.max_speed_clu = 0.
    self.curve_speed_ms = 0.

  def _cal_max_speed(self, CS, sm, clu_speed, v_cruise_kph):
    apply_limit_speed, road_limit_speed, left_dist, first_started = (
      SpeedLimiter.instance().get_max_speed(clu_speed, self.is_metric))

    self._cal_curve_speed(sm, CS)

    curve_limited_speed = min(v_cruise_kph * CV.KPH_TO_MS, self.curve_speed_ms)
    max_speed_clu = int(round(curve_limited_speed * self.speed_conv_to_clu))

    self.active_cam = road_limit_speed > 0 and left_dist > 0

    if apply_limit_speed >= self._kph_to_clu(10):
      if first_started:
        self.max_speed_clu = clu_speed
      max_speed_clu = min(max_speed_clu, apply_limit_speed)

    lead_speed, lead = self._get_long_lead_speed(clu_speed, sm)

    if lead is not None and self.min_set_speed_clu <= lead_speed < max_speed_clu:
      max_speed_clu = lead_speed
      self.max_speed_clu = min(self.max_speed_clu, clu_speed + 3.)

    kp = np.interp(clu_speed, [0, 30], [0.015, 0.005])
    if not self.long_control or self.max_speed_clu <= 0:
      self.max_speed_clu = int(round(max_speed_clu))
    else:
      error = int(round(max_speed_clu)) - self.max_speed_clu
      self.max_speed_clu += error * kp

    return max_speed_clu

  def _get_long_lead_speed(self, clu_speed, sm):
    radar = sm['radarState']
    lead = radar.leadOne if radar.leadOne.status else None
    lead_decay_factor = 22.
    lead_accel_gain = 1.2

    if self.long_control and lead is not None:
      d = lead.dRel - 5.
      if 0. < d < -lead.vRel * lead_decay_factor and lead.vRel < -1.:
        t = d / lead.vRel if abs(lead.vRel) > 1e-3 else 0.1
        accel = -(lead.vRel / t) * self.speed_conv_to_clu * lead_accel_gain
        if accel < 0.:
          return max(clu_speed + accel, self.min_set_speed_clu), lead

    return 0, None

  def _cal_curve_speed(self, sm, CS):
    speed = CS.vEgo
    steering_angle = abs(CS.steeringAngleDeg)
    model_msg = sm['modelV2']
    no_limit_speed = V_CRUISE_UNSET
    trajectory_size = ModelConstants.IDX_N

    if sm.frame % 10 != 0:
      return

    if len(model_msg.position.x) != trajectory_size or len(model_msg.position.y) != trajectory_size:
      self.curve_speed_ms = no_limit_speed
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
    a_y_max = 2.975 - speed * curve_a_y_slope
    v_curvature = np.sqrt(a_y_max / np.clip(curv_segment_abs, 1e-4, None))
    model_speed = float(np.mean(v_curvature))

    min_curve_speed = np.interp(speed, [0.0, 60.0 * CV.KPH_TO_MS], [30.0 * CV.KPH_TO_MS, 50.0 * CV.KPH_TO_MS])
    model_based_speed = float(max(model_speed, min_curve_speed)) if not math.isnan(
      model_speed) and model_speed < speed else no_limit_speed

    orientation_rate = np.array(model_msg.orientationRate.z)
    velocity = np.array(model_msg.velocity.x)
    predicted_lat_acc = float(np.max(np.abs(orientation_rate * velocity)))
    acc_based_curvature = predicted_lat_acc / max(speed, 1.0) ** 2

    if acc_based_curvature > 1e-4:
      acc_based_speed = np.sqrt(a_y_max / acc_based_curvature)
      acc_based_speed = float(max(acc_based_speed, min_curve_speed)) if acc_based_speed < speed else no_limit_speed
    else:
      acc_based_speed = no_limit_speed

    steering_decel_angle_deg = 60.0
    steer_based_speed = no_limit_speed
    if steering_angle >= steering_decel_angle_deg:
      steer_based_speed = max(min(speed * 0.85, speed - 3.0), min_curve_speed)

    candidates = [s for s in [model_based_speed, acc_based_speed, steer_based_speed] if s != no_limit_speed]
    self.curve_speed_ms = min(candidates) if candidates else no_limit_speed

  def _cal_target_speed(self, CS, clu_speed, v_cruise_kph, cruise_btn_pressed):
    override_speed = -1
    syncing = CS.gasPressed and not cruise_btn_pressed
    sync_margin = 3.

    if not self.long_control:
      if syncing and clu_speed + sync_margin > self._kph_to_clu(v_cruise_kph):
        set_speed = np.clip(clu_speed + sync_margin, V_CRUISE_INITIAL, self.max_set_speed_clu)
        v_cruise_kph = int(round(set_speed * self.speed_conv_to_ms * CV.MS_TO_KPH))
        override_speed = v_cruise_kph

      self.target_speed = self._kph_to_clu(v_cruise_kph)
      if self.max_speed_clu > V_CRUISE_INITIAL:
        self.target_speed = np.clip(self.target_speed, V_CRUISE_INITIAL, self.max_speed_clu)

    elif CS.cruiseState.enabled and syncing:
      if clu_speed + sync_margin > self._kph_to_clu(v_cruise_kph):
        set_speed = np.clip(clu_speed + sync_margin, self.min_set_speed_clu, self.max_set_speed_clu)
        self.target_speed = set_speed
        CruiseStateManager.instance().speed = set_speed * self.speed_conv_to_ms

    return override_speed

  def _get_button(self, current_set_speed):
    if self.target_speed < V_CRUISE_INITIAL:
      return Buttons.NONE
    error = self.target_speed - current_set_speed
    if abs(error) < 0.9:
      return Buttons.NONE
    return Buttons.RES_ACCEL if error > 0 else Buttons.SET_DECEL

  def update_v_cruise(self, CS, sm, enabled):
    self.v_cruise_kph_last = self.v_cruise_kph
    v_cruise_kph = self.v_cruise_kph

    if CS.cruiseState.enabled:
      if not self.CP.openpilotLongitudinalControl or not self.CP.pcmCruise:
        v_cruise_kph = self.v_cruise_helper.update_v_cruise(CS, enabled, self.is_metric)
        #v_cruise_kph = self._update_cruise_button(v_cruise_kph, CS.buttonEvents, enabled)
      else:
        v_cruise_kph = CS.cruiseState.speed * CV.MS_TO_KPH
    else:
      v_cruise_kph = V_CRUISE_UNSET

    if self.prev_cruise_enabled != CS.cruiseState.enabled:
      self.prev_cruise_enabled = CS.cruiseState.enabled
      if CS.cruiseState.enabled:
        if not self.CP.pcmCruise:
          v_cruise_kph = self.v_cruise_helper.initialize_v_cruise(CS, self.experimental_mode)
          #v_cruise_kph = self._initialize_v_cruise(CS)
        else:
          v_cruise_kph = CS.cruiseState.speed * CV.MS_TO_KPH

    self.real_set_speed_kph = v_cruise_kph
    if CS.cruiseState.enabled:
      clu_speed = CS.vEgoCluster * self.speed_conv_to_clu
      self._cal_max_speed(CS, sm, clu_speed, v_cruise_kph)
      self.cruise_speed_kph = float(np.clip(v_cruise_kph, V_CRUISE_MIN, self.max_speed_clu * self.speed_conv_to_ms * CV.MS_TO_KPH))

      if CruiseStateManager.instance().cruise_state_control:
        self.cruise_speed_kph = min(self.cruise_speed_kph, max(self.real_set_speed_kph, V_CRUISE_MIN))

      override_speed = self._cal_target_speed(CS, clu_speed, self.real_set_speed_kph, self.CI.CS.cruise_buttons[-1] != Buttons.NONE)
      if override_speed > 0:
        v_cruise_kph = override_speed

    else:
      self.reset()

    self.v_cruise_kph = v_cruise_kph
    self._update_message(CS)

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
        current_set_speed_clu = int(round(CS.cruiseState.speed * self.speed_conv_to_clu))
        self.btn = self._get_button(current_set_speed_clu)
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
      elif self.long_control and self.target_speed >= V_CRUISE_INITIAL:
        self.target_speed = 0.
    elif self.long_control:
      self.target_speed = 0.

  def _update_message(self, CS):
    exState = CS.exState
    exState.vCruiseKph = float(self.v_cruise_kph)
