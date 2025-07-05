from cereal import log
from openpilot.common.conversions import Conversions as CV
from openpilot.common.realtime import DT_MDL

LaneChangeState = log.LaneChangeState
LaneChangeDirection = log.LaneChangeDirection
TurnDirection = log.Desire

LANE_CHANGE_SPEED_MIN = 20 * CV.MPH_TO_MS
LANE_CHANGE_TIME_MAX = 10.

DESIRES = {
  LaneChangeDirection.none: {
    LaneChangeState.off: log.Desire.none,
    LaneChangeState.preLaneChange: log.Desire.none,
    LaneChangeState.laneChangeStarting: log.Desire.none,
    LaneChangeState.laneChangeFinishing: log.Desire.none,
  },
  LaneChangeDirection.left: {
    LaneChangeState.off: log.Desire.none,
    LaneChangeState.preLaneChange: log.Desire.none,
    LaneChangeState.laneChangeStarting: log.Desire.laneChangeLeft,
    LaneChangeState.laneChangeFinishing: log.Desire.laneChangeLeft,
  },
  LaneChangeDirection.right: {
    LaneChangeState.off: log.Desire.none,
    LaneChangeState.preLaneChange: log.Desire.none,
    LaneChangeState.laneChangeStarting: log.Desire.laneChangeRight,
    LaneChangeState.laneChangeFinishing: log.Desire.laneChangeRight,
  },
}

TURN_DESIRES = {
  TurnDirection.none: log.Desire.none,
  TurnDirection.turnLeft: log.Desire.turnLeft,
  TurnDirection.turnRight: log.Desire.turnRight,
}

class DesireHelper:
  def __init__(self):
    self.lane_change_state = LaneChangeState.off
    self.lane_change_direction = LaneChangeDirection.none
    self.lane_change_timer = 0.0
    self.lane_change_ll_prob = 1.0
    self.keep_pulse_timer = 0.0
    self.prev_one_blinker = False
    self.desire = log.Desire.none

    # FrogPilot variables
    self.lane_change_completed = False

    self.aalc_active = False
    self.aalc_return_direction = LaneChangeDirection.none

    self.lane_change_wait_timer = 0.0

    self.turn_direction = TurnDirection.none

  def update(self, carstate, lateral_active, lane_change_prob, frogpilotPlan, frogpilot_toggles, radarstate=None):
    v_ego = carstate.vEgo
    one_blinker = carstate.leftBlinker != carstate.rightBlinker
    if self.aalc_active:
      one_blinker = True
    below_lane_change_speed = v_ego < frogpilot_toggles.minimum_lane_change_speed

    lane_available_left = frogpilotPlan.laneWidthLeft >= frogpilot_toggles.lane_detection_width
    lane_available_right = frogpilotPlan.laneWidthRight >= frogpilot_toggles.lane_detection_width
    if not (frogpilot_toggles.lane_detection and one_blinker) or below_lane_change_speed:
      lane_available = True
    else:
      desired_lane = frogpilotPlan.laneWidthLeft if carstate.leftBlinker else frogpilotPlan.laneWidthRight
      lane_available = desired_lane >= frogpilot_toggles.lane_detection_width

    if radarstate is not None and frogpilot_toggles.aalc_enabled and not one_blinker and self.lane_change_state == LaneChangeState.off:
      lead = radarstate.leadOne
      if lead.status and v_ego > 80 * CV.KPH_TO_MS and (v_ego - lead.vLead) >= 15 * CV.KPH_TO_MS:
        if lane_available_left and not carstate.leftBlindspot:
          self.lane_change_state = LaneChangeState.preLaneChange
          self.lane_change_direction = LaneChangeDirection.left
          self.aalc_active = True
          self.aalc_return_direction = LaneChangeDirection.right
          self.lane_change_ll_prob = 1.0
          self.lane_change_wait_timer = 0.0
        elif lane_available_right and not carstate.rightBlindspot:
          self.lane_change_state = LaneChangeState.preLaneChange
          self.lane_change_direction = LaneChangeDirection.right
          self.aalc_active = True
          self.aalc_return_direction = LaneChangeDirection.left
          self.lane_change_ll_prob = 1.0
          self.lane_change_wait_timer = 0.0

    if not lateral_active or self.lane_change_timer > LANE_CHANGE_TIME_MAX:
      self.lane_change_state = LaneChangeState.off
      self.lane_change_direction = LaneChangeDirection.none
      self.turn_direction = TurnDirection.none
    elif one_blinker and below_lane_change_speed and not carstate.standstill and frogpilot_toggles.use_turn_desires:
      self.lane_change_state = LaneChangeState.off
      self.lane_change_direction = LaneChangeDirection.none
      self.turn_direction = TurnDirection.turnLeft if carstate.leftBlinker else TurnDirection.turnRight
    else:
      self.turn_direction = TurnDirection.none

      # LaneChangeState.off
      if self.lane_change_state == LaneChangeState.off and one_blinker and not self.prev_one_blinker and not below_lane_change_speed:
        self.lane_change_state = LaneChangeState.preLaneChange
        self.lane_change_ll_prob = 1.0
        self.lane_change_wait_timer = 0.0

      # LaneChangeState.preLaneChange
      elif self.lane_change_state == LaneChangeState.preLaneChange:
        self.lane_change_wait_timer += DT_MDL

        # Set lane change direction
        self.lane_change_direction = LaneChangeDirection.left if \
          carstate.leftBlinker else LaneChangeDirection.right

        torque_applied = carstate.steeringPressed and \
                         ((carstate.steeringTorque > 0 and self.lane_change_direction == LaneChangeDirection.left) or
                          (carstate.steeringTorque < 0 and self.lane_change_direction == LaneChangeDirection.right))

        if torque_applied:
          self.lane_change_wait_timer = frogpilot_toggles.lane_change_delay

        torque_applied |= frogpilot_toggles.nudgeless or self.aalc_active

        blindspot_detected = ((carstate.leftBlindspot and self.lane_change_direction == LaneChangeDirection.left) or
                              (carstate.rightBlindspot and self.lane_change_direction == LaneChangeDirection.right))

        if not one_blinker or below_lane_change_speed or self.lane_change_completed:
          self.lane_change_state = LaneChangeState.off
          self.lane_change_direction = LaneChangeDirection.none
        elif torque_applied and not blindspot_detected and lane_available and self.lane_change_wait_timer >= frogpilot_toggles.lane_change_delay:
          self.lane_change_state = LaneChangeState.laneChangeStarting
          self.lane_change_completed = frogpilot_toggles.one_lane_change
          self.lane_change_wait_timer = 0.0

      # LaneChangeState.laneChangeStarting
      elif self.lane_change_state == LaneChangeState.laneChangeStarting:
        # fade out over .5s
        self.lane_change_ll_prob = max(self.lane_change_ll_prob - 2 * DT_MDL, 0.0)

        # 98% certainty
        if lane_change_prob < 0.02 and self.lane_change_ll_prob < 0.01:
          self.lane_change_state = LaneChangeState.laneChangeFinishing

      # LaneChangeState.laneChangeFinishing
      elif self.lane_change_state == LaneChangeState.laneChangeFinishing:
        # fade in laneline over 1s
        self.lane_change_ll_prob = min(self.lane_change_ll_prob + DT_MDL, 1.0)

        if self.lane_change_ll_prob > 0.99:
          self.lane_change_direction = LaneChangeDirection.none
          if one_blinker:
            self.lane_change_state = LaneChangeState.preLaneChange
          else:
            self.lane_change_state = LaneChangeState.off
            if self.aalc_active and self.aalc_return_direction != LaneChangeDirection.none:
              if (self.aalc_return_direction == LaneChangeDirection.left and lane_available_left and not carstate.leftBlindspot) or \
                 (self.aalc_return_direction == LaneChangeDirection.right and lane_available_right and not carstate.rightBlindspot):
                self.lane_change_state = LaneChangeState.preLaneChange
                self.lane_change_direction = self.aalc_return_direction
                self.lane_change_ll_prob = 1.0
                self.lane_change_wait_timer = 0.0
                self.aalc_active = False
                self.aalc_return_direction = LaneChangeDirection.none

    if self.lane_change_state in (LaneChangeState.off, LaneChangeState.preLaneChange):
      self.lane_change_timer = 0.0
    else:
      self.lane_change_timer += DT_MDL

    if self.lane_change_state == LaneChangeState.off and self.aalc_return_direction == LaneChangeDirection.none:
      self.aalc_active = False

    self.lane_change_completed &= one_blinker
    self.prev_one_blinker = one_blinker

    if self.turn_direction != TurnDirection.none:
      self.desire = TURN_DESIRES[self.turn_direction]
    else:
      self.desire = DESIRES[self.lane_change_direction][self.lane_change_state]

    # Send keep pulse once per second during LaneChangeStart.preLaneChange
    if self.lane_change_state in (LaneChangeState.off, LaneChangeState.laneChangeStarting):
      self.keep_pulse_timer = 0.0
    elif self.lane_change_state == LaneChangeState.preLaneChange:
      self.keep_pulse_timer += DT_MDL
      if self.keep_pulse_timer > 1.0:
        self.keep_pulse_timer = 0.0
      elif self.desire in (log.Desire.keepLeft, log.Desire.keepRight):
        self.desire = log.Desire.none
