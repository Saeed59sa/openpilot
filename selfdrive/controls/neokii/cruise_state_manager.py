import capnp
import threading

from typing import Dict
from cereal import car, log
from openpilot.common.numpy_fast import clip
from openpilot.common.conversions import Conversions as CV
from openpilot.common.params import Params
from openpilot.selfdrive.car.cruise import V_CRUISE_ENABLE_MIN, V_CRUISE_MAX
from openpilot.selfdrive.controls.neokii.navi_controller import SpeedLimiter
from openpilot.selfdrive.selfdrived.events import Events

V_CRUISE_MIN_CRUISE_STATE = 10
V_CRUISE_DELTA_MI = 5 * CV.MPH_TO_KPH
V_CRUISE_DELTA_KM = 10

ButtonType = car.CarState.ButtonEvent.Type
EventName = log.OnroadEvent.EventName

def create_button_event(cur_but: int, prev_but: int, buttons_dict: Dict[int, capnp.lib.capnp._EnumModule],
                        unpressed: int = 0) -> capnp.lib.capnp._DynamicStructBuilder:
  if cur_but != unpressed:
    be = car.CarState.ButtonEvent(pressed=True)
    but = cur_but
  else:
    be = car.CarState.ButtonEvent(pressed=False)
    but = prev_but
  be.type = buttons_dict.get(but, ButtonType.unknown)
  return be

class CruiseStateManager:
  def __init__(self):
    self.params = Params()

    self.available = False
    self.enabled = False
    self.speed = V_CRUISE_ENABLE_MIN * CV.KPH_TO_MS
    self.lead_distance_bars = self.get_lead_distance_bars()

    self.prev_speed = 0
    self.prev_cruise_button = 0
    self.button_events = None

    self.prev_btn = ButtonType.unknown
    self.btn_count = 0
    self.btn_long_pressed = False
    self.is_metric = self.params.get_bool('IsMetric')
    self.prev_brake_pressed = False

    self.is_cruise_enabled = False
    self.cruise_state_control = self.params.get_bool('CruiseStateControl')

  def get_lead_distance_bars(self):
    gap = self.params.get('SccGapAdjust')
    bars = int(gap) if gap is not None else 4
    return clip(bars, 1, 4)

  @classmethod
  def instance(cls):
    if not hasattr(cls, "_instance"):
      cls._instance = cls()
    return cls._instance

  def reset_available(self):
    threading.Timer(1.0, self.set_available_true).start()

  def set_available_true(self):
    self.available = True

  def update(self, CS, cruise_buttons, buttons_dict, available, enabled):
    cruise_button = cruise_buttons[-1]
    if cruise_button != self.prev_cruise_button:
      self.button_events = [create_button_event(cruise_button, self.prev_cruise_button, buttons_dict)]
      if cruise_button != 0 and self.prev_cruise_button != 0:
        self.button_events.append(create_button_event(0, self.prev_cruise_button, buttons_dict))
        self.prev_cruise_button = 0
      else:
        self.prev_cruise_button = cruise_button

    button = self.update_buttons()
    if button != ButtonType.unknown:
      self.update_cruise_state(CS, int(round(self.speed * CV.MS_TO_KPH)), button)

    if not self.available:
      self.enabled = False

    if self.prev_brake_pressed != CS.brakePressed and CS.brakePressed:
      self.enabled = False
    self.prev_brake_pressed = CS.brakePressed

    CS.cruiseState.available = available and self.available
    CS.cruiseState.enabled = enabled and self.enabled
    CS.cruiseState.standstill = False
    CS.cruiseState.speed = self.speed
    CS.cruiseState.leadDistanceBars = self.lead_distance_bars

    self.update_available_state(CS)

  def update_available_state(self, CS):
    if not CS.cruiseState.enabled:
      if CS.cruiseState.speed > 5:
        self.available = True

  def update_buttons(self):
    if self.button_events is None:
      return ButtonType.unknown

    btn = ButtonType.unknown

    if self.btn_count > 0:
      self.btn_count += 1

    for b in self.button_events:
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
    events = Events()

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
    else:
      if not self.btn_long_pressed:
        if btn == ButtonType.decelCruise and not self.enabled:
          if not self.available:
            events.add(EventName.wrongCarMode)
          else:
            self.enabled = True

          v_cruise_kph = max(clip(round(CS.vEgoCluster * CV.MS_TO_KPH, 1), V_CRUISE_MIN_CRUISE_STATE, V_CRUISE_MAX), V_CRUISE_ENABLE_MIN)
        elif btn == ButtonType.accelCruise and not self.enabled:
          if not self.available:
            events.add(EventName.wrongCarMode)
          else:
            self.enabled = True

          v_cruise_kph = clip(round(self.speed * CV.MS_TO_KPH, 1), V_CRUISE_ENABLE_MIN, V_CRUISE_MAX)
          v_cruise_kph = max(v_cruise_kph, round(CS.vEgoCluster * CV.MS_TO_KPH, 1))
          road_limit_speed = SpeedLimiter.instance().get_road_limit_speed()
          if V_CRUISE_ENABLE_MIN < road_limit_speed < V_CRUISE_MAX:
            v_cruise_kph = max(v_cruise_kph, road_limit_speed)

    if btn == ButtonType.gapAdjustCruise:
      if not self.btn_long_pressed:
        self.lead_distance_bars -= 1
        self.lead_distance_bars = clip(self.lead_distance_bars, 1, 4)
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
        self.available = not self.available
      else:
        self.enabled = False
        self.available = False
        self.reset_available()

    v_cruise_kph = clip(round(v_cruise_kph, 1), V_CRUISE_MIN_CRUISE_STATE, V_CRUISE_MAX)
    self.speed = v_cruise_kph * CV.KPH_TO_MS
