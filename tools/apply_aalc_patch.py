# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 Saeed Almansoori
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

# tools/apply_aalc_patch.py
# ------------------------------------------------------------
# SDpilot / openpilot one-shot patcher: Automatic Adaptive Lane Change (AALC)
# - Creates selfdrive/controls/lib/aalc.py (policy + params + consent gate)
# - Patches selfdrive/controls/lib/lane_change.py (auto request hook)
# - Patches selfdrive/ui/qt/offroad/settings.cc:
#     * Adds includes
#     * Adds AALC UI group with Enable (Agreement) / Disable buttons
#       Agreement text auto-selects Arabic if system language is Arabic, else English
# ------------------------------------------------------------

from __future__ import annotations
import re, sys, textwrap
from pathlib import Path

ROOT_MARKERS = ["SConstruct", "selfdrive", "common", "opendbc"]

def find_repo_root() -> Path:
  cur = Path.cwd()
  for _ in range(8):
    if all((cur / m).exists() for m in ROOT_MARKERS):
      return cur
    cur = cur.parent
  print("ERROR: run inside the repo (couldn't find SConstruct/common/opendbc/selfdrive).")
  sys.exit(1)

ROOT = find_repo_root()

def write_file(path: Path, content: str):
  path.parent.mkdir(parents=True, exist_ok=True)
  old = path.read_text(encoding="utf-8") if path.exists() else None
  if old == content:
    print(f"UNCHANGED: {path}")
  else:
    path.write_text(content, encoding="utf-8")
    print(f"WROTE: {path}")

def insert_once(path: Path, marker_begin: str, marker_end: str, payload: str, after_regex: str | None = None):
  if not path.exists():
    print(f"SKIP (missing): {path}")
    return False
  txt = path.read_text(encoding="utf-8")
  if marker_begin in txt and marker_end in txt:
    print(f"ALREADY PATCHED: {path}")
    return True

  idx = len(txt)
  if after_regex:
    m = re.search(after_regex, txt, flags=re.M | re.S)
    if m:
      idx = m.end()

  patched = txt[:idx] + "\n" + marker_begin + "\n" + payload.rstrip() + "\n" + marker_end + "\n" + txt[idx:]
  path.write_text(patched, encoding="utf-8")
  print(f"PATCHED: {path}")
  return True

# ---------------------------------------------------------------------------
# 1) Create AALC module
# ---------------------------------------------------------------------------
aalc_py = textwrap.dedent(r'''
  #!/usr/bin/env python3
  # SPDX-License-Identifier: MIT
  #
  # selfdrive/controls/lib/aalc.py
  # ------------------------------------------------------------
  # AALC: Automatic Adaptive Lane Change policy for SDpilot/openpilot
  # - Requires user consent param (AALCAgreed=1) to activate
  # ------------------------------------------------------------

  from __future__ import annotations
  import time
  from typing import Optional, Tuple

  try:
    from cereal import log
  except Exception:
    log = None

  try:
    from common.params import Params
  except Exception:
    class Params:
      def get(self, *_args, **_kwargs): return None
      def put(self, *_args, **_kwargs): return None

  _P = Params()

  # ----------------------------
  # Param helpers
  # ----------------------------
  def _get_bool(key: str, default: bool) -> bool:
    v = _P.get(key)
    if v is None: return default
    try: return v.decode() in ("1", "true", "True", "TRUE")
    except Exception: return default

  def _get_int(key: str, default: int) -> int:
    v = _P.get(key)
    if v is None: return default
    try: return int(v.decode())
    except Exception: return default

  def _get_str(key: str, default: str) -> str:
    v = _P.get(key)
    if v is None: return default
    try:
      s = v.decode().strip()
      return s or default
    except Exception:
      return default

  def ensure_default_params():
    defaults = {
      "AALCEnabled": "0",
      "AALCAgreed": "0",            # must be "1" after user accepts agreement
      "AALCAutoBlinker": "1",
      "AALCSpeedDeltaKph": "20",
      "AALCMinSpeedKph": "60",
      "AALCPreferredSide": "left",  # "left"|"right"|"auto"
      "AALCSafetyGapSec": "1.5",
    }
    for k, v in defaults.items():
      if _P.get(k) is None:
        _P.put(k, v)

  ensure_default_params()

  # ----------------------------
  # CAN blinker stub (fill per-vehicle DBC)
  # ----------------------------
  def _aalc_send_blinker_can(direction: str):
    """
    Implement per your car’s DBC:
      - direction: "left" or "right"
      - Send body-control msg to toggle blinker before requesting lane change
    This is intentionally empty to avoid unintended signaling.
    """
    return

  # ----------------------------
  # Core AALC policy
  # ----------------------------
  class AALC:
    def __init__(self):
      self.last_request_ts = 0.0
      self.cooldown_s = 6.0   # prevent rapid re-triggers

    def enabled(self) -> bool:
      # Feature requires BOTH: user enabled + user agreed
      return _get_bool("AALCEnabled", False) and _get_bool("AALCAgreed", False)

    def auto_blinker(self) -> bool:
      return _get_bool("AALCAutoBlinker", True)

    def speed_delta_kph(self) -> int:
      return max(5, _get_int("AALCSpeedDeltaKph", 20))

    def min_speed_kph(self) -> int:
      return max(0, _get_int("AALCMinSpeedKph", 60))

    def preferred_side(self) -> str:
      side = _get_str("AALCPreferredSide", "left").lower()
      return side if side in ("left", "right", "auto") else "left"

    def safety_gap_s(self) -> float:
      try: return max(0.5, float(_get_str("AALCSafetyGapSec", "1.5")))
      except Exception: return 1.5

    # ---- Lead extraction (modelV2.leadsV3) ----
    def _best_lead(self, sm) -> Tuple[Optional[object], float, float]:
      if not sm.updated("modelV2"):
        return None, 0.0, 1e9
      m = sm["modelV2"]
      leads = getattr(m, "leadsV3", []) or []
      best = None; best_p = 0.0
      for L in leads:
        p = getattr(L, "prob", 0.0)
        if p > best_p:
          best, best_p = L, p
      if best is None:
        return None, 0.0, 1e9
      return best, getattr(best, "vRel", 0.0), getattr(best, "x", 1e9)

    def _lane_availability(self, sm) -> Tuple[bool, bool]:
      if not sm.updated("modelV2"):
        return False, False
      probs = list(getattr(sm["modelV2"], "laneLineProbs", []))
      left_ok = right_ok = False
      if len(probs) >= 4:
        left_ok  = probs[0] > 0.3 or probs[1] > 0.35
        right_ok = probs[3] > 0.3 or probs[2] > 0.35
      return left_ok, right_ok

    def choose_direction(self, sm, v_ego_mps: float) -> Optional[str]:
      if not self.enabled(): return None

      v_kph = v_ego_mps * 3.6
      if v_kph < self.min_speed_kph(): return None

      now = time.monotonic()
      if now - self.last_request_ts < self.cooldown_s: return None

      lead, v_rel, x = self._best_lead(sm)
      if lead is None or getattr(lead, "prob", 0.0) < 0.5: return None

      # slower lead? (negative v_rel)
      delta_needed = self.speed_delta_kph() / 3.6
      if v_rel > -delta_needed: return None
      if x > 80.0: return None

      left_ok, right_ok = self._lane_availability(sm)
      pref = self.preferred_side()
      if   pref == "left":  side = "left"  if left_ok  else ("right" if right_ok else None)
      elif pref == "right": side = "right" if right_ok else ("left"  if left_ok  else None)
      else:
        if left_ok and not right_ok:   side = "left"
        elif right_ok and not left_ok: side = "right"
        elif left_ok and right_ok:     side = "left"
        else:                          side = None

      if side is None: return None
      self.last_request_ts = now
      return side

    def prime_blinker(self, side: str):
      if self.auto_blinker() and side in ("left","right"):
        _aalc_send_blinker_can(side)

  _AALC_INSTANCE: Optional[AALC] = None
  def get_aalc() -> AALC:
    global _AALC_INSTANCE
    if _AALC_INSTANCE is None:
      _AALC_INSTANCE = AALC()
    return _AALC_INSTANCE
''').strip() + "\n"

write_file(ROOT / "selfdrive/controls/lib/aalc.py", aalc_py)

# ---------------------------------------------------------------------------
# 2) Patch lane_change.py (import + update hook)
# ---------------------------------------------------------------------------
lane_change_path = ROOT / "selfdrive/controls/lib/lane_change.py"

insert_once(
  lane_change_path,
  "// >>> AALC BEGIN (import)",
  "// >>> AALC END",
  r'''
# AALC: best-effort import (no hard dependency)
try:
  from selfdrive.controls.lib.aalc import get_aalc
except Exception:
  get_aalc = None
''',
  after_regex=r"(^|\n)from\s+cereal\s+import\s+log.*?\n"
)

insert_once(
  lane_change_path,
  "// >>> AALC BEGIN (update policy)",
  "// >>> AALC END",
  r'''
# AALC auto-request policy (called inside update)
try:
  _aalc = get_aalc() if get_aalc else None
except Exception:
  _aalc = None

# try to resolve ego speed symbol
try:
  v_ego_here = v_ego
except NameError:
  try:
    v_ego_here = self.v_ego
  except Exception:
    v_ego_here = 0.0

try:
  if (_aalc is not None
      and lane_change_allowed
      and getattr(self.lane_change_state, "name", str(self.lane_change_state)) in ("off","Off","kOff","IDLE","Idle","0","LCLaneChangeStateOff")):
    chosen = _aalc.choose_direction(sm, v_ego_here)
    if chosen in ("left","right"):
      _aalc.prime_blinker(chosen)

      # set direction across forks
      try:
        from cereal import log as _lc_log
        if hasattr(self, "lane_change_direction"):
          self.lane_change_direction = (_lc_log.LateralPlan.LaneChangeDirection.left
                                        if chosen=="left" else
                                        _lc_log.LateralPlan.LaneChangeDirection.right)
      except Exception:
        pass

      # move to preLaneChange
      try:
        from cereal import log as _lc_log2
        self.lane_change_state = _lc_log2.LateralPlan.LaneChangeState.preLaneChange
      except Exception:
        try:
          self.lane_change_state = type(self.lane_change_state).preLaneChange
        except Exception:
          try:
            self.lane_change_state = "preLaneChange"
          except Exception:
            pass
except Exception:
  # keep non-fatal
  pass
''',
  after_regex=r"def\s+update\s*\(.*?\):"
)

# ---------------------------------------------------------------------------
# 3) Patch settings.cc (includes + UI group with agreement)
# ---------------------------------------------------------------------------
settings_cc = ROOT / "selfdrive/ui/qt/offroad/settings.cc"

# 3a) include block (safe to duplicate)
insert_once(
  settings_cc,
  "// >>> AALC BEGIN (includes)",
  "// >>> AALC END",
  r'''
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include "common/params.h"
''',
  after_regex=r"(^|\n)#include[^\n]+\n(.*\n)*?"
)

# 3b) UI group with agreement logic
ui_payload = r'''
// AALC UI group with activation agreement
{
  auto agreementText = []() -> QString {
    QLocale::Language lang = QLocale::system().language();
    if (lang == QLocale::Arabic) {
      return QString::fromUtf8(
        "اتفاقية تفعيل AALC (التجاوز التلقائي المتكيّف)\n\n"
        "• هذه الميزة تجريبية وليست نظام قيادة مستقل.\n"
        "• عند التفعيل، قد يقوم النظام بتشغيل الغماز وبدء انتقال الحارة عندما يكتشف مركبة أبطأ أمامك.\n"
        "• يجب إبقاء اليدين على المقود والنظر للطريق وتحمل المسؤولية الكاملة عن القيادة.\n"
        "• التزم بقوانين الدولة والسرعات المسموحة، وتحقق من المرايا والنقطة العمياء قبل أي انتقال.\n"
        "• قد لا تعمل الميزة بشكل صحيح في جميع الظروف وقد تتعطل أو تخطئ.\n\n"
        "بالمتابعة، أنت توافق على أنك المسؤول بالكامل عن التحكم بالسيارة واستخدام هذه الميزة على مسؤوليتك."
      );
    } else {
      return QString(
        "AALC Activation Agreement (Automatic Adaptive Lane Change)\n\n"
        "• This feature is experimental and NOT a self-driving system.\n"
        "• When enabled, the system may signal and initiate lane changes upon detecting a slower lead vehicle.\n"
        "• Keep hands on the wheel, eyes on the road, and maintain full responsibility for driving.\n"
        "• Obey local laws and speed limits; check mirrors and blind spots before any lane change.\n"
        "• The feature may not function correctly in all conditions and may fail or behave unexpectedly.\n\n"
        "By proceeding, you acknowledge full responsibility and use this feature at your own risk."
      );
    }
  };

  // Group UI
  QGroupBox *aalcGroup = new QGroupBox(tr("Lane Change – AALC (Auto Adaptive)"));
  QVBoxLayout *aalcLayout = new QVBoxLayout(aalcGroup);

  // Buttons: Enable (Agreement), Disable
  QPushButton *btnEnable = new QPushButton(tr("Enable AALC (Agreement)"));
  QPushButton *btnDisable = new QPushButton(tr("Disable AALC"));

  aalcLayout->addWidget(btnEnable);
  aalcLayout->addWidget(btnDisable);

  // Optional: quick params (visible for tuning convenience)
  // You can still add ParamControl rows elsewhere; here we rely on Python defaults.

  // Enable with agreement
  QObject::connect(btnEnable, &QPushButton::clicked, [this, agreementText]() {
    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("AALC Activation"));
    box.setText(agreementText());
    auto *acceptBtn = box.addButton(tr("I Agree"), QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Cancel);
    box.exec();
    if (box.clickedButton() == acceptBtn) {
      Params().put("AALCAgreed", "1");
      Params().put("AALCEnabled", "1");
      QMessageBox::information(this, tr("AALC Enabled"),
                               tr("AALC has been enabled. You can adjust parameters in Advanced settings if available."));
    }
  });

  // Disable
  QObject::connect(btnDisable, &QPushButton::clicked, []() {
    Params().put("AALCEnabled", "0");
  });

  // Add to main settings
  try {
    main_layout->addWidget(aalcGroup);
  } catch (...) {
    main_layout->addWidget(aalcGroup);
  }
}
'''

insert_once(
  settings_cc,
  "// >>> AALC BEGIN (settings UI with agreement)",
  "// >>> AALC END",
  ui_payload,
  after_regex=r"SettingsWindow::SettingsWindow\s*\([^)]*\)\s*:\s*QFrame\(parent\)\s*{"
)

print("\nDONE.")
print("Next:")
print("  1) Implement blinker CAN in selfdrive/controls/lib/aalc.py -> _aalc_send_blinker_can(direction).")
print("  2) Build & run. Enable via Settings -> 'Enable AALC (Agreement)'.")
print("  3) Params you can tune (default in aalc.py): AALCSpeedDeltaKph, AALCMinSpeedKph, AALCPreferredSide, AALCAutoBlinker, AALCSafetyGapSec.")
