import types
from cereal import car, log
from openpilot.common.conversions import Conversions as CV
from openpilot.selfdrive.controls.lib.desire_helper import DesireHelper, LaneChangeState
from openpilot.selfdrive.controls.lib.aalc_constants import AALC_DELAY
from openpilot.common.realtime import DT_MDL


def make_carstate():
    cs = car.CarState.new_message()
    cs.vEgo = 60 * CV.KPH_TO_MS
    cs.steeringPressed = False
    cs.brakePressed = False
    cs.leftBlindspot = False
    cs.rightBlindspot = False
    cs.leftBlinker = False
    cs.rightBlinker = False
    cs.standstill = False
    return cs


def make_plan(active=True):
    plan = log.FrogPilotPlan.new_message()
    plan.aalcActive = active
    plan.leadDistance = 10
    plan.laneWidthLeft = 3.0
    plan.laneWidthRight = 3.0
    return plan


def make_toggles():
    return types.SimpleNamespace(
        lane_detection=False,
        minimum_lane_change_speed=0,
        lane_detection_width=0,
        aalc_enabled=True,
        use_turn_desires=False,
        lane_change_delay=0,
        nudgeless=False,
        one_lane_change=False,
        aalc_stay_left=False,
    )


def test_aalc_delay_countdown():
    dh = DesireHelper()
    cs = make_carstate()
    plan = make_plan()
    toggles = make_toggles()

    dh.update(cs, True, 1.0, plan, toggles)
    assert dh.aalc_active
    assert dh.aalc_delay_timer == AALC_DELAY
    assert dh.lane_change_state == LaneChangeState.off

    for _ in range(int(AALC_DELAY / DT_MDL)):
        dh.update(cs, True, 1.0, plan, toggles)
    assert dh.lane_change_state == LaneChangeState.laneChangeStarting
