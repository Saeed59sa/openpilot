import threading
import numpy as np

from cereal import car
from openpilot.common.conversions import Conversions as CV
from openpilot.common.params import Params
from openpilot.selfdrive.car.cruise import V_CRUISE_ENABLE_MIN, V_CRUISE_MAX
from openpilot.selfdrive.controls.neokii.navi_controller import SpeedLimiter

V_CRUISE_MIN_CRUISE_STATE = 10
V_CRUISE_DELTA_MI = 5 * CV.MPH_TO_KPH
V_CRUISE_DELTA_KM = 10

ButtonType = car.CarState.ButtonEvent.Type

class CruiseStateManager:
  def __init__(self):
    self.params = Params()

    self.available = True
    self.enabled = False
    self.speed = V_CRUISE_ENABLE_MIN * CV.KPH_TO_MS
    self.lead_distance_bars = self.get_lead_distance_bars()

    self.prev_btn = ButtonType.unknown
    self.btn_count = 0
    self.btn_long_pressed = False
    self.prev_brake_pressed = False

    self.is_cruise_enabled = False

    self.is_metric = self.params.get_bool('IsMetric')
    self.cruise_state_control = self.params.get_bool('CruiseStateControl')

  def get_lead_distance_bars(self):
    gap = self.params.get('SccGapAdjust')
    bars = int(gap) if gap is not None else 4
    return np.clip(bars, 1, 4)

  @classmethod
  def instance(cls):
    if not hasattr(cls, "_instance"):
      cls._instance = cls()
    return cls._instance

  def reset_available(self):
    threading.Timer(3.0, lambda: setattr(self, 'available', True)).start()

  def update(self, CS, enabled):
    btn = self.update_buttons(CS)
    if btn != ButtonType.unknown:
      self.update_cruise_state(CS, int(round(self.speed * CV.MS_TO_KPH)), btn)

    if not self.prev_brake_pressed and CS.brakePressed:
      self.enabled = False
    self.prev_brake_pressed = CS.brakePressed

    CS.cruiseState.available = self.available
    CS.cruiseState.enabled = enabled and self.enabled
    CS.cruiseState.standstill = False
    CS.cruiseState.speed = float(self.speed)
    CS.cruiseState.leadDistanceBars = int(self.lead_distance_bars)

  def update_buttons(self, CS):
    buttonEvents = CS.buttonEvents
    btn = ButtonType.unknown

    if self.btn_count > 0:
      self.btn_count += 1

    for b in buttonEvents:
      if (
        b.pressed and self.btn_count == 0 and b.type in
        [
          ButtonType.accelCruise,
          ButtonType.decelCruise,
          ButtonType.gapAdjustCruise,
          ButtonType.cancel,
          ButtonType.lfaButton
        ]
      ):
        self.btn_count = 1
        self.prev_btn = b.type
      elif not b.pressed and self.btn_count > 0:
        if not self.btn_long_pressed:
          btn = b.type
        self.btn_long_pressed = False
        self.btn_count = 0

    if self.btn_count > 70:
      self.btn_long_pressed = True
      btn = self.prev_btn
      self.btn_count %= 70

    return btn

  def update_cruise_state(self, CS, v_cruise_kph, btn):
    v_cruise_delta = V_CRUISE_DELTA_KM if self.is_metric else V_CRUISE_DELTA_MI

    if self.enabled:
      if not self.btn_long_pressed:
        if btn == ButtonType.accelCruise:
          v_cruise_kph += 1 if self.is_metric else 1 * CV.MPH_TO_KPH
        elif btn == ButtonType.decelCruise:
          v_cruise_kph -= 1 if self.is_metric else 1 * CV.MPH_TO_KPH
      else:
        if btn == ButtonType.accelCruise:
          v_cruise_kph += v_cruise_delta - v_cruise_kph % v_cruise_delta
        elif btn == ButtonType.decelCruise:
          v_cruise_kph -= v_cruise_delta - -v_cruise_kph % v_cruise_delta
    elif not self.enabled and self.available:
      if not self.btn_long_pressed:
        if btn == ButtonType.decelCruise:
          self.enabled = True
          v_cruise_kph = max(np.clip(round(CS.vEgoCluster * CV.MS_TO_KPH, 1), V_CRUISE_MIN_CRUISE_STATE, V_CRUISE_MAX), V_CRUISE_ENABLE_MIN)
        elif btn == ButtonType.accelCruise:
          self.enabled = True
          v_cruise_kph = np.clip(round(self.speed * CV.MS_TO_KPH, 1), V_CRUISE_ENABLE_MIN, V_CRUISE_MAX)
          v_cruise_kph = max(v_cruise_kph, round(CS.vEgoCluster * CV.MS_TO_KPH, 1))
          road_limit_speed = SpeedLimiter.instance().get_road_limit_speed()
          if V_CRUISE_ENABLE_MIN < road_limit_speed < V_CRUISE_MAX:
            v_cruise_kph = max(v_cruise_kph, road_limit_speed)

    if btn == ButtonType.gapAdjustCruise:
      if not self.btn_long_pressed:
        self.lead_distance_bars -= 1
        self.lead_distance_bars = np.clip(self.lead_distance_bars, 1, 4)
        self.params.put_nonblocking("SccGapAdjust", str(self.lead_distance_bars))
      else:
        self.params.put_bool("ExperimentalMode", not self.params.get_bool("ExperimentalMode"))

    if btn == ButtonType.cancel:
      if not self.btn_long_pressed:
        self.enabled = False
      else:
        self.enabled = False
        self.available = False
        self.reset_available()

    if btn == ButtonType.lfaButton:
      if not self.btn_long_pressed:
        self.enabled = False
        self.available = False
        self.reset_available()

    v_cruise_kph = np.clip(round(v_cruise_kph, 1), V_CRUISE_MIN_CRUISE_STATE, V_CRUISE_MAX)
    self.speed = v_cruise_kph * CV.KPH_TO_MS
