import numpy as np

from cereal import log
from common.filter_simple import FirstOrderFilter
from common.realtime import DT_MDL
from openpilot.selfdrive.controls.lib.drive_helpers import CONTROL_N, MAX_LATERAL_JERK
from openpilot.selfdrive.modeld.constants import ModelConstants

class LanePlanner:
  def __init__(self,):
    self.ll_t = np.zeros((ModelConstants.IDX_N,))
    self.ll_x = np.zeros((ModelConstants.IDX_N,))
    self.lll_y = np.zeros((ModelConstants.IDX_N,))
    self.rll_y = np.zeros((ModelConstants.IDX_N,))
    self.lane_width_estimate = FirstOrderFilter(3.7, 9.95, DT_MDL)
    self.lane_width_certainty = FirstOrderFilter(1.0, 0.95, DT_MDL)
    self.lane_width = 3.7

    self.lll_prob = 0.
    self.rll_prob = 0.
    self.d_prob = 0.

    self.lll_std = 0.
    self.rll_std = 0.

    self.l_lane_change_prob = 0.
    self.r_lane_change_prob = 0.

    self.camera_offset = 0.0

    self.readings = []
    self.frame = 0

  def parse_model(self, model_msg):
    lane_lines = model_msg.laneLines
    if len(lane_lines) == 4 and len(lane_lines[0].t) == ModelConstants.IDX_N:
      self.ll_t = (np.array(lane_lines[1].t) + np.array(lane_lines[2].t))/2
      # left and right ll x is the same
      self.ll_x = lane_lines[1].x
      self.lll_y = np.array(lane_lines[1].y) + self.camera_offset
      self.rll_y = np.array(lane_lines[2].y) + self.camera_offset
      self.lll_prob = model_msg.laneLineProbs[1]
      self.rll_prob = model_msg.laneLineProbs[2]
      self.lll_std = model_msg.laneLineStds[1]
      self.rll_std = model_msg.laneLineStds[2]

    desire_state = model_msg.meta.desireState
    if len(desire_state):
      self.l_lane_change_prob = desire_state[log.Desire.laneChangeLeft]
      self.r_lane_change_prob = desire_state[log.Desire.laneChangeRight]

  def get_d_path(self, v_ego, path_t, path_xyz):
    l_prob, r_prob = self.lll_prob, self.rll_prob
    width_pts = self.rll_y - self.lll_y
    prob_mods = []
    for t_check in (0.0, 1.5, 3.0):
      width_at_t = np.interp(t_check * (v_ego + 7), self.ll_x, width_pts)
      prob_mods.append(np.interp(width_at_t, [4.0, 5.0], [1.0, 0.0]))
    mod = min(prob_mods)
    l_prob *= mod
    r_prob *= mod

    l_std_mod = np.interp(self.lll_std, [.15, .3], [1.0, 0.0])
    r_std_mod = np.interp(self.rll_std, [.15, .3], [1.0, 0.0])
    l_prob *= l_std_mod
    r_prob *= r_std_mod

    if l_prob > 0.5 and r_prob > 0.5:
      self.frame += 1
      if self.frame > 20:
        self.frame = 0
        current_lane_width = np.clip(abs(self.rll_y[0] - self.lll_y[0]), 2.5, 3.5)
        self.readings.append(current_lane_width)
        self.lane_width = float(np.mean(self.readings))
        if len(self.readings) >= 30:
          self.readings.pop(0)

    if abs(self.rll_y[0] - self.lll_y[0]) > self.lane_width:
      r_prob = r_prob / np.interp(l_prob, [0, 1], [1, 3])

    clipped_lane_width = min(4.0, self.lane_width)
    path_from_left_lane = self.lll_y + clipped_lane_width / 2.0
    path_from_right_lane = self.rll_y - clipped_lane_width / 2.0

    self.d_prob = l_prob + r_prob - l_prob * r_prob

    if self.d_prob > 0.65:
      self.d_prob = min(self.d_prob * 1.3, 1.0)

    lane_path_y = (l_prob * path_from_left_lane + r_prob * path_from_right_lane) / (l_prob + r_prob + 0.0001)
    safe_idxs = np.isfinite(self.ll_t)
    if safe_idxs[0]:
      lane_path_y_interp = np.interp(path_t, self.ll_t[safe_idxs], lane_path_y[safe_idxs])
      path_xyz[:,1] = self.d_prob * lane_path_y_interp + (1.0 - self.d_prob) * path_xyz[:,1]
    return path_xyz


  @staticmethod
  def get_lag_adjusted_curvature(v_ego, psis, curvatures, distances):
    if len(psis) != CONTROL_N:
      psis = [0.0] * CONTROL_N
      curvatures = [0.0] * CONTROL_N
      distances = [0.0] * CONTROL_N
    v_ego = max(1.0, v_ego)

    # TODO this needs more thought, use .2s extra for now to estimate other delays
    delay = 0.4
    path_factor = 0.95
    # MPC can plan to turn the wheel and turn back before t_delay. This means
    # in high delay cases some corrections never even get commanded. So just use
    # psi to calculate a simple linearization of desired curvature
    current_curvature_desired = curvatures[0]
    psi = np.interp(delay, ModelConstants.T_IDXS[:CONTROL_N], psis)
    distance = max(np.interp(delay, ModelConstants.T_IDXS[:CONTROL_N], distances), 0.001)
    average_curvature_desired = psi / distance
    desired_curvature = 2 * average_curvature_desired - current_curvature_desired

    # This is the "desired rate of the setpoint" not an actual desired rate
    max_curvature_rate = MAX_LATERAL_JERK / (v_ego ** 2)  # inexact calculation, check https://github.com/commaai/openpilot/pull/24755
    safe_desired_curvature = np.clip(desired_curvature,
                                     current_curvature_desired - max_curvature_rate * DT_MDL,
                                     current_curvature_desired + max_curvature_rate * DT_MDL)

    return safe_desired_curvature * path_factor
