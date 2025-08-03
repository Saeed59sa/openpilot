from openpilot.system.ui.lib.list_view import toggle_item, multiple_button_item, dual_button_item
from openpilot.system.ui.lib.scroller import Scroller
from openpilot.system.ui.lib.widget import Widget
from openpilot.common.params import Params


class SDpilotDeveloperLayout(Widget):
  def __init__(self):
    super().__init__()
    self._params = Params()

    self._mode = (self._params.get("HybridTACCMode") or b"Auto").decode()
    self._mode_index = 0 if self._mode == "Manual" else 1
    self._smoothness = float(self._params.get("HybridTACCSmoothness") or b"0.5")
    self._switch_delay = float(self._params.get("HybridTACCSwitchDelay") or b"1.0")

    items = [
      toggle_item(
        "Enable HybridTACC",
        initial_state=self._params.get_bool("HybridTACCEnabled"),
        callback=self._toggle_enabled,
      ),
      multiple_button_item(
        "HybridTACC Mode",
        "",
        buttons=["Manual", "Auto"],
        selected_index=self._mode_index,
        callback=self._set_mode,
      ),
      dual_button_item(
        "-",
        "+",
        self._dec_smoothness,
        self._inc_smoothness,
        description=lambda: f"Smoothness: {self._smoothness:.2f}",
        enabled=lambda: self._mode_index == 0,
      ),
      dual_button_item(
        "-",
        "+",
        self._dec_delay,
        self._inc_delay,
        description=lambda: f"Switch Delay: {self._switch_delay:.1f}s",
        enabled=lambda: self._mode_index == 0,
      ),
      toggle_item(
        "Enable HybridTACC Learner",
        initial_state=self._params.get_bool("HybridTACCLearnerEnabled"),
        callback=self._toggle_learner,
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

  def _render(self, rect):
    self._scroller.render(rect)

  # Callbacks
  def _toggle_enabled(self):
    current = self._params.get_bool("HybridTACCEnabled")
    self._params.put_bool("HybridTACCEnabled", not current)

  def _toggle_learner(self):
    current = self._params.get_bool("HybridTACCLearnerEnabled")
    self._params.put_bool("HybridTACCLearnerEnabled", not current)

  def _set_mode(self, index: int):
    self._mode_index = index
    self._mode = "Manual" if index == 0 else "Auto"
    self._params.put("HybridTACCMode", self._mode)

  def _dec_smoothness(self):
    self._smoothness = max(0.0, self._smoothness - 0.05)
    self._params.put("HybridTACCSmoothness", f"{self._smoothness:.2f}")

  def _inc_smoothness(self):
    self._smoothness = min(1.0, self._smoothness + 0.05)
    self._params.put("HybridTACCSmoothness", f"{self._smoothness:.2f}")

  def _dec_delay(self):
    self._switch_delay = max(0.1, self._switch_delay - 0.1)
    self._params.put("HybridTACCSwitchDelay", f"{self._switch_delay:.1f}")

  def _inc_delay(self):
    self._switch_delay = min(3.0, self._switch_delay + 0.1)
    self._params.put("HybridTACCSwitchDelay", f"{self._switch_delay:.1f}")

