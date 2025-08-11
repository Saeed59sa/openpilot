# -*- coding: utf-8 -*-
# =============================================================================
# SDpilot cruise helper (single-file version)
# - Dynamic V_CRUISE_MAX from Params (default 160 kph)
# - Cluster/UI spoof: show chosen max even if actual/PCM cap is lower
# - Minimal dependencies; compatible with typical OP/SDpilot forks
# =============================================================================

from __future__ import annotations
import math
from typing import Optional

# --- imports (support both openpilot/common layouts) ---
try:
  # Newer layout
  from openpilot.cereal import car
  from openpilot.common.conversions import Conversions as CV
  from openpilot.common.params import Params
except Exception:
  # Older layout / forks
  from cereal import car
  try:
    from common.conversions import Conversions as CV
  except Exception:
    from openpilot.common.conversions import Conversions as CV
  try:
    from common.params import Params
  except Exception:
    from openpilot.common.params import Params

# =========================
# Cruise limits (in kph)
# =========================
V_CRUISE_MIN = 8
V_CRUISE_UNSET = 255
V_CRUISE_INITIAL = 40
V_CRUISE_INITIAL_EXPERIMENTAL_MODE = 105
IMPERIAL_INCREMENT = round(CV.MPH_TO_KPH, 1)  # ≈ 1.6

ButtonEvent = car.CarState.ButtonEvent
ButtonType = car.CarState.ButtonEvent.Type

CRUISE_LONG_PRESS = 50  # ~5s @ 0.1s tick; adjust if needed

CRUISE_NEAREST_FUNC = {
  ButtonType.accelCruise: math.ceil,
  ButtonType.decelCruise: math.floor,
}

CRUISE_INTERVAL_SIGN = {
  ButtonType.accelCruise: +1,
  ButtonType.decelCruise: -1,
}

# ---------------- helpers ----------------
def _safe_int(b, default: int) -> int:
  try:
    return int(b) if b is not None else default
  except Exception:
    return default

def _read_user_max_kph(default=160) -> int:
  """
  Read user-selectable cruise max from Params (string int),
  clamp to a sane range to avoid accidental extremes.
  """
  v = _safe_int(Params().get("UserVCruiseMaxKph"), default)
  return max(120, min(180, v))

# ----- User override for max -----
_USER_MAX_KPH = _read_user_max_kph(160)

# Public constants that other modules may import
V_CRUISE_MAX = _USER_MAX_KPH
try:
  V_CRUISE_MAX_KPH = _USER_MAX_KPH
except Exception:
  pass
try:
  # Some places expect a value in m/s; keep both styles available
  V_CRUISE_MAX_VISION = _USER_MAX_KPH * CV.KPH_TO_MS
except Exception:
  pass

def _nearest_kph(value_kph: float, is_metric: bool) -> float:
  step = 1.0 if is_metric else IMPERIAL_INCREMENT
  return round(value_kph / step) * step

def _cluster_display_kph(actual_set_kph: float) -> float:
  """
  Value to SHOW on cluster/UI.
  Show at least the user-selected max (e.g., 160),
  but never below the actual set speed for consistency.
  """
  return max(float(actual_set_kph), float(_USER_MAX_KPH))

# ---------------- main class ----------------
class VCruiseHelper:
  """
  Minimal, drop-in VCruise helper.
  Exposes:
    - v_cruise_kph: numeric set speed (controller view)
    - v_cruise_cluster_kph: what we show on cluster/UI (spoofed up to user max)
    - initialize_v_cruise(CS, is_metric, experimental_mode)
    - update_v_cruise(CS, enabled, is_metric, experimental_mode)
    - update(CS, enabled, is_metric, experimental_mode)
  """

  def __init__(self, CP) -> None:
    self.CP = CP
    self.v_cruise_kph: float = V_CRUISE_UNSET
    self.v_cruise_cluster_kph: float = V_CRUISE_UNSET
    self.button_type: Optional[int] = None
    self.button_timer: int = 0
    self.button_counter: int = 0
    self.long_press_counter: int = 0

  # ---------- Public API ----------
  def initialize_v_cruise(self, CS, is_metric: bool, experimental_mode: bool) -> None:
    if self.CP.pcmCruise:
      # Use PCM reported speed when available
      set_speed_kph = CS.cruiseState.speed * CV.MS_TO_KPH
      if set_speed_kph > 0:
        self.v_cruise_kph = set_speed_kph
      else:
        self.v_cruise_kph = V_CRUISE_UNSET
    else:
      self.v_cruise_kph = V_CRUISE_INITIAL_EXPERIMENTAL_MODE if experimental_mode else V_CRUISE_INITIAL
      self.v_cruise_kph = _nearest_kph(self.v_cruise_kph, is_metric)

    self.v_cruise_cluster_kph = _cluster_display_kph(self.v_cruise_kph)

  def update_v_cruise(self, CS, enabled: bool, is_metric: bool, experimental_mode: bool) -> None:
    self.update(CS, enabled, is_metric, experimental_mode)

  def update(self, CS, enabled: bool, is_metric: bool, experimental_mode: bool) -> None:
    """
    Main update entrypoint called by controls.
    """
    # 1) Handle button events first
    self._handle_cruise_buttons(CS, enabled, is_metric, experimental_mode)

    # 2) Update values from PCM or our non-PCM logic
    if self.CP.pcmCruise:
      self._update_from_pcm(CS)
    else:
      self._update_non_pcm(enabled, is_metric)

    # 3) UI/cluster spoof (always applied)
    if self.v_cruise_kph in (V_CRUISE_UNSET, -1):
      self.v_cruise_cluster_kph = self.v_cruise_kph
    else:
      self.v_cruise_cluster_kph = _cluster_display_kph(self.v_cruise_kph)

  # ---------- Internals ----------
  def _update_from_pcm(self, CS) -> None:
    # Actual set speed from PCM (m/s -> kph)
    self.v_cruise_kph = CS.cruiseState.speed * CV.MS_TO_KPH

    # If PCM reports not active, reflect UNSET/-1
    if CS.cruiseState.speed == 0:
      self.v_cruise_kph = V_CRUISE_UNSET
    elif CS.cruiseState.speed == -1:
      self.v_cruise_kph = -1

  def _update_non_pcm(self, enabled: bool, is_metric: bool) -> None:
    if self.v_cruise_kph in (V_CRUISE_UNSET, -1):
      return
    # Enforce min/max based on our single-file override
    self.v_cruise_kph = max(float(V_CRUISE_MIN), min(float(V_CRUISE_MAX), float(self.v_cruise_kph)))
    self.v_cruise_kph = _nearest_kph(self.v_cruise_kph, is_metric)

  def _step_for_button(self, btn: int, is_metric: bool) -> float:
    # 1 kph (metric) or 1 mph (imperial) per tick
    return (1.0 if is_metric else IMPERIAL_INCREMENT) * CRUISE_INTERVAL_SIGN.get(btn, +1)

  def _handle_cruise_buttons(self, CS, enabled: bool, is_metric: bool, experimental_mode: bool) -> None:
    # Read button events
    for be in getattr(CS, "buttonEvents", []):
      if be.pressed and be.type in (ButtonType.accelCruise, ButtonType.decelCruise):
        self.button_type = be.type
        self.button_timer = 1
        self.long_press_counter = 0
        # If unset, initialize immediately on first press
        if self.v_cruise_kph in (V_CRUISE_UNSET, -1):
          self.initialize_v_cruise(CS, is_metric, experimental_mode)
          # Snap then apply first step
          self.v_cruise_kph = _nearest_kph(self.v_cruise_kph, is_metric)
          self.v_cruise_kph += self._step_for_button(self.button_type, is_metric)
        continue

      if not be.pressed and be.type == self.button_type:
        # Button released -> stop repeat
        self.button_type = None
        self.button_timer = 0
        self.long_press_counter = 0

    # Hold logic (autorepeat)
    if self.button_type is not None and self.button_timer > 0:
      self.button_timer += 1

      # After a small delay, start repeating
      if self.button_timer >= 10:
        self.long_press_counter += 1
        step = self._step_for_button(self.button_type, is_metric)

        # Fast repeat after long press
        repeat_every = 2 if self.long_press_counter >= CRUISE_LONG_PRESS else 5
        if self.long_press_counter % repeat_every == 0:
          if self.v_cruise_kph in (V_CRUISE_UNSET, -1):
            self.initialize_v_cruise(CS, is_metric, experimental_mode)
          self.v_cruise_kph += step

      # Clamp to our single-file max/min
      if self.v_cruise_kph not in (V_CRUISE_UNSET, -1):
        self.v_cruise_kph = max(float(V_CRUISE_MIN), min(float(V_CRUISE_MAX), float(self.v_cruise_kph)))
        self.v_cruise_kph = _nearest_kph(self.v_cruise_kph, is_metric)
