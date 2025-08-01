#!/usr/bin/env python3
"""Interface for reading openpilot HUDControl data."""

import time
from dataclasses import dataclass
from typing import Dict

from cereal import messaging, car

@dataclass
class HUDData:
    set_speed: float
    left_lane_visible: bool
    right_lane_visible: bool
    lead_distance_bars: int
    visual_alert: int
    audible_alert: int

class OpenpilotInterface:
    """Subscribe to openpilot messaging and expose HUDControl state."""

    def __init__(self) -> None:
        # Subscribe to carControl and controlsState like openpilot's UI
        self.sm = messaging.SubMaster(['carControl', 'controlsState'])

    def update(self) -> HUDData:
        """Poll for new messages and return HUD data."""
        self.sm.update(0)
        cc = self.sm['carControl']
        hud = cc.hudControl
        return HUDData(
            set_speed=float(hud.setSpeed),
            left_lane_visible=bool(hud.leftLaneVisible),
            right_lane_visible=bool(hud.rightLaneVisible),
            lead_distance_bars=int(hud.leadDistanceBars),
            visual_alert=int(hud.visualAlert),
            audible_alert=int(hud.audibleAlert),
        )
