import json
import os


class HybridTACCLearner:
  """Simple learner for HybridTACC parameters."""

  def __init__(self, storage_path: str = "/data/hybrid_tacc/learner.json"):
    self.storage_path = storage_path
    self.smoothness = 0.5
    self.switch_delay = 1.0
    self._load()

  def _load(self) -> None:
    try:
      with open(self.storage_path, "r") as f:
        data = json.load(f)
        self.smoothness = float(data.get("smoothness", self.smoothness))
        self.switch_delay = float(data.get("switch_delay", self.switch_delay))
    except Exception:
      pass

  def _save(self) -> None:
    os.makedirs(os.path.dirname(self.storage_path), exist_ok=True)
    with open(self.storage_path, "w") as f:
      json.dump({"smoothness": self.smoothness, "switch_delay": self.switch_delay}, f)

  def update(self, decision, car_state, radar_state, model_state) -> tuple[float, float]:
    """Update learner with latest driving data.

    This is a very lightweight learner that nudges parameters based on
    radar availability. It persists updated values to disk and returns
    the latest smoothness and switch delay.
    """

    if getattr(radar_state.leadOne, "status", False):
      self.smoothness = min(1.0, self.smoothness + 0.01)
    else:
      self.smoothness = max(0.0, self.smoothness - 0.01)

    # Keep switch delay within bounds
    self.switch_delay = min(3.0, max(0.1, self.switch_delay))

    self._save()
    return self.smoothness, self.switch_delay

