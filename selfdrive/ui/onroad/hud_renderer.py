# SPDX-License-Identifier: MIT
# Author: Saeed ALmansoori (SDpilot)

import json
from typing import Any


def _aalc_draw_hud(rl: Any, sm: Any) -> None:
  """Draw Auto Adaptive Lane Change status overlay."""
  try:
    st_raw = sm.get("AALCStatus") if hasattr(sm, "get") else None
    if st_raw is None:
      return
    st = json.loads(st_raw) if isinstance(st_raw, (bytes, str)) else st_raw
    text = st.get("text", "AALC")
    dirn = st.get("dir")
    rl.draw_rectangle(20, 10, 380, 36)
    rl.draw_text(f"AALC: {text}", 30, 16, 18)
    if dirn == "left":
      rl.draw_text("<", 400, 16, 24)
    elif dirn == "right":
      rl.draw_text(">", 400, 16, 24)
    gap = st.get("need_min_gap")
    dv = st.get("need_delta_kph")
    rl.draw_text(f"Gap≥{int(gap)}m  Δv≥{int(dv)}kph", 30, 36, 14)
  except Exception:
    pass
