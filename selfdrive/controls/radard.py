#!/usr/bin/env python3
import math
import numpy as np
from collections import deque
from typing import Any, Dict, Tuple

import capnp
from cereal import messaging, log, car
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.params import Params
from openpilot.common.realtime import DT_MDL, Priority, config_realtime_process
from openpilot.common.swaglog import cloudlog
from openpilot.common.simple_kalman import KF1D


# Default lead acceleration decay set to 50% at 1s
_LEAD_ACCEL_TAU = 1.5

# radar tracks
SPEED, ACCEL = 0, 1     # Kalman filter states enum

# stationary qualification parameters
V_EGO_STATIONARY = 4.   # no stationary object flag below this speed

RADAR_TO_CENTER = 2.7   # (deprecated) RADAR is ~ 2.7m ahead from center of car
RADAR_TO_CAMERA = 1.52  # RADAR is ~ 1.5m ahead from center of mesh frame

# Constants for lead tracking
LEAD_PROB_THRESHOLD = 0.5
VISION_PROB_HIGH_THRESHOLD = 0.8
VISION_PROB_VERY_HIGH_THRESHOLD = 0.97
VISION_PROB_INTERP_LOW = 0.4
VISION_PROB_INTERP_HIGH = 0.0
VISION_DREL_RESET_THRESHOLD = 5.0
VISION_CNT_THRESHOLD = 20
VISION_ACCEL_THRESHOLD = 0.3
VISION_ACCEL_TAU_LOW = 0.2
VISION_ACCEL_TAU_DECAY = 0.9
VISION_VLAT_ALPHA = 0.002
MIN_LEAD_SIDE_DRel = 5.0


class KalmanParams:
  def __init__(self, dt: float):
    # Lead Kalman Filter params, calculating K from A, C, Q, R requires the control library.
    # hardcoding a lookup table to compute K for values of radar_ts between 0.01s and 0.2s
    assert dt > .01 and dt < .2, "Radar time step must be between .01s and 0.2s"
    self.A = [[1.0, dt], [0.0, 1.0]]
    self.C = [1.0, 0.0]
    #Q = np.matrix([[10., 0.0], [0.0, 100.]])
    #R = 1e3
    #K = np.matrix([[ 0.05705578], [ 0.03073241]])
    dts = [i * 0.01 for i in range(1, 21)]
    K0 = [0.12287673, 0.14556536, 0.16522756, 0.18281627, 0.1988689,  0.21372394,
          0.22761098, 0.24069424, 0.253096,   0.26491023, 0.27621103, 0.28705801,
          0.29750003, 0.30757767, 0.31732515, 0.32677158, 0.33594201, 0.34485814,
          0.35353899, 0.36200124]
    K1 = [0.29666309, 0.29330885, 0.29042818, 0.28787125, 0.28555364, 0.28342219,
          0.28144091, 0.27958406, 0.27783249, 0.27617149, 0.27458948, 0.27307714,
          0.27162685, 0.27023228, 0.26888809, 0.26758976, 0.26633338, 0.26511557,
          0.26393339, 0.26278425]
    self.K = [[np.interp(dt, dts, K0)], [np.interp(dt, dts, K1)]]


class Track:
  def __init__(self, identifier: int):
    self.identifier = identifier
    self.cnt = 0
    self.aLeadTau = FirstOrderFilter(_LEAD_ACCEL_TAU, 0.45, DT_MDL)

  def update(self, d_rel: float, y_rel: float, v_rel: float, v_lead: float, a_lead: float, j_lead: float, measured: float, yv_lead: float):
    self.dRel = d_rel   # LONG_DIST
    self.yRel = y_rel   # -LAT_DIST
    self.vRel = v_rel   # REL_SPEED

    self.vLead = self.vLeadK = v_lead
    self.aLead = self.aLeadK = a_lead
    self.jLead = j_lead
    self.yvLead = yv_lead

    self.measured = measured   # measured or estimate

    if abs(a_lead) < 0.5 and abs(j_lead) < 0.5:
      self.aLeadTau.x = _LEAD_ACCEL_TAU
    else:
      self.aLeadTau.update(0.0)
    self.cnt += 1

  def get_RadarState(self, model_msg, model_prob=0.0, vision_y_rel=None):
    y_rel = float(vision_y_rel) if vision_y_rel is not None else float(self.yRel)
    d_path = y_rel + np.interp(self.dRel, model_msg.position.x, model_msg.position.y)
    return {
      "dRel": float(self.dRel),
      "yRel": y_rel,
      "dPath": float(d_path),
      "vRel": float(self.vRel),
      "vLead": float(self.vLead),
      "vLeadK": float(self.vLeadK),
      "aLead": float(self.aLead),
      "aLeadK": float(self.aLeadK),
      "aLeadTau": float(self.aLeadTau.x),
      "jLead": float(self.jLead),
      "vLat": float(self.yvLead),
      "status": True,
      "fcw": self.is_potential_fcw(model_prob),
      "modelProb": model_prob,
      "radar": True,
      "radarTrackId": self.identifier,
    }

  def potential_low_speed_lead(self, v_ego: float):
    # stop for stuff in front of you and low speed, even without model confirmation
    # Radar points closer than 0.75, are almost always glitches on toyota radars
    return abs(self.yRel) < 1.0 and (v_ego < V_EGO_STATIONARY) and (0.75 < self.dRel < 25)

  def is_potential_fcw(self, model_prob: float):
    return model_prob > .9

  def __str__(self):
    ret = f"x: {self.dRel:4.1f}  y: {self.yRel:4.1f}  v: {self.vRel:4.1f}  a: {self.aLeadK:4.1f}"
    return ret


def laplacian_pdf(x: float, mu: float, b: float):
  b = max(b, 1e-4)
  return math.exp(-abs(x-mu)/b)


class VisionTrack:
  def __init__(self, radar_ts):
    self.radar_ts = radar_ts
    self.dRel = 0.0
    self.vRel = 0.0
    self.yRel = 0.0
    self.vLead = 0.0
    self.aLead = 0.0
    self.vLeadK = 0.0
    self.aLeadK = 0.0
    self.aLeadTau = _LEAD_ACCEL_TAU
    self.prob = 0.0
    self.status = False
    self.dRel_last = 0.0
    self.vLead_last = 0.0
    self.alpha = 0.02
    self.alpha_a = 0.02
    self.vLat = 0.0
    self.v_ego = 0.0
    self.cnt = 0
    self.dPath = 0.0

  def reset(self):
    self.status = False
    self.aLeadTau = _LEAD_ACCEL_TAU

    self.vRel = 0.0
    self.vLead = self.vLeadK = self.v_ego
    self.aLead = self.aLeadK = 0.0
    self.vLat = 0.0
    self.cnt = 0
    self.dPath = 0.0

  def get_lead(self, model_msg):
    return {
      "dRel": self.dRel,
      "vRel": self.vRel,
      "yRel": self.yRel,
      "dPath": self.dPath,
      "vLead": self.vLead,
      "vLeadK": self.vLeadK,
      "aLead": self.aLead,
      "aLeadK": self.aLeadK,
      "aLeadTau": self.aLeadTau,
      "jLead": 0.0,
      "vLat": 0.0,
      "fcw": False,
      "modelProb": self.prob,
      "status": self.status,
      "radar": False,
      "radarTrackId": -1,
      #"aLead": self.aLead,
      #"vLat": self.vLat,
    }

  def _update_velocity_and_acceleration(self, lead_v_rel_pred: float, v_ego: float, a_lead_vision: float):
    """Updates relative and lead velocities, and lead acceleration based on vision."""
    if self.cnt < VISION_CNT_THRESHOLD or self.prob < VISION_PROB_VERY_HIGH_THRESHOLD:
      self.vRel = lead_v_rel_pred
      self.vLead = float(v_ego + lead_v_rel_pred)
      self.aLead = a_lead_vision
    else:
      # Calculate v_rel from dRel derivative
      v_rel_derived = (self.dRel - self.dRel_last) / self.radar_ts
      v_rel_filtered = self.vRel * (1. - self.alpha) + v_rel_derived * self.alpha

      # Blend with model velocity prediction
      model_weight = np.interp(self.prob, [VISION_PROB_VERY_HIGH_THRESHOLD, 1.0], [VISION_PROB_INTERP_LOW, VISION_PROB_INTERP_HIGH])
      self.vRel = float(lead_v_rel_pred * model_weight + v_rel_filtered * (1. - model_weight))
      self.vLead = float(v_ego + self.vRel)

      # Calculate a_lead from vLead derivative
      a_lead_derived = (self.vLead - self.vLead_last) / self.radar_ts * 0.2 # 0.5 -> 0.2 vel 미분적용을 줄임.
      self.aLead = self.aLead * (1. - self.alpha_a) + a_lead_derived * self.alpha_a
      if abs(a_lead_vision) > abs(self.aLead):
        self.aLead = a_lead_vision

  def _update_lateral_velocity(self, model_msg):
    dPath_current = self.yRel + np.interp(self.dRel, model_msg.position.x, model_msg.position.y)
    self.vLat = self.vLat * (1. - VISION_VLAT_ALPHA) + (dPath_current - self.dPath) / self.radar_ts * VISION_VLAT_ALPHA
    self.dPath = float(dPath_current)

  def update(self, lead_msg: capnp._DynamicStructReader, model_v_ego: float, v_ego: float, model_msg: capnp._DynamicStructReader):
    lead_v_rel_pred = lead_msg.v[0] - model_v_ego
    self.prob = lead_msg.prob
    self.v_ego = v_ego

    if self.prob > LEAD_PROB_THRESHOLD:
      dRel_current = float(lead_msg.x[0]) - RADAR_TO_CAMERA
      if abs(self.dRel - dRel_current) > VISION_DREL_RESET_THRESHOLD:
        self.cnt = 0 # Reset counter if dRel jumps significantly
      self.dRel = dRel_current

      self.yRel = float(-lead_msg.y[0])
      a_lead_vision = lead_msg.a[0]

      self._update_velocity_and_acceleration(lead_v_rel_pred, v_ego, a_lead_vision)
      self._update_lateral_velocity(model_msg)

      self.vLeadK = self.vLead
      self.aLeadK = self.aLead

      self.status = True
      self.cnt += 1
    else:
      self.reset()
      self.cnt = 0
      self.dPath = float(self.yRel + np.interp(v_ego ** 2 / (2 * 2.5), model_msg.position.x, model_msg.position.y))

    self.dRel_last = self.dRel
    self.vLead_last = self.vLead

    # Learn if constant acceleration
    if abs(self.aLead) < VISION_ACCEL_THRESHOLD:
      self.aLeadTau = VISION_ACCEL_TAU_LOW
    else:
      self.aLeadTau *= VISION_ACCEL_TAU_DECAY


class RadarD:
  def __init__(self, delay: float = 0.0):
    self.current_time = 0.0

    self.tracks: dict[int, Track] = {}

    self.v_ego = 0.0
    self.v_ego_hist = deque([0.0], maxlen=int(round(delay / DT_MDL))+1)
    self.last_v_ego_frame = -1

    self.radar_state: capnp._DynamicStructBuilder | None = None
    self.radar_state_valid = False

    self.ready = False

    self.vision_tracks = [VisionTrack(DT_MDL), VisionTrack(DT_MDL)]

    self.radar_detected = False


  def _match_vision_to_track(self, v_ego: float, lead: capnp._DynamicStructReader, tracks: dict[int, Track]):
    offset_vision_dist = lead.x[0] - RADAR_TO_CAMERA
    vel_tolerance = 25.0 if lead.prob > 0.99 else 10.0
    max_offset_vision_dist = max(offset_vision_dist * 0.35, 5.0)

    def prob(c):
      if abs(c.dRel - offset_vision_dist) > max_offset_vision_dist:
        return -1e6
      if not ((abs(c.vLead - lead.v[0]) < vel_tolerance) or (c.vLead > 3)):
        return -1e6
      prob_d = laplacian_pdf(c.dRel, offset_vision_dist, lead.xStd[0])
      prob_y = laplacian_pdf(c.yRel, -lead.y[0], lead.yStd[0])
      prob_v = laplacian_pdf(c.vLead, lead.v[0], lead.vStd[0])

      weight_v = np.interp(c.vLead, [0, 10], [0.3, 1])

      return prob_d * prob_y * prob_v * weight_v

    track = max(tracks.values(), key=prob, default=None)
    return track if track and prob(track) > -1e6 else None

  def _get_lead_data(self, model_msg, tracks: dict[int, Track], index: int, lead_msg: capnp._DynamicStructReader,
                    low_speed_override: bool = True) \
    -> tuple[dict[str, Any], bool]:
    v_ego = self.v_ego
    ready = self.ready

    track = None
    if len(tracks) > 0 and ready and lead_msg.prob > LEAD_PROB_THRESHOLD:
      track = self._match_vision_to_track(v_ego, lead_msg, tracks)

    lead_dict: Dict[str, Any] = {'status': False}
    radar_detected = False

    if track is not None:
      # If a radar track matches vision, prioritize radar data
      lead_dict = track.get_RadarState(model_msg, lead_msg.prob, self.vision_tracks[index].yRel)
      radar_detected = True
    elif ready and (lead_msg.prob > VISION_PROB_HIGH_THRESHOLD):
      # If no radar track matches but vision probability is high, use vision only
      lead_dict = self.vision_tracks[index].get_lead(model_msg)

    if low_speed_override:
      low_speed_tracks = [c for c in tracks.values() if c.potential_low_speed_lead(v_ego)]
      if len(low_speed_tracks) > 0:
        closest_track = min(low_speed_tracks, key=lambda c: c.dRel)

        # Only choose new track if it is actually closer than the previous one,
        # or if no lead was previously found.
        if (not lead_dict['status']) or (closest_track.dRel < lead_dict['dRel']):
          lead_dict = closest_track.get_RadarState(model_msg, lead_msg.prob, self.vision_tracks[index].yRel)
          radar_detected = True # If low speed track is chosen, it's from radar

    return lead_dict, radar_detected

  def _get_lead_side_data(self, v_ego: float, tracks: dict[int, Track], model_msg: capnp._DynamicStructReader, lane_width: float, model_v_ego: float):
    """
    Identifies leads in center, left, and right lanes based on radar tracks and vision.
    Refactored from the standalone `get_lead_side` function.
    """
    lead_msg = model_msg.leadsV3[0]

    # Initialize default empty dictionaries for leads
    leadCenter = {'status': False}
    leadLeft = {'status': False}
    leadRight = {'status': False}

    if not (model_msg is not None and len(model_msg.position.x) == 33): #ModelConstants.IDX_N:
        return [[],[],[],leadCenter,leadLeft,leadRight]

    y_path = model_msg.position.y
    x_path = model_msg.position.x

    leads_center = {}
    leads_left = {}
    leads_right = {}
    next_lane_y = lane_width / 2 + lane_width * 0.8 # Define as constant if used elsewhere

    # Categorize radar tracks by lane
    for c in tracks.values():
      d_y = c.yRel + np.interp(c.dRel, x_path, y_path)
      if abs(d_y) < lane_width/2:
        ld = c.get_RadarState(model_msg, lead_msg.prob, float(-lead_msg.y[0]))
        leads_center[c.dRel] = ld
      elif -next_lane_y < d_y < 0: # Right lane
        ld = c.get_RadarState(model_msg, 0.0, 0.0) # Model prob 0, y_rel 0 for side leads not directly matched to vision lead
        leads_right[c.dRel] = ld
      elif 0 < d_y < next_lane_y: # Left lane
        ld = c.get_RadarState(model_msg, 0.0, 0.0) # Model prob 0, y_rel 0 for side leads not directly matched to vision lead
        leads_left[c.dRel] = ld

    # Add primary vision lead to center if probability is high
    if lead_msg.prob > LEAD_PROB_THRESHOLD:
      # Re-use vision track's get_lead to get the structured dictionary
      ld = self.vision_tracks[0].get_lead(model_msg) # Assuming lead_msg[0] corresponds to vision_tracks[0]
      leads_center[ld['dRel']] = ld

    ll = list(leads_left.values())
    lr = list(leads_right.values())

    # Find the closest lead in each category
    if leads_center:
      dRel_min = min(leads_center.keys())
      lc = [leads_center[dRel_min]] # Only return the closest center lead
    else:
      lc = []

    # Filter for leads beyond a minimum distance
    leadLeft = min((lead for dRel, lead in leads_left.items() if lead['dRel'] > MIN_LEAD_SIDE_DRel), key=lambda x: x['dRel'], default=leadLeft)
    leadRight = min((lead for dRel, lead in leads_right.items() if lead['dRel'] > MIN_LEAD_SIDE_DRel), key=lambda x: x['dRel'], default=leadRight)

    # Lead center special condition (radar detected and certain speed)
    leadCenter = min((lead for dRel, lead in leads_center.items() if lead['vLead'] > 10 / 3.6 and lead['radar']), key=lambda x: x['dRel'], default=leadCenter)

    return [ll, lc, lr, leadCenter, leadLeft, leadRight]


  def update(self, sm: messaging.SubMaster, rr: car.RadarData):
    self.ready = sm.seen['modelV2']
    self.current_time = 1e-9 * max(sm.logMonoTime.values())

    leads_v3 = sm['modelV2'].leadsV3
    if sm.recv_frame['carState'] != self.last_v_ego_frame:
      self.v_ego = sm['carState'].vEgo
      self.v_ego_hist.append(self.v_ego)
      self.last_v_ego_frame = sm.recv_frame['carState']

    ar_pts = {}
    for pt in rr.points:
      pt_yRel = -leads_v3[0].y[0] if pt.trackId in [0, 1] and pt.yRel == 0 and self.ready and leads_v3[0].prob > LEAD_PROB_THRESHOLD else pt.yRel
      ar_pts[pt.trackId] = [pt.dRel, pt_yRel, pt.vRel, pt.measured, pt.vLead, pt.aLead, pt.jLead, pt.yvRel]

    # *** remove missing points from meta data ***
    for ids in list(self.tracks.keys()):
      if ids not in ar_pts:
        self.tracks.pop(ids, None)

    # *** compute the tracks ***
    for ids in ar_pts:
      rpt = ar_pts[ids]

      # align v_ego by a fixed time to align it with the radar measurement
      #v_lead = rpt[2] + self.v_ego_hist[0]
      v_lead = rpt[4] # carrot
      a_lead = rpt[5]
      j_lead = rpt[6]
      yv_lead = rpt[7]

      # create the track if it doesn't exist or it's a new track
      if ids not in self.tracks:
        self.tracks[ids] = Track(ids)
      self.tracks[ids].update(rpt[0], rpt[1], rpt[2], v_lead, a_lead, j_lead, rpt[3], yv_lead)

    # *** publish radarState ***
    self.radar_state_valid = sm.all_checks()
    self.radar_state = log.RadarState.new_message()

    model_updated = False if self.radar_state.mdMonoTime == sm.logMonoTime['modelV2'] else True

    self.radar_state.mdMonoTime = sm.logMonoTime['modelV2']
    self.radar_state.radarErrors = rr.errors
    self.radar_state.carStateMonoTime = sm.logMonoTime['carState']

    if len(sm['modelV2'].velocity.x):
      model_v_ego = sm['modelV2'].velocity.x[0]
    else:
      model_v_ego = self.v_ego

    if len(leads_v3) > 1:
      if model_updated:
        if self.radar_detected:
          self.vision_tracks[0].reset()
          self.vision_tracks[1].reset()
        self.vision_tracks[0].update(leads_v3[0], model_v_ego, self.v_ego, sm['modelV2'])
        self.vision_tracks[1].update(leads_v3[1], model_v_ego, self.v_ego, sm['modelV2'])

      self.radar_state.leadOne, self.radar_detected = self._get_lead_data(sm['modelV2'], self.tracks, 0, leads_v3[0], low_speed_override=False)
      self.radar_state.leadTwo, _ = self._get_lead_data(sm['modelV2'], self.tracks, 1, leads_v3[1], low_speed_override=False)

      ll, lc, lr, leadCenter, self.radar_state.leadLeft, self.radar_state.leadRight = self._get_lead_side_data(self.v_ego, self.tracks, sm['modelV2'], 3.2, model_v_ego)
      self.radar_state.leadsLeft = list(ll)
      self.radar_state.leadsCenter = list(lc)
      self.radar_state.leadsRight = list(lr)

  def publish(self, pm: messaging.PubMaster):
    assert self.radar_state is not None

    radar_msg = messaging.new_message("radarState")
    radar_msg.valid = self.radar_state_valid
    radar_msg.radarState = self.radar_state
    pm.send("radarState", radar_msg)


# fuses camera and radar data for best lead detection
def main() -> None:
  config_realtime_process(5, Priority.CTRL_LOW)

  # wait for stats about the car to come in from controls
  cloudlog.info("radard is waiting for CarParams")
  CP = messaging.log_from_bytes(Params().get("CarParams", block=True), car.CarParams)
  cloudlog.info("radard got CarParams")

  # *** setup messaging
  sm = messaging.SubMaster(['modelV2', 'carState', 'liveTracks'], poll='modelV2')
  pm = messaging.PubMaster(['radarState'])

  RD = RadarD(CP.radarDelay)

  while True:
    sm.update()

    RD.update(sm, sm['liveTracks'])
    RD.publish(pm)


if __name__ == "__main__":
  main()
