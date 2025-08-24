# SPDX-License-Identifier: MIT
# Author: Saeed ALmansoori (SDpilot)

from dataclasses import dataclass
import json
import os
import time
from openpilot.common.params import Params

@dataclass
class AALCConfig:
  enabled: bool = True
  mode_auto: bool = True
  min_gap_m: float = 25.0
  speed_delta_kph: float = 15.0
  max_accel_mps2: float = 1.5
  responsiveness: int = 2   # 0..3 (Low..High)
  learner: bool = True

class AALCState:
  IDLE = "idle"
  ARMING = "arming"
  SIGNAL = "signal"
  LANE_CHANGE = "lane_change"
  COMPLETE = "complete"
  ABORT = "abort"

RESP_PROFILES = {
  0: dict(min_gap_m=35.0, speed_delta_kph=22.0),
  1: dict(min_gap_m=30.0, speed_delta_kph=18.0),
  2: dict(min_gap_m=25.0, speed_delta_kph=15.0),
  3: dict(min_gap_m=20.0, speed_delta_kph=12.0),
}

class AALC:
  def __init__(self, cfg: AALCConfig):
    self.p = Params()
    self.cfg = self._read_cfg_defaults(cfg)
    self.state, self.dir = AALCState.IDLE, None
    self._last_status = {}
    self._learn_buf = {"overtakes":0,"blocks":0}
    os.makedirs("/data/aalc_logs", exist_ok=True)

  # -------- public API --------
  def update(self, ego_kph, lead_kph, gap_m, lane_free_left, lane_free_right):
    self.cfg = self._read_cfg_defaults(self.cfg)  # live-read Params

    if not self.cfg.enabled:
      self._set_status("OFF")
      self._to_idle()
      return

    # effective thresholds by responsiveness
    prof = RESP_PROFILES.get(self.cfg.responsiveness, RESP_PROFILES[2])
    min_gap = float(self._get("AALCMinGapM", prof["min_gap_m"]))
    d_kph   = float(self._get("AALCSpeedDeltaKph", prof["speed_delta_kph"]))

    need_overtake = (lead_kph is not None) and ((lead_kph - ego_kph) >= d_kph)
    safe_gap = (gap_m is None) or (gap_m >= min_gap)

    if self.state == AALCState.IDLE:
      self._set_status("Ready")
      if need_overtake and safe_gap:
        if lane_free_left:
          self._arm("left")
        elif lane_free_right:
          self._arm("right")

    elif self.state == AALCState.ARMING:
      self._send_blinker(self.dir)
      self._goto(AALCState.SIGNAL, "Signaling")

    elif self.state == AALCState.SIGNAL:
      if self._ready_to_change(lane_free_left, lane_free_right):
        self._request_lane_change(self.dir)
        self._goto(AALCState.LANE_CHANGE, "Changing")
      elif not safe_gap:
        self._goto(AALCState.ABORT, "Abort: gap")

    elif self.state == AALCState.LANE_CHANGE:
      if self._lane_change_done():
        self._learn_event("overtakes")
        self._goto(AALCState.COMPLETE, "Done")

    elif self.state in (AALCState.COMPLETE, AALCState.ABORT):
      self._to_idle()

    # persist lightweight HUD status
    self._publish_status(ego_kph, lead_kph, gap_m, min_gap, d_kph)

  def get_status(self):
    return self._last_status

  # -------- internals --------
  def _arm(self, d):
    self.dir = d
    self._goto(AALCState.ARMING, f"Arming {d}")

  def _to_idle(self):
    self._goto(AALCState.IDLE, "Idle")
    self.dir = None

  def _goto(self, st, msg):
    self.state = st
    self._set_status(msg)

  def _set_status(self, msg):
    self._last_status["msg"] = msg
    self._last_status["state"] = self.state
    self._last_status["dir"] = self.dir

  def _publish_status(self, ego_kph, lead_kph, gap_m, min_gap, d_kph):
    st = {
      "state": self.state, "dir": self.dir,
      "text": self._last_status.get("msg",""),
      "ego_kph": round(ego_kph or 0.0,1),
      "lead_kph": round(lead_kph,1) if lead_kph is not None else None,
      "gap_m": round(gap_m,1) if gap_m is not None else None,
      "need_min_gap": min_gap, "need_delta_kph": d_kph,
      "ts": int(time.time())
    }
    self._last_status = st
    self.p.put("AALCStatus", json.dumps(st))

  def _ready_to_change(self, l_ok, r_ok):
    return (self.dir=="left" and l_ok) or (self.dir=="right" and r_ok)

  # === hooks (wire these) ===
  def _send_blinker(self, direction):
    """
    Prefer: call existing turn-signal API if your stack exposes it.
    Else TODO: implement CAN turn signal per-vehicle DBC (bus + signal).
    """
    # no-op here by default
    return None

  def _request_lane_change(self, direction):
    """
    If your stack has a lane-change request helper, call it here.
    Else lane change can be triggered implicitly by sending the blinker + lane_free_* = True.
    """
    try:
      # Example: if self.controls has method start_lane_change(dir)
      ctrl = getattr(self, "_controls_ref", None)
      if ctrl and hasattr(ctrl, "start_lane_change"):
        ctrl.start_lane_change(direction)
    except Exception:
      pass

  def _lane_change_done(self) -> bool:
    """
    Read lane-change completion from planner/state machine if available.
    TODO: wire to actual flag in your tree (e.g., laneChangeState == off).
    """
    return False

  # === Params / Learner ===
  def _get(self, k, default):
    v = self.p.get(k)
    return v.decode() if v else str(default)

  def _read_cfg_defaults(self, cfg):
    en = self.p.get_bool("AALCEnabled")
    if en is not None:
      cfg.enabled = en
    la = self.p.get("AALCLearner")
    cfg.learner = (la.decode() == "1") if la else cfg.learner
    rp = self.p.get("AALCResponsiveness")
    cfg.responsiveness = int(rp.decode()) if rp else cfg.responsiveness
    return cfg

  def _learn_event(self, key):
    if not self.cfg.learner:
      return
    self._learn_buf[key] = self._learn_buf.get(key, 0) + 1
    if sum(self._learn_buf.values()) % 5 == 0:
      path = f"/data/aalc_logs/learn_{int(time.time())}.json"
      with open(path, "w") as f:
        json.dump(self._learn_buf, f)
      if self._learn_buf.get("overtakes", 0) >= 5:
        try:
          cur = float(self._get("AALCMinGapM", 25.0))
          self.p.put("AALCMinGapM", str(max(15.0, cur - 1.0)))
        except Exception:
          pass
