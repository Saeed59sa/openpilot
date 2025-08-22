#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
#
# selfdrive/controls/lib/aalc.py
# ------------------------------------------------------------
# AALC: Automatic Adaptive Lane Change policy for SDpilot/openpilot
# - Requires user consent param (AALCAgreed=1) to activate
# ------------------------------------------------------------

from __future__ import annotations
import time
from typing import Optional, Tuple

try:
  from cereal import log
except Exception:
  log = None

try:
  from common.params import Params
except Exception:
  class Params:
    def get(self, *_args, **_kwargs): return None
    def put(self, *_args, **_kwargs): return None

_P = Params()

# ----------------------------
# Param helpers
# ----------------------------
def _get_bool(key: str, default: bool) -> bool:
  v = _P.get(key)
  if v is None: return default
  try: return v.decode() in ("1", "true", "True", "TRUE")
  except Exception: return default

def _get_int(key: str, default: int) -> int:
  v = _P.get(key)
  if v is None: return default
  try: return int(v.decode())
  except Exception: return default

def _get_str(key: str, default: str) -> str:
  v = _P.get(key)
  if v is None: return default
  try:
    s = v.decode().strip()
    return s or default
  except Exception:
    return default

def ensure_default_params():
  defaults = {
    "AALCEnabled": "0",
    "AALCAgreed": "0",            # must be "1" after user accepts agreement
    "AALCAutoBlinker": "1",
    "AALCSpeedDeltaKph": "20",
    "AALCMinSpeedKph": "60",
    "AALCPreferredSide": "left",  # "left"|"right"|"auto"
    "AALCSafetyGapSec": "1.5",
  }
  for k, v in defaults.items():
    if _P.get(k) is None:
      _P.put(k, v)

ensure_default_params()

# ----------------------------
# CAN blinker stub (fill per-vehicle DBC)
# ----------------------------
def _aalc_send_blinker_can(direction: str):
  """
  Implement per your car’s DBC:
    - direction: "left" or "right"
    - Send body-control msg to toggle blinker before requesting lane change
  This is intentionally empty to avoid unintended signaling.
  """
  return

# ----------------------------
# Core AALC policy
# ----------------------------
class AALC:
  def __init__(self):
    self.last_request_ts = 0.0
    self.cooldown_s = 6.0   # prevent rapid re-triggers

  def enabled(self) -> bool:
    # Feature requires BOTH: user enabled + user agreed
    return _get_bool("AALCEnabled", False) and _get_bool("AALCAgreed", False)

  def auto_blinker(self) -> bool:
    return _get_bool("AALCAutoBlinker", True)

  def speed_delta_kph(self) -> int:
    return max(5, _get_int("AALCSpeedDeltaKph", 20))

  def min_speed_kph(self) -> int:
    return max(0, _get_int("AALCMinSpeedKph", 60))

  def preferred_side(self) -> str:
    side = _get_str("AALCPreferredSide", "left").lower()
    return side if side in ("left", "right", "auto") else "left"

  def safety_gap_s(self) -> float:
    try: return max(0.5, float(_get_str("AALCSafetyGapSec", "1.5")))
    except Exception: return 1.5

  # ---- Lead extraction (modelV2.leadsV3) ----
  def _best_lead(self, sm) -> Tuple[Optional[object], float, float]:
    if not sm.updated("modelV2"):
      return None, 0.0, 1e9
    m = sm["modelV2"]
    leads = getattr(m, "leadsV3", []) or []
    best = None; best_p = 0.0
    for L in leads:
      p = getattr(L, "prob", 0.0)
      if p > best_p:
        best, best_p = L, p
    if best is None:
      return None, 0.0, 1e9
    return best, getattr(best, "vRel", 0.0), getattr(best, "x", 1e9)

  def _lane_availability(self, sm) -> Tuple[bool, bool]:
    if not sm.updated("modelV2"):
      return False, False
    probs = list(getattr(sm["modelV2"], "laneLineProbs", []))
    left_ok = right_ok = False
    if len(probs) >= 4:
      left_ok  = probs[0] > 0.3 or probs[1] > 0.35
      right_ok = probs[3] > 0.3 or probs[2] > 0.35
    return left_ok, right_ok

  def choose_direction(self, sm, v_ego_mps: float) -> Optional[str]:
    if not self.enabled(): return None

    v_kph = v_ego_mps * 3.6
    if v_kph < self.min_speed_kph(): return None

    now = time.monotonic()
    if now - self.last_request_ts < self.cooldown_s: return None

    lead, v_rel, x = self._best_lead(sm)
    if lead is None or getattr(lead, "prob", 0.0) < 0.5: return None

    # slower lead? (negative v_rel)
    delta_needed = self.speed_delta_kph() / 3.6
    if v_rel > -delta_needed: return None
    if x > 80.0: return None

    left_ok, right_ok = self._lane_availability(sm)
    pref = self.preferred_side()
    if   pref == "left":  side = "left"  if left_ok  else ("right" if right_ok else None)
    elif pref == "right": side = "right" if right_ok else ("left"  if left_ok  else None)
    else:
      if left_ok and not right_ok:   side = "left"
      elif right_ok and not left_ok: side = "right"
      elif left_ok and right_ok:     side = "left"
      else:                          side = None

    if side is None: return None
    self.last_request_ts = now
    return side

  def prime_blinker(self, side: str):
    if self.auto_blinker() and side in ("left","right"):
      _aalc_send_blinker_can(side)

_AALC_INSTANCE: Optional[AALC] = None
def get_aalc() -> AALC:
  global _AALC_INSTANCE
  if _AALC_INSTANCE is None:
    _AALC_INSTANCE = AALC()
  return _AALC_INSTANCE
