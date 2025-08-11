import pyray as rl
from dataclasses import dataclass
from cereal.messaging import SubMaster
from openpilot.selfdrive.ui.ui_state import ui_state, UIStatus
from openpilot.system.ui.lib.application import gui_app, FontWeight
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.common.conversions import Conversions as CV

# Constants
SET_SPEED_NA = 255
KM_TO_MILE = 0.621371
CRUISE_DISABLED_CHAR = '–'

@dataclass(frozen=True)
class UIConfig:
  header_height: int = 300
  border_size: int = 30
  button_size: int = 192
  set_speed_width_metric: int = 200
  set_speed_width_imperial: int = 172
  set_speed_height: int = 204
  wheel_icon_size: int = 144

@dataclass(frozen=True)
class FontSizes:
  current_speed: int = 176
  speed_unit: int = 66
  max_speed: int = 66

# Default max speed (will be overridden from carParams)
max_speed: int = 160
set_speed: int = 90

@dataclass(frozen=True)
class Colors:
  white: rl.Color = rl.Color(255, 255, 255, 255)
  disengaged: rl.Color = rl.Color(145, 155, 149, 255)
  override: rl.Color = rl.Color(145, 155, 149, 255)
  engaged: rl.Color = rl.Color(128, 216, 166, 255)
  disengaged_bg: rl.Color = rl.Color(0, 0, 0, 153)
  override_bg: rl.Color = rl.Color(145, 155, 149, 204)
  engaged_bg: rl.Color = rl.Color(128, 216, 166, 204)
  grey: rl.Color = rl.Color(166, 166, 166, 255)
  dark_grey: rl.Color = rl.Color(114, 114, 114, 255)
  black_translucent: rl.Color = rl.Color(0, 0, 0, 166)
  white_translucent: rl.Color = rl.Color(255, 255, 255, 200)
  border_translucent: rl.Color = rl.Color(255, 255, 255, 75)
  header_gradient_start: rl.Color = rl.Color(0, 0, 0, 114)
  header_gradient_end: rl.Color = rl.Color(0, 0, 0, 0)

UI_CONFIG = UIConfig()
FONT_SIZES = FontSizes()
COLORS = Colors()

class HudRenderer:
  def __init__(self):
    """Initialize the HUD renderer."""
    self.is_cruise_set: bool = False
    self.max_speed = max_speed
    self.set_speed = set_speed
    self.sm = SubMaster(['carParams'])

  def update_max_speed_from_cp(self):
    try:
      self.sm.update(0)
      if 'carParams' in self.sm.data:
        cp = self.sm['carParams']
        v = int(getattr(cp, 'vCruiseMax', 0))
        if 100 <= v <= 200:
          self.max_speed = v
    except Exception:
      pass

  def render_header(self):
    self.update_max_speed_from_cp()
    max_text = f"{self.max_speed}"
    max_text_width = measure_text_cached(self._font_semi_bold, max_text, FONT_SIZES.max_speed).x

    # Draw MAX text
    rl.draw_text(max_text, UI_CONFIG.border_size, UI_CONFIG.border_size, FONT_SIZES.max_speed, COLORS.white)

  # The rest of your HUD rendering methods remain unchanged...
