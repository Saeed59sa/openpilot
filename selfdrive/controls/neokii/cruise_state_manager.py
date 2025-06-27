import threading
import numpy as np

from opendbc.car import structs
from openpilot.common.conversions import Conversions as CV
from openpilot.common.params import Params
from openpilot.selfdrive.car.cruise import V_CRUISE_INITIAL, V_CRUISE_MIN, V_CRUISE_MAX, IMPERIAL_INCREMENT, CRUISE_LONG_PRESS
from openpilot.selfdrive.controls.neokii.navi_controller import SpeedLimiter

ButtonType = structs.CarState.ButtonEvent.Type
GearShifter = structs.CarState.GearShifter

class CruiseStateManager:
  def __init__(self):
    self.params = Params()

    self.is_metric = self.params.get_bool('IsMetric')
    self.cruise_state_control = self.params.get_bool('CruiseStateControl')

    self.available = False
    self.enabled = False
    self.speed = self.conv_to_ms(V_CRUISE_INITIAL)

    self.prev_btn = ButtonType.unknown
    self.btn_count = 0
    self.btn_long_pressed = False
    self.prev_brake_pressed = False

    self.prev_main_buttons = 0

  def conv_to_ms(self, speed):
    return speed * CV.KPH_TO_MS if self.is_metric else speed * CV.MPH_TO_MS

  def conv_to_clu(self, speed):
    return speed * CV.MS_TO_KPH if self.is_metric else speed * CV.MS_TO_MPH

  @classmethod
  def instance(cls):
    if not hasattr(cls, "_instance"):
      cls._instance = cls()
    return cls._instance

  def reset_available(self):
    threading.Timer(3.0, lambda: setattr(self, 'available', True)).start()

  def update(self, CS, main_buttons):
    btn = self.update_buttons(CS)
    if btn != ButtonType.unknown:
      self.update_cruise_state(CS, int(round(self.conv_to_clu(self.speed))), btn)

    if main_buttons[-1] != self.prev_main_buttons and main_buttons[-1]:
      self.available = not self.available

    self.prev_main_buttons = main_buttons[-1]

    if not self.available:
      self.enabled = False

    if not self.prev_brake_pressed and CS.brakePressed:
      self.enabled = False
    self.prev_brake_pressed = CS.brakePressed

    if CS.gearShifter == GearShifter.park:
      self.enabled = False

    CS.cruiseState.available = self.available
    CS.cruiseState.enabled = self.enabled
    CS.cruiseState.standstill = False
    CS.cruiseState.speed = float(self.speed)

  def update_buttons(self, CS):
    btn = ButtonType.unknown

    if self.btn_count > 0:
      self.btn_count += 1

    for b in CS.buttonEvents:
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

    if self.btn_count > CRUISE_LONG_PRESS:
      self.btn_long_pressed = True
      btn = self.prev_btn
      self.btn_count %= CRUISE_LONG_PRESS

    return btn

  def update_cruise_state(self, CS, v_cruise_kph, btn):
    v_cruise_delta = 10 if self.is_metric else IMPERIAL_INCREMENT * 5
    road_limit_speed_nda = SpeedLimiter.instance().get_road_limit_speed()
    road_limit_speed_stock = CS.exState.navLimitSpeed

    if road_limit_speed_nda is not None and road_limit_speed_nda > 0:
      road_limit_speed = road_limit_speed_nda
    elif road_limit_speed_stock is not None and road_limit_speed_stock > 0:
      road_limit_speed = road_limit_speed_stock
    else:
      road_limit_speed = None

    if self.enabled:
      if not self.btn_long_pressed:
        if btn == ButtonType.accelCruise:
          v_cruise_kph += 1 if self.is_metric else IMPERIAL_INCREMENT
        elif btn == ButtonType.decelCruise:
          v_cruise_kph -= 1 if self.is_metric else IMPERIAL_INCREMENT
      else:
        if btn == ButtonType.accelCruise:
          v_cruise_kph += v_cruise_delta - v_cruise_kph % v_cruise_delta
        elif btn == ButtonType.decelCruise:
          v_cruise_kph -= v_cruise_delta - -v_cruise_kph % v_cruise_delta
    elif not self.enabled and self.available and CS.gearShifter != GearShifter.park:
      if not self.btn_long_pressed:
        if btn == ButtonType.decelCruise:
          self.enabled = True
          v_cruise_kph = max(np.clip(round(self.conv_to_clu(CS.vEgoCluster)), V_CRUISE_MIN, V_CRUISE_MAX),
                             V_CRUISE_INITIAL)
        elif btn == ButtonType.accelCruise:
          self.enabled = True
          v_cruise_kph = max(np.clip(round(self.conv_to_clu(self.speed)), V_CRUISE_INITIAL, V_CRUISE_MAX),
                             round(self.conv_to_clu(CS.vEgoCluster)))

          if road_limit_speed is not None and V_CRUISE_INITIAL < road_limit_speed < V_CRUISE_MAX:
            v_cruise_kph = max(v_cruise_kph, road_limit_speed)

    if btn == ButtonType.gapAdjustCruise:
      #if not self.btn_long_pressed:
      if self.btn_long_pressed:
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
        if road_limit_speed is not None:
          if self.enabled:
            v_cruise_kph = road_limit_speed
          elif not self.enabled and self.available and CS.gearShifter != GearShifter.park:
            self.enabled = True
            v_cruise_kph = road_limit_speed
        else:
          if not self.enabled and self.available and CS.gearShifter != GearShifter.park:
            self.enabled = True
            v_cruise_kph = max(np.clip(round(self.conv_to_clu(self.speed)), V_CRUISE_INITIAL, V_CRUISE_MAX),
                               round(self.conv_to_clu(CS.vEgoCluster)))
      else:
        self.enabled = False
        self.available = False
        self.reset_available()

    v_cruise_kph = np.clip(round(v_cruise_kph), V_CRUISE_MIN, V_CRUISE_MAX)
    self.speed = self.conv_to_ms(v_cruise_kph)
