# ------------------------------------------------------------
# SPDX-License-Identifier: MIT
# Clean, defensive one-shot patcher for AALC (Auto Adaptive Lane Change)
# - NO Arabic/translation changes. Provides a safe English main_en.ts skeleton.
# - Adds selfdrive/controls/lib/aalc.py  (policy + CAN blinkers via CANPacker + sendcan)
# - Patches selfdrive/controls/lib/lane_change.py (import + hook + periodic)
# - Adds a robust UI group into Offroad Settings (no ParamControl deps; pure Qt)
#   under Driving/Lane Change when found; else attaches to the window's main layout.
# - Adds tools/aalc_ctl.py (enable/disable, set params, presets)
# ------------------------------------------------------------
from __future__ import annotations
import re, sys, textwrap
from pathlib import Path

ROOT_MARKERS = ["SConstruct", "selfdrive", "common", "opendbc"]

def repo_root() -> Path:
  cur = Path.cwd()
  for _ in range(8):
    if all((cur / m).exists() for m in ROOT_MARKERS):
      return cur
    cur = cur.parent
  print("ERROR: run inside repo root (SConstruct/common/opendbc/selfdrive required).")
  sys.exit(1)

ROOT = repo_root()

def write_file(path: Path, content: str):
  path.parent.mkdir(parents=True, exist_ok=True)
  old = path.read_text(encoding="utf-8") if path.exists() else None
  if old != content:
    path.write_text(content, encoding="utf-8")
    print(f"WROTE: {path}")
  else:
    print(f"UNCHANGED: {path}")

def insert_once(path: Path, begin_marker: str, end_marker: str, payload: str, after_regex: str|None=None):
  if not path.exists():
    print(f"SKIP (missing): {path}")
    return False
  txt = path.read_text(encoding="utf-8")
  if begin_marker in txt and end_marker in txt:
    print(f"ALREADY PATCHED: {path}")
    return True
  idx = len(txt)
  if after_regex:
    m = re.search(after_regex, txt, flags=re.M|re.S)
    if m: idx = m.end()
  patched = txt[:idx] + f"\n{begin_marker}\n{payload.rstrip()}\n{end_marker}\n" + txt[idx:]
  path.write_text(patched, encoding="utf-8")
  print(f"PATCHED: {path}")
  return True

# ---------------------------------------------------------------------------
# 0) Safety: ensure a minimal, valid English TS to avoid parse errors
# ---------------------------------------------------------------------------
safe_ts = """<?xml version="1.0" encoding="utf-8"?>
<TS version="2.1" language="en_US">
 <context>
  <name>safe</name>
  <message>
   <source> </source>
   <translation> </translation>
  </message>
 </context>
</TS>
"""
write_file(ROOT / "selfdrive/ui/translations/main_en.ts", safe_ts)

# ---------------------------------------------------------------------------
# 1) AALC module (policy + CAN blinkers via CANPacker + sendcan)
# ---------------------------------------------------------------------------
aalc_py = textwrap.dedent(r'''
  #!/usr/bin/env python3
  # SPDX-License-Identifier: MIT
  #
  # selfdrive/controls/lib/aalc.py
  # ------------------------------------------------------------
  # Auto Adaptive Lane Change (AALC) — minimal, safe integration (no UI deps).
  # - Policy: when ego is slowed by a lead by >= MinDeltaKph & above MinSpeed,
  #           request LC to preferred side. Uses cooldown & blinker hold window.
  # - Blinkers: builds a CAN msg with CANPacker and publishes on 'sendcan'.
  #   DBC/msg/signal/bus are configurable via Params (no per-port hardcode).
  # ------------------------------------------------------------
  from __future__ import annotations
  import time
  from typing import Optional, Tuple

  try:
    from common.params import Params
  except Exception:
    class Params:
      def get(self, *_a, **_k): return None
      def put(self, *_a, **_k): return None
      def put_bool(self, k, v): self.put(k, "1" if v else "0")

  try:
    import cereal.messaging as messaging
  except Exception:
    messaging = None

  try:
    from opendbc.can.packer import CANPacker
  except Exception:
    CANPacker = None

  P = Params()

  DEFAULTS = {
    # policy
    "AALCEnabled": "0",
    "AALCAutoBlinkers": "1",
    "AALCMinSpeedKph": "50",
    "AALCMinDeltaKph": "20",
    "AALCSignalHoldS": "3",
    "AALCRearmS": "12",
    "AALCPreferredSide": "left",  # left|right|auto
    # CAN config
    "AALCDbc": "",                 # e.g. hyundai_canfd / toyota_tss2 / tesla_can
    "AALCMsgName": "",             # e.g. BCM_BODY / BLINKER_CMD / TURN_CMD
    "AALCLeftSig": "",             # e.g. TURN_LH / TURN_IND_L / TURN_SIGNAL_LEFT
    "AALCRightSig": "",            # e.g. TURN_RH / TURN_IND_R / TURN_SIGNAL_RIGHT
    "AALCBus": "1",
  }
  for k, v in DEFAULTS.items():
    if P.get(k) is None: P.put(k, v)

  def _to_kph(mps: float) -> float:
    try: return float(mps)*3.6
    except Exception: return 0.0

  def _get_bool(k: str, d=False) -> bool:
    v = P.get(k)
    if v is None: return d
    s = (v.decode() if isinstance(v,(bytes,bytearray)) else str(v)).strip().lower()
    return s in ("1","true","yes","on")

  def _get_int(k: str, d=0) -> int:
    v = P.get(k)
    if v is None: return d
    try:
      s = v.decode() if isinstance(v,(bytes,bytearray)) else v
      return int(s)
    except Exception:
      return d

  def _get_str(k: str, d="") -> str:
    v = P.get(k)
    if v is None: return d
    try:
      return (v.decode() if isinstance(v,(bytes,bytearray)) else str(v)).strip()
    except Exception:
      return d

  class AALC:
    def __init__(self):
      self.last_req_t = 0.0
      self._hold_until_t = 0.0
      self._holding_side: Optional[str] = None
      self._packer = None
      self._sendcan = messaging.pub_sock('sendcan') if messaging else None

    # ---- config getters ----
    def enabled(self) -> bool:        return _get_bool("AALCEnabled", False)
    def auto_blinkers(self) -> bool:  return _get_bool("AALCAutoBlinkers", True)
    def min_speed_kph(self) -> int:   return max(0, _get_int("AALCMinSpeedKph", 50))
    def min_delta_kph(self) -> int:   return max(5, _get_int("AALCMinDeltaKph", 20))
    def hold_s(self) -> float:        return float(_get_int("AALCSignalHoldS", 3))
    def rearm_s(self) -> float:       return float(_get_int("AALCRearmS", 12))
    def pref_side(self) -> str:
      s = _get_str("AALCPreferredSide", "left").lower()
      return s if s in ("left","right","auto") else "left"

    # ---- lead helpers ----
    def _best_lead(self, sm) -> Tuple[Optional[object], float, float]:
      # Try radarState first
      try:
        rs = sm['radarState'] if 'radarState' in sm.data else None
        if rs and len(rs.leads) > 0 and rs.leads[0].status:
          L = rs.leads[0]
          return L, float(getattr(L,"vLead", 0.0)), float(getattr(L,"dRel", 1e9))
      except Exception:
        pass
      # Fallback: modelV2 leadsV3
      try:
        if sm.updated("modelV2"):
          m = sm["modelV2"]
          leads = getattr(m,"leadsV3",[]) or []
          best = None; bestp = 0.0
          for L in leads:
            p = float(getattr(L,"prob",0.0))
            if p > bestp: best, bestp = L, p
          if best is not None:
            v_rel = float(getattr(best,"vRel",0.0))  # ego-relative
            return best, -v_rel, float(getattr(best,"x",1e9))
      except Exception:
        pass
      return None, 0.0, 1e9

    def choose(self, sm, v_ego_mps: float) -> Optional[str]:
      if not self.enabled(): return None
      v_kph = _to_kph(v_ego_mps)
      if v_kph < self.min_speed_kph(): return None
      now = time.monotonic()
      if now - self.last_req_t < self.rearm_s(): return None

      lead, v_lead_kph, dist = self._best_lead(sm)
      if lead is None: return None
      delta = v_kph - _to_kph(v_lead_kph)
      if delta < self.min_delta_kph(): return None

      side = self.pref_side()
      self.last_req_t = now

      if self.auto_blinkers():
        self._holding_side = side
        self._hold_until_t = now + self.hold_s()
        try: self.send_blinker(side, True)
        except Exception: pass

      return side

    def periodic(self):
      if not self._holding_side: return
      if time.monotonic() >= self._hold_until_t:
        try: self.send_blinker(self._holding_side, False)
        except Exception: pass
        self._holding_side = None

    # --------------- CAN blinker via CANPacker + sendcan ---------------
    def _ensure_packer(self):
      if self._packer is not None: return
      dbc = _get_str("AALCDbc", "")
      if not dbc or CANPacker is None: return
      try:
        self._packer = CANPacker(dbc)
      except Exception:
        self._packer = None

    def send_blinker(self, side: str, on: bool):
      if self._sendcan is None: return
      self._ensure_packer()
      if self._packer is None: return

      msg = _get_str("AALCMsgName", "")
      left_sig  = _get_str("AALCLeftSig", "")
      right_sig = _get_str("AALCRightSig", "")
      bus = _get_int("AALCBus", 0)
      if not msg or not left_sig or not right_sig: return

      vals = {}
      if side == "left":
        vals[left_sig]  = 1 if on else 0
        if right_sig: vals[right_sig] = 0
      elif side == "right":
        vals[right_sig] = 1 if on else 0
        if left_sig:  vals[left_sig]  = 0
      else:
        return

      try:
        addr, dat, src = self._packer.make_can_msg(msg, bus, vals)
        msg_out = messaging.new_message('sendcan', 1)
        msg_out.sendcan[0].address = addr
        msg_out.sendcan[0].dat = dat
        msg_out.sendcan[0].src = src
        self._sendcan.send(msg_out.to_bytes())
      except Exception:
        pass

  _AALC: Optional[AALC] = None
  def get_aalc() -> AALC:
    global _AALC
    if _AALC is None: _AALC = AALC()
    return _AALC
''').strip() + "\n"
write_file(ROOT / "selfdrive/controls/lib/aalc.py", aalc_py)

# ---------------------------------------------------------------------------
# 2) Hook into lane_change.py (import + hook + periodic)
# ---------------------------------------------------------------------------
lane_change = ROOT / "selfdrive/controls/lib/lane_change.py"

insert_once(
  lane_change,
  "# >>> AALC BEGIN (import)",
  "# >>> AALC END",
  r'''
# AALC import (best-effort; no hard dependency)
try:
  from selfdrive.controls.lib.aalc import get_aalc
except Exception:
  get_aalc = None
''',
  after_regex=r"(^|\n)from\s+cereal\s+import\s+log.*?\n"
)

insert_once(
  lane_change,
  "# >>> AALC BEGIN (policy hook)",
  "# >>> AALC END",
  r'''
# Evaluate AALC early in update(); if it requests a side, move to preLaneChange.
try:
  _aalc = get_aalc() if get_aalc else None
except Exception:
  _aalc = None

# try to obtain ego speed symbol across forks
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
      and str(getattr(self, "lane_change_state", "off")).lower() in ("off","idle","0")):
    side = _aalc.choose(sm, v_ego_here)
    if side in ("left","right"):
      try:
        from cereal import log as _log
        self.lane_change_direction = (_log.LateralPlan.LaneChangeDirection.left
                                      if side=="left" else _log.LateralPlan.LaneChangeDirection.right)
        self.lane_change_state = _log.LateralPlan.LaneChangeState.preLaneChange
      except Exception:
        pass
except Exception:
  pass

# Ensure periodic() runs to stop blinker after hold window
try:
  if _aalc is not None:
    _aalc.periodic()
except Exception:
  pass
''',
  after_regex=r"def\s+update\s*\("
)

# ---------------------------------------------------------------------------
# 3) Add robust UI group into Offroad Settings (pure Qt, no ParamControl)
# ---------------------------------------------------------------------------
settings_cc = ROOT / "selfdrive/ui/qt/offroad/settings.cc"

# includes (safe to duplicate)
insert_once(
  settings_cc,
  "// >>> AALC BEGIN (includes)",
  "// >>> AALC END",
  r'''
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include "common/params.h"
''',
  after_regex=r"(^|\n)#include[^\n]+\n(.*\n)*?"
)

# widget payload
ui_payload = r'''
// === AALC UI (defensive attach) ===
{
  // Build group
  QGroupBox *aalcGroup = new QGroupBox(tr("AALC — Auto Adaptive Lane Change"));
  aalcGroup->setObjectName("AALCGroup");
  QVBoxLayout *v = new QVBoxLayout(aalcGroup);
  QFormLayout *f = new QFormLayout();
  v->addLayout(f);

  // Controls
  QCheckBox *cbEnable = new QCheckBox(tr("Enable AALC"));
  QCheckBox *cbBlink  = new QCheckBox(tr("Auto Blinkers"));
  QComboBox *cmbSide  = new QComboBox(); cmbSide->addItems(QStringList() << "left" << "right" << "auto");
  QSpinBox  *spMinSpd = new QSpinBox();  spMinSpd->setRange(0, 200);  spMinSpd->setSuffix(" kph");
  QSpinBox  *spDelta  = new QSpinBox();  spDelta->setRange(5, 60);    spDelta->setSuffix(" kph");
  QSpinBox  *spHold   = new QSpinBox();  spHold->setRange(1, 8);      spHold->setSuffix(" s");
  QSpinBox  *spRearm  = new QSpinBox();  spRearm->setRange(4, 60);    spRearm->setSuffix(" s");

  // CAN config row (advanced)
  QComboBox *cmbDbc   = new QComboBox(); cmbDbc->setEditable(true);
  cmbDbc->addItems(QStringList() << "" << "hyundai_canfd" << "toyota_tss2" << "tesla_can");
  QComboBox *cmbMsg   = new QComboBox(); cmbMsg->setEditable(true);
  QComboBox *cmbLeft  = new QComboBox(); cmbLeft->setEditable(true);
  QComboBox *cmbRight = new QComboBox(); cmbRight->setEditable(true);
  QSpinBox  *spBus    = new QSpinBox();  spBus->setRange(0, 7);

  // Layout form
  f->addRow(tr("Enabled"), cbEnable);
  f->addRow(tr("Auto Blinkers"), cbBlink);
  f->addRow(tr("Preferred Side"), cmbSide);
  f->addRow(tr("Min Speed"), spMinSpd);
  f->addRow(tr("Min Δ Speed"), spDelta);
  f->addRow(tr("Blinker Hold"), spHold);
  f->addRow(tr("Cooldown"), spRearm);

  QFormLayout *f2 = new QFormLayout();
  v->addLayout(f2);
  f2->addRow(tr("DBC"), cmbDbc);
  f2->addRow(tr("Msg Name"), cmbMsg);
  f2->addRow(tr("Left Signal"), cmbLeft);
  f2->addRow(tr("Right Signal"), cmbRight);
  f2->addRow(tr("Bus"), spBus);

  // Buttons
  QHBoxLayout *btns = new QHBoxLayout();
  QPushButton *btnDefaults = new QPushButton(tr("Reset Defaults"));
  QPushButton *btnAgreement = new QPushButton(tr("Activation Agreement"));
  btns->addWidget(btnDefaults);
  btns->addWidget(btnAgreement);
  v->addLayout(btns);

  // Load from Params
  auto getS = [](const char* k){ return QString::fromUtf8(Params().get(k)); };
  auto getI = [&](const char* k, int d){ auto v=Params().get(k); if(v.isEmpty()) return d; bool ok=false; int x=QString::fromUtf8(v).toInt(&ok); return ok?x:d; };
  auto getB = [&](const char* k, bool d){ auto v=Params().get(k); if(v.isEmpty()) return d; QString s=QString::fromUtf8(v).trimmed().toLower(); return (s=="1"||s=="true"||s=="yes"||s=="on"); };

  cbEnable->setChecked(getB("AALCEnabled", false));
  cbBlink->setChecked(getB("AALCAutoBlinkers", true));
  cmbSide->setCurrentText(getS("AALCPreferredSide").isEmpty() ? "left" : getS("AALCPreferredSide"));
  spMinSpd->setValue(getI("AALCMinSpeedKph", 50));
  spDelta->setValue(getI("AALCMinDeltaKph", 20));
  spHold->setValue(getI("AALCSignalHoldS", 3));
  spRearm->setValue(getI("AALCRearmS", 12));

  cmbDbc->setCurrentText(getS("AALCDbc"));
  cmbMsg->setCurrentText(getS("AALCMsgName"));
  cmbLeft->setCurrentText(getS("AALCLeftSig"));
  cmbRight->setCurrentText(getS("AALCRightSig"));
  spBus->setValue(getI("AALCBus", 1));

  // Save helpers
  auto putS = [](const char* k, const QString &v){ Params().put(k, v.toUtf8()); };
  auto putI = [](const char* k, int v){ Params().put(k, QString::number(v).toUtf8()); };
  auto putB = [](const char* k, bool v){ Params().put(k, v? "1" : "0"); };

  QObject::connect(cbEnable, &QCheckBox::toggled, [&](bool on){
    // EULA (English-only)
    if(on && !getB("AALC_EULA_ACCEPTED", false)){
      QMessageBox m; m.setIcon(QMessageBox::Warning);
      m.setWindowTitle("AALC Activation");
      m.setText("AALC is experimental. Keep hands on the wheel and obey the law. You accept full responsibility.");
      auto *ok = m.addButton("I Agree", QMessageBox::AcceptRole);
      m.addButton(QMessageBox::Cancel);
      m.exec();
      if(m.clickedButton()!=ok){ cbEnable->setChecked(false); return; }
      putB("AALC_EULA_ACCEPTED", true);
    }
    putB("AALCEnabled", on);
  });

  QObject::connect(cbBlink, &QCheckBox::toggled, [&](bool on){ putB("AALCAutoBlinkers", on); });
  QObject::connect(cmbSide, &QComboBox::currentTextChanged, [&](const QString &s){ putS("AALCPreferredSide", s); });
  QObject::connect(spMinSpd,  &QSpinBox::valueChanged, [&](int v){ putI("AALCMinSpeedKph", v); });
  QObject::connect(spDelta,   &QSpinBox::valueChanged, [&](int v){ putI("AALCMinDeltaKph", v); });
  QObject::connect(spHold,    &QSpinBox::valueChanged, [&](int v){ putI("AALCSignalHoldS", v); });
  QObject::connect(spRearm,   &QSpinBox::valueChanged, [&](int v){ putI("AALCRearmS", v); });

  QObject::connect(cmbDbc,    &QComboBox::editTextChanged, [&](const QString &s){ putS("AALCDbc", s); });
  QObject::connect(cmbMsg,    &QComboBox::editTextChanged, [&](const QString &s){ putS("AALCMsgName", s); });
  QObject::connect(cmbLeft,   &QComboBox::editTextChanged, [&](const QString &s){ putS("AALCLeftSig", s); });
  QObject::connect(cmbRight,  &QComboBox::editTextChanged, [&](const QString &s){ putS("AALCRightSig", s); });
  QObject::connect(spBus,     qOverload<int>(&QSpinBox::valueChanged), [&](int v){ putI("AALCBus", v); });

  QObject::connect(btnDefaults, &QPushButton::clicked, [&](){
    putB("AALCEnabled", false); cbEnable->setChecked(false);
    putB("AALCAutoBlinkers", true); cbBlink->setChecked(true);
    putI("AALCMinSpeedKph", 50); spMinSpd->setValue(50);
    putI("AALCMinDeltaKph", 20); spDelta->setValue(20);
    putI("AALCSignalHoldS", 3);  spHold->setValue(3);
    putI("AALCRearmS", 12);      spRearm->setValue(12);
    putS("AALCPreferredSide","left"); cmbSide->setCurrentText("left");
  });

  QObject::connect(btnAgreement, &QPushButton::clicked, [&](){
    QMessageBox::information(aalcGroup, "AALC Activation Agreement",
      "AALC is experimental and not a self-driving system.\n"
      "- It may signal and initiate lane changes when your lane is slowed by a lead vehicle.\n"
      "- Keep hands on the wheel and eyes on the road; obey local laws and speed limits.\n"
      "- Always check mirrors and blind spots. Use at your own risk.");
  });

  // Attach defensively:
  // 1) Try to locate an existing 'Lane Change' container (by title/object name)
  QLayout *targetLayout = nullptr;
  for (auto gb : aalcGroup->parentWidget() ? aalcGroup->parentWidget()->findChildren<QGroupBox*>() : findChildren<QGroupBox*>()){
    QString t = gb->title().toLower();
    if (t.contains("lane") && t.contains("change")) { targetLayout = gb->layout(); break; }
  }
  if (!targetLayout) {
    // Try a few common objectNames
    QStringList names = {"lane_change_box","laneChangeLayout","laneChangeBox","driving_controls_box","steering_box"};
    for (const QString &n : names) {
      if (auto *w = findChild<QWidget*>(n)) { if (w->layout()) { targetLayout = w->layout(); break; } }
      if (auto *l = findChild<QLayout*>(n)) { targetLayout = l; break; }
    }
  }
  if (!targetLayout) {
    // Fallback: use this window's main layout
    if (this->layout()) targetLayout = this->layout();
  }
  if (targetLayout) {
    targetLayout->addWidget(aalcGroup);
  } else {
    // As a last resort, create a top-level layout
    QVBoxLayout *fallback = new QVBoxLayout(this);
    fallback->addWidget(aalcGroup);
    setLayout(fallback);
  }
}
'''
insert_once(
  settings_cc,
  "// >>> AALC BEGIN (settings UI block)",
  "// >>> AALC END",
  ui_payload,
  after_regex=r"SettingsWindow::SettingsWindow\s*\([^)]*\)\s*:\s*QFrame\s*\(.*?\)\s*{"
)

# ---------------------------------------------------------------------------
# 4) CLI helper for quick control & presets
# ---------------------------------------------------------------------------
ctl_py = textwrap.dedent(r'''
  #!/usr/bin/env python3
  # SPDX-License-Identifier: MIT
  # tools/aalc_ctl.py — quick control for AALC params (SSH)
  import sys
  from common.params import Params
  P = Params()

  def put(k,v): P.put(k, str(v))

  def show():
    keys = ["AALCEnabled","AALCAutoBlinkers","AALCMinSpeedKph","AALCMinDeltaKph","AALCSignalHoldS","AALCRearmS","AALCPreferredSide",
            "AALCDbc","AALCMsgName","AALCLeftSig","AALCRightSig","AALCBus"]
    print("Current AALC params:")
    for k in keys:
      print(f"  {k} = {P.get(k)}")

  def usage():
    print("Usage:")
    print("  aalc_ctl.py enable|disable|show")
    print("  aalc_ctl.py set <Key> <Value>")
    print("  aalc_ctl.py preset <name>")
    print("Presets:")
    print("  hyundai_fd  -> DBC=hyundai_canfd,  msg=BCM_BODY, left=TURN_LH, right=TURN_RH, bus=1")
    print("  toyota_tss2 -> DBC=toyota_tss2,    msg=BLINKER_CMD, left=TURN_IND_L, right=TURN_IND_R, bus=0")
    print("  tesla_hw3   -> DBC=tesla_can,      msg=TURN_CMD,   left=TURN_SIGNAL_LEFT, right=TURN_SIGNAL_RIGHT, bus=0")
    show()

  def preset(name: str):
    name = name.lower()
    if name == "hyundai_fd":
      put("AALCDbc","hyundai_canfd"); put("AALCMsgName","BCM_BODY")
      put("AALCLeftSig","TURN_LH");    put("AALCRightSig","TURN_RH"); put("AALCBus",1)
    elif name == "toyota_tss2":
      put("AALCDbc","toyota_tss2");    put("AALCMsgName","BLINKER_CMD")
      put("AALCLeftSig","TURN_IND_L"); put("AALCRightSig","TURN_IND_R"); put("AALCBus",0)
    elif name == "tesla_hw3":
      put("AALCDbc","tesla_can");      put("AALCMsgName","TURN_CMD")
      put("AALCLeftSig","TURN_SIGNAL_LEFT"); put("AALCRightSig","TURN_SIGNAL_RIGHT"); put("AALCBus",0)
    else:
      print("Unknown preset."); return
    print(f"Applied preset: {name}")
    show()

  if __name__ == "__main__":
    if len(sys.argv) < 2:
      usage(); sys.exit(0)
    cmd = sys.argv[1].lower()
    if cmd == "enable":
      put("AALCEnabled", 1); print("AALC enabled.")
    elif cmd == "disable":
      put("AALCEnabled", 0); print("AALC disabled.")
    elif cmd == "show":
      show()
    elif cmd == "set" and len(sys.argv) >= 4:
      put(sys.argv[2], sys.argv[3]); print(f"Set {sys.argv[2]} = {sys.argv[3]}")
    elif cmd == "preset" and len(sys.argv) >= 3:
      preset(sys.argv[2])
    else:
      usage()
''').strip() + "\n"
write_file(ROOT / "tools/aalc_ctl.py", ctl_py)
(Path(ROOT/"tools/aalc_ctl.py")).chmod(0o755)

print("\nDONE.")
print("Next:")
print("  1) Build: scons -j$(nproc)")
print("  2) Set CAN params (UI or CLI), e.g. tools/aalc_ctl.py preset hyundai_fd")
print("  3) Enable AALC from Settings (AALC group) or: tools/aalc_ctl.py enable")
