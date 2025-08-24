# SPDX-License-Identifier: MIT
# Author: Saeed ALmansoori (SDpilot)

from dataclasses import dataclass


@dataclass
class AALCConfig:
  enabled: bool = True
  mode_auto: bool = True
  min_gap_m: float = 25.0
  speed_delta_kph: float = 15.0
  max_accel_mps2: float = 1.5


class AALCState:
  IDLE = "idle"
  ARMING = "arming"
  SIGNAL = "signal"
  LANE_CHANGE = "lane_change"
  COMPLETE = "complete"
  ABORT = "abort"


class AALC:
  def __init__(self, cfg: AALCConfig):
    self.cfg = cfg
    self.state = AALCState.IDLE
    self.dir = None  # "left"|"right"|None

  def update(self, ego_kph, lead_kph, gap_m, lane_free_left, lane_free_right):
    if not self.cfg.enabled:
      self.state, self.dir = AALCState.IDLE, None
      return

    need_overtake = (lead_kph is not None) and ((lead_kph - ego_kph) >= self.cfg.speed_delta_kph)
    safe_gap = (gap_m is None) or (gap_m >= self.cfg.min_gap_m)

    if self.state == AALCState.IDLE and need_overtake and safe_gap:
      if lane_free_left:
        self.state, self.dir = AALCState.ARMING, "left"
      elif lane_free_right:
        self.state, self.dir = AALCState.ARMING, "right"

    elif self.state == AALCState.ARMING:
      self._send_blinker_can(self.dir)
      self.state = AALCState.SIGNAL

    elif self.state == AALCState.SIGNAL:
      if self._ready_to_change(lane_free_left, lane_free_right):
        self._request_lane_change(self.dir)
        self.state = AALCState.LANE_CHANGE
      else:
        self.state = AALCState.ABORT

    elif self.state == AALCState.LANE_CHANGE:
      if self._lane_change_done():
        self.state, self.dir = AALCState.COMPLETE, None

    elif self.state in (AALCState.COMPLETE, AALCState.ABORT):
      self.state, self.dir = AALCState.IDLE, None

  # ===== Hooks to wire per-vehicle / stack =====
  def _send_blinker_can(self, direction: str):
    # TODO: Implement per DBC (signal name & bus) e.g. send CAN turn-signal for 'direction'
    pass

  def _ready_to_change(self, lane_free_left: bool, lane_free_right: bool) -> bool:
    return (self.dir == "left" and lane_free_left) or (self.dir == "right" and lane_free_right)

  def _request_lane_change(self, direction: str):
    # TODO: Call existing lane-change request in your stack if available
    pass

  def _lane_change_done(self) -> bool:
    # TODO: Read lane-change completion from planner/model if available
    return False
