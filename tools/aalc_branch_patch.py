#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/aalc_branch_patch.py
Author: Saeed ALmansoori
SPDX-License-Identifier: MIT

Purpose:
  Patch & enhance AALC on a specific branch:
   - Ensure AALC core (aalc.py) + CLI (tools/aalc_ctl.py) with MIT header and author signature
   - Hook lane_change.py (import + preLaneChange transition + periodic)
   - Fix Qt connects in settings.cc (3-arg -> 4-arg) and ensure required includes
   - Update translations ONLY for EN/AR (add AALC strings), remove accidental AALC keys from others
   - Optional build with scons

   Extras (safe, branch-aligned enhancements):
   - Add 10s countdown gate (delay timer) for AALC activation in desire_helper.py (best-effort)
   - On-road UI overlay countdown & visual blink in annotated_camera.(h|cc) (best-effort)
   - Raise slower_lead threshold to 20 kph in frogpilot_planner.py (best-effort)
"""

from __future__ import annotations
import argparse, re, stat, subprocess, sys
from pathlib import Path
from typing import Dict, List

ROOT_MARKERS = ["SConstruct", "selfdrive", "common", "opendbc"]
AALC_CONTEXT = "OffroadSettings"

AALC_STRS: Dict[str, Dict[str, str]] = {
  "AALC — Auto Adaptive Lane Change": {"ar": "AALC — الانتقال التلقائي التكيّفي بين الحارات"},
  "Enable AALC": {"ar": "تفعيل AALC"},
  "Auto Blinkers": {"ar": "إشارات الانعطاف التلقائية"},
  "Preferred Side": {"ar": "الجهة المفضلة"},
  "Min Speed": {"ar": "أدنى سرعة"},
  "Min Δ Speed": {"ar": "فارق السرعة الأدنى"},
  "Blinker Hold": {"ar": "مدة إبقاء الإشارة"},
  "Cooldown": {"ar": "فترة التبريد"},
  "DBC": {"ar": "قاعدة بيانات DBC"},
  "Msg Name": {"ar": "اسم الرسالة"},
  "Left Signal": {"ar": "إشارة اليسار"},
  "Right Signal": {"ar": "إشارة اليمين"},
  "Bus": {"ar": "الباص"},
  "Reset Defaults": {"ar": "إعادة القيم الافتراضية"},
  "Activation Agreement": {"ar": "اتفاقية التفعيل"},
  "AALC Activation": {"ar": "تفعيل AALC"},
  "I Agree": {"ar": "أوافق"},
  "Cancel": {"ar": "إلغاء"},
  "AALC Activation Agreement": {"ar": "اتفاقية تفعيل AALC"},
  ("AALC is experimental and provided under the MIT License.\n"
   "- Keep your hands on the wheel and eyes on the road.\n"
   "- AALC may signal/request lane changes when traffic slows due to a lead vehicle.\n"
   "- Always obey local laws and speed limits.\n"
   "- Always check mirrors and blind spots.\n"
   "By enabling AALC, you accept full responsibility."): {
    "ar": ("ميزة AALC تجريبية ومقدّمة بموجب ترخيص MIT.\n"
           "- أبقِ يديك على المقود وناظريك على الطريق.\n"
           "- قد تقوم AALC بتشغيل الإشارات وطلب تغيير الحارة عند تباطؤ السير بسبب مركبة أمامية.\n"
           "- التزم دائمًا بالقوانين والسرعات المحلية.\n"
           "- افحص المرايا والنقاط العمياء دائمًا.\n"
           "بتفعيل AALC، تتحمّل المسؤولية الكاملة.")
  },
}


def repo_root() -> Path:
  cur = Path.cwd()
  for _ in range(8):
    if all((cur / m).exists() for m in ROOT_MARKERS):
      return cur
    cur = cur.parent
  raise SystemExit("Run inside repo (must contain: SConstruct, selfdrive, common, opendbc)")

ROOT = repo_root()


def run(cmd: List[str]) -> subprocess.CompletedProcess:
  return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)


def ensure_branch(name: str, apply_changes: bool):
  if not (ROOT / ".git").exists() or not name:
    print("[INFO] branch check skipped"); return
  cur = run(["bash","-lc","git rev-parse --abbrev-ref HEAD"])
  if cur.returncode == 0 and cur.stdout.strip() == name:
    print(f"[OK] on branch: {name}"); return
  if not apply_changes:
    print(f"[WOULD] checkout branch {name}"); return
  cp = run(["bash","-lc", f"git checkout {name} || git checkout -b {name}"])
  if cp.returncode != 0:
    print(cp.stdout); print(cp.stderr)
    raise SystemExit(f"[ERR] cannot checkout/create branch {name}")
  print(f"[OK] checked out {name}")


def read(p: Path) -> str:
  return p.read_text(encoding="utf-8", errors="ignore")


def write_with_backup(p: Path, txt: str, apply_changes: bool) -> bool:
  if not apply_changes: return False
  p.parent.mkdir(parents=True, exist_ok=True)
  bak = p.with_suffix(p.suffix + ".bak_fix")
  if not bak.exists() and p.exists():
    bak.write_text(read(p), encoding="utf-8")
  p.write_text(txt, encoding="utf-8")
  return True


def ensure_exec(p: Path):
  try:
    mode = p.stat().st_mode
    p.chmod(mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
  except Exception:
    pass


def print_change(label: str, path: Path, changed: bool, apply_changes: bool):
  if changed and apply_changes:
    print(f"[FIXED] {label}: {path}")
  elif changed and not apply_changes:
    print(f"[WOULD FIX] {label}: {path}")

# ---------------- 1) Ensure aalc.py + aalc_ctl.py ----------------

def ensure_aalc_py(apply_changes: bool):
  path = ROOT / "selfdrive/controls/lib/aalc.py"
  template = """#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Author: Saeed ALmansoori
from __future__ import annotations
import time
from typing import Optional, Tuple
try:
  from common.params import Params
except Exception:
  class Params:
    def get(self,*a,**k): return None
    def put(self,*a,**k): return None
    def put_bool(self,k,v): self.put(k,"1" if v else "0")
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
  "AALCEnabled":"0","AALCAutoBlinkers":"1","AALCMinSpeedKph":"50","AALCMinDeltaKph":"20",
  "AALCSignalHoldS":"3","AALCRearmS":"12","AALCPreferredSide":"left",
  "AALCDbc":"","AALCMsgName":"","AALCLeftSig":"","AALCRightSig":"","AALCBus":"1",
}
for k,v in DEFAULTS.items():
  if P.get(k) is None: P.put(k, v)

def _to_kph(mps: float)->float:
  try: return float(mps)*3.6
  except Exception: return 0.0
def _get_bool(k:str,d=False)->bool:
  v=P.get(k)
  if v is None: return d
  s=(v.decode() if isinstance(v,(bytes,bytearray)) else str(v)).strip().lower()
  return s in ("1","true","yes","on")
def _get_int(k:str,d=0)->int:
  v=P.get(k)
  if v is None: return d
  try:
    s=v.decode() if isinstance(v,(bytes,bytearray)) else v
    return int(s)
  except Exception: return d
def _get_str(k:str,d="")->str:
  v=P.get(k)
  if v is None: return d
  try: return (v.decode() if isinstance(v,(bytes,bytearray)) else str(v)).strip()
  except Exception: return d

class AALC:
  def __init__(self):
    self.last_req_t=0.0
    self._hold_until_t=0.0
    self._holding_side: Optional[str]=None
    self._packer=None
    self._sendcan= messaging.pub_sock('sendcan') if messaging else None

  def enabled(self)->bool:       return _get_bool("AALCEnabled",False)
  def auto_blinkers(self)->bool: return _get_bool("AALCAutoBlinkers",True)
  def min_speed_kph(self)->int:  return max(0,_get_int("AALCMinSpeedKph",50))
  def min_delta_kph(self)->int:  return max(5,_get_int("AALCMinDeltaKph",20))
  def hold_s(self)->float:       return float(_get_int("AALCSignalHoldS",3))
  def rearm_s(self)->float:      return float(_get_int("AALCRearmS",12))
  def pref_side(self)->str:
    s=_get_str("AALCPreferredSide","left").lower()
    return s if s in ("left","right","auto") else "left"

  def _best_lead(self, sm)->Tuple[Optional[object],float,float]:
    try:
      rs = sm['radarState'] if 'radarState' in sm.data else None
      if rs and len(rs.leads)>0 and rs.leads[0].status:
        L=rs.leads[0]; return L, float(getattr(L,"vLead",0.0)), float(getattr(L,"dRel",1e9))
    except Exception: pass
    try:
      if sm.updated("modelV2"):
        m=sm["modelV2"]; leads=getattr(m,"leadsV3",[]) or []
        best=None; bestp=0.0
        for L in leads:
          p=float(getattr(L,"prob",0.0))
          if p>bestp: best,bestp=L,p
        if best is not None:
          v_rel=float(getattr(best,"vRel",0.0))
          return best, -v_rel, float(getattr(best,"x",1e9))
    except Exception: pass
    return None,0.0,1e9

  def choose(self, sm, v_ego_mps: float)->Optional[str]:
    if not self.enabled(): return None
    v_kph=_to_kph(v_ego_mps)
    if v_kph<self.min_speed_kph(): return None
    now=time.monotonic()
    if now-self.last_req_t<self.rearm_s(): return None
    lead, v_lead_kph, dist = self._best_lead(sm)
    if lead is None: return None
    delta = v_kph - _to_kph(v_lead_kph)
    if delta < self.min_delta_kph(): return None
    side=self.pref_side(); self.last_req_t=now
    if self.auto_blinkers():
      self._holding_side=side; self._hold_until_t=now+self.hold_s()
      try: self.send_blinker(side, True)
      except Exception: pass
    return side

  def periodic(self):
    if not self._holding_side: return
    if time.monotonic()>=self._hold_until_t:
      try: self.send_blinker(self._holding_side, False)
      except Exception: pass
      self._holding_side=None

  def _ensure_packer(self):
    if self._packer is not None: return
    dbc=_get_str("AALCDbc","")
    if not dbc or CANPacker is None: return
    try: self._packer=CANPacker(dbc)
    except Exception: self._packer=None

  def send_blinker(self, side:str, on:bool):
    if self._sendcan is None: return
    self._ensure_packer()
    if self._packer is None: return
    msg=_get_str("AALCMsgName","")
    left=_get_str("AALCLeftSig",""); right=_get_str("AALCRightSig",""); bus=_get_int("AALCBus",0)
    if not msg or not left or not right: return
    try:
      addr, dat, src = self._packer.make_can_msg(msg, bus, {
        left: 1 if (side=="left" and on) else 0,
        right: 1 if (side=="right" and on) else 0,
      })
      m = messaging.new_message('sendcan', 1)
      m.sendcan[0].address = addr; m.sendcan[0].dat = dat; m.sendcan[0].src = src
      self._sendcan.send(m.to_bytes())
    except Exception: pass

_AALC: Optional[AALC] = None
def get_aalc()->AALC:
  global _AALC
  if _AALC is None: _AALC = AALC()
  return _AALC
"""
  changed = False
  if not path.exists():
    changed = write_with_backup(path, template, args.apply)
  else:
    src = read(path)
    if "SPDX-License-Identifier: MIT" not in src or "Author: Saeed ALmansoori" not in src:
      if "SPDX-License-Identifier: MIT" not in src:
        src = "# SPDX-License-Identifier: MIT\n" + src
      if "Author: Saeed ALmansoori" not in src:
        src = src.replace("# SPDX-License-Identifier: MIT", "# SPDX-License-Identifier: MIT\n# Author: Saeed ALmansoori", 1)
      changed = write_with_backup(path, src, args.apply)
  print_change("aalc.py", path, changed, args.apply)
  if path.exists(): ensure_exec(path)


def ensure_ctl_py(apply_changes: bool):
  path = ROOT / "tools/aalc_ctl.py"
  if path.exists():
    # ensure header/signature
    src = read(path)
    changed = False
    if "SPDX-License-Identifier: MIT" not in src or "Author: Saeed ALmansoori" not in src:
      if "SPDX-License-Identifier: MIT" not in src:
        src = "# SPDX-License-Identifier: MIT\n" + src
      if "Author: Saeed ALmansoori" not in src:
        src = src.replace("# SPDX-License-Identifier: MIT", "# SPDX-License-Identifier: MIT\n# Author: Saeed ALmansoori", 1)
      changed = write_with_backup(path, src, args.apply)
      print_change("aalc_ctl.py (header/signature)", path, changed, args.apply)
    ensure_exec(path)
    return
  template = """#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Author: Saeed ALmansoori
import sys
from common.params import Params
P = Params()
def put(k,v): P.put(k, str(v))
def show():
  keys=["AALCEnabled","AALCAutoBlinkers","AALCMinSpeedKph","AALCMinDeltaKph",
        "AALCSignalHoldS","AALCRearmS","AALCPreferredSide","AALCDbc",
        "AALCMsgName","AALCLeftSig","AALCRightSig","AALCBus","AALC_EULA_ACCEPTED"]
  print("Current AALC params:")
  for k in keys: print(f"  {k} = {P.get(k)}")
def usage():
  print("Usage:\n  aalc_ctl.py enable|disable|show\n  aalc_ctl.py set <Key> <Value>\n  aalc_ctl.py preset <name>")
  print("Presets:\n  hyundai_fd | toyota_tss2 | tesla_hw3"); show()
def preset(name:str):
  n=name.lower()
  if n=='hyundai_fd':
    put('AALCDbc','hyundai_canfd'); put('AALCMsgName','BCM_BODY')
    put('AALCLeftSig','TURN_LH'); put('AALCRightSig','TURN_RH'); put('AALCBus',1)
  elif n=='toyota_tss2':
    put('AALCDbc','toyota_tss2'); put('AALCMsgName','BLINKER_CMD')
    put('AALCLeftSig','TURN_IND_L'); put('AALCRightSig','TURN_IND_R'); put('AALCBus',0)
  elif n=='tesla_hw3':
    put('AALCDbc','tesla_can'); put('AALCMsgName','TURN_CMD')
    put('AALCLeftSig','TURN_SIGNAL_LEFT'); put('AALCRightSig','TURN_SIGNAL_RIGHT'); put('AALCBus',0)
  else:
    print('Unknown preset.'); return
  print(f'Applied preset: {n}'); show()
if __name__=='__main__':
  if len(sys.argv)<2: usage(); sys.exit(0)
  cmd=sys.argv[1].lower()
  if cmd=='enable': put('AALCEnabled',1); print('AALC enabled.')
  elif cmd=='disable': put('AALCEnabled',0); print('AALC disabled.')
  elif cmd=='show': show()
  elif cmd=='set' and len(sys.argv)>=4: put(sys.argv[2],sys.argv[3]); print(f'Set {sys.argv[2]} = {sys.argv[3]}')
  elif cmd=='preset' and len(sys.argv)>=3: preset(sys.argv[2])
  else: usage()
"""
  changed = write_with_backup(path, template, apply_changes)
  print_change("aalc_ctl.py (created)", path, changed, args.apply)
  if path.exists(): ensure_exec(path)

# ---------------- 2) lane_change.py hook ----------------

def insert_once(path: Path, begin_marker: str, end_marker: str, payload: str, after_regex: str|None=None) -> bool:
  if not path.exists(): print(f"[SKIP] missing: {path}"); return False
  src = read(path)
  if begin_marker in src and end_marker in src:
    print(f"[OK] already patched: {path}"); return False
  idx = len(src)
  if after_regex:
    m = re.search(after_regex, src, flags=re.M|re.S)
    if m: idx = m.end()
  new_src = src[:idx] + f"\n{begin_marker}\n{payload.rstrip()}\n{end_marker}\n" + src[idx:]
  return write_with_backup(path, new_src, args.apply)


def patch_lane_change(apply_changes: bool):
  p = ROOT / "selfdrive/controls/lib/lane_change.py"
  ch = False
  ch |= insert_once(
    p, "# >>> AALC BEGIN (import)", "# >>> AALC END",
    r'''
# AALC import (best-effort)
try:
  from selfdrive.controls.lib.aalc import get_aalc
except Exception:
  get_aalc = None
''',
    after_regex=r"(^|\n)from\s+cereal\s+import\s+log.*?\n"
  ) or False
  ch |= insert_once(
    p, "# >>> AALC BEGIN (policy hook)", "# >>> AALC END",
    r'''
# Evaluate AALC in update(); if it returns a side, move to preLaneChange.
try:
  _aalc = get_aalc() if get_aalc else None
except Exception:
  _aalc = None

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
      from cereal import log as _log
      self.lane_change_direction = (_log.LateralPlan.LaneChangeDirection.left
                                    if side=="left" else _log.LateralPlan.LaneChangeDirection.right)
      self.lane_change_state = _log.LateralPlan.LaneChangeState.preLaneChange
except Exception:
  pass

# ensure periodic() runs to stop blinker after hold window
try:
  if _aalc is not None: _aalc.periodic()
except Exception:
  pass
''',
    after_regex=r"def\s+update\s*\("
  ) or False
  print_change("lane_change.py (hook)", p, ch, apply_changes)

# ---------------- 3) settings.cc: includes + fix connect lambdas ----------------
CONNECT_LAMBDA_3ARGS = re.compile(
  r"(?P<prefix>\b(?:QObject::)?connect\s*\(\s*[^,()]+?\s*,\s*(?:&\s*[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*::[A-Za-z_]\w*|QOverload<[^>]+>::of\(\s*&\s*[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*::[A-Za-z_]\w*\s*\))\s*,\s*)(?![A-Za-z_]\w*\s*,)(?P<lambda_intro>\[[^\]]*\])(?P<rest>\s*\()",
  re.VERBOSE | re.MULTILINE | re.DOTALL,
)
ALREADY_4ARGS_NEARBY = re.compile(r",\s*[A-Za-z_]\w*\s*,\s*\[")


def patch_settings_cc(apply_changes: bool):
  p = ROOT / "selfdrive/ui/qt/offroad/settings.cc"
  if not p.exists(): print(f"[SKIP] missing: {p}"); return
  src = read(p)
  need_headers = [
    "QGroupBox","QVBoxLayout","QFormLayout","QHBoxLayout","QLayout",
    "QCheckBox","QComboBox","QSpinBox","QPushButton","QMessageBox"
  ]
  for h in need_headers:
    if re.search(rf'^\s*#\s*include\s*<{re.escape(h)}>', src, flags=re.M) is None:
      m = list(re.finditer(r'^\s*#\s*include[^\n]*\n', src, flags=re.M))
      idx = m[-1].end() if m else 0
      src = src[:idx] + f'#include <{h}>\n' + src[idx:]
  if re.search(r'^\s*#\s*include\s*"common/params.h"', src, flags=re.M) is None:
    m = list(re.finditer(r'^\s*#\s*include[^\n]*\n', src, flags=re.M))
    idx = m[-1].end() if m else 0
    src = src[:idx] + '#include "common/params.h"\n' + src[idx:]
  def _repl(m):
    st = m.start(); probe = max(0, st-80)
    if ALREADY_4ARGS_NEARBY.search(src[probe:st]): return m.group(0)
    return f"{m.group('prefix')}this, {m.group('lambda_intro')}{m.group('rest')}"
  new_src = CONNECT_LAMBDA_3ARGS.sub(_repl, src)
  changed = (new_src != src)
  if changed: write_with_backup(p, new_src, apply_changes)
  print_change("settings.cc (includes+connect fix)", p, changed, apply_changes)

# ---------------- 4) Translations: EN/AR only ----------------

def find_ts_files(dirp: Path) -> List[Path]:
  return [q for q in sorted(dirp.glob("*.ts")) if q.is_file()]


def ensure_context(ts_txt: str, ctx: str) -> str:
  if re.search(rf'<context>\s*<name>\s*{re.escape(ctx)}\s*</name>', ts_txt):
    return ts_txt
  return re.sub(r'</TS>\s*$', f'\n  <context>\n    <name>{ctx}</name>\n  </context>\n</TS>', ts_txt)


def ensure_message(ts_txt: str, ctx: str, source: str, translation: str) -> str:
  parts = re.split(r'(<context>.*?</context>)', ts_txt, flags=re.S)
  out, found = [], False
  for part in parts:
    if not part.startswith("<context>"): out.append(part); continue
    if re.search(rf'<name>\s*{re.escape(ctx)}\s*</name>', part):
      found = True
      if re.search(rf'<message>\s*<source>\s*{re.escape(source)}\s*</source>', part):
        out.append(part)
      else:
        msg = ("    <message>\n"
               f"      <source>{source}</source>\n"
               f"      <translation>{translation}</translation>\n"
               "    </message>\n")
        part = re.sub(r'</context>\s*$', msg + '</context>', part)
        out.append(part)
    else:
      out.append(part)
  if not found:
    ts_txt = re.sub(r'</TS>\s*$', ("  <context>\n"
                                   f"    <name>{ctx}</name>\n"
                                   "  </context>\n</TS>"), ts_txt)
    return ensure_message(ts_txt, ctx, source, translation)
  return ''.join(out)


def remove_aalc_msgs(ts_txt: str, ctx: str) -> str:
  def strip(block: str) -> str:
    for src in AALC_STRS.keys():
      block = re.sub(rf'\s*<message>\s*<source>\s*{re.escape(src)}\s*</source>.*?</message>\s*', '\n', block, flags=re.S)
    return block
  parts = re.split(r'(<context>.*?</context>)', ts_txt, flags=re.S)
  out=[]
  for part in parts:
    if not part.startswith("<context>"): out.append(part); continue
    if re.search(rf'<name>\s*{re.escape(ctx)}\s*</name>', part):
      out.append(strip(part))
    else:
      out.append(part)
  return ''.join(out)


def patch_translations(apply_changes: bool):
  tdir = ROOT / "selfdrive" / "ui" / "translations"
  if not tdir.exists():
    print(f"[WARN] no translations dir: {tdir}"); return
  files = find_ts_files(tdir)
  if not files: print("[WARN] no .ts files found"); return
  for ts in files:
    txt = read(ts)
    m = re.search(r'language\s*=\s*"([^"]+)"', txt)
    lang = m.group(1) if m else ""
    changed = False
    if lang.startswith("en") or lang.startswith("ar"):
      before = txt
      txt = ensure_context(txt, AALC_CONTEXT)
      for src, trs in AALC_STRS.items():
        tr = src if lang.startswith("en") else trs.get("ar", src)
        txt = ensure_message(txt, AALC_CONTEXT, src, tr)
      changed = (txt != before)
    else:
      before = txt
      txt = remove_aalc_msgs(txt, AALC_CONTEXT)
      changed = (txt != before)
    if changed: write_with_backup(ts, txt, apply_changes)
    print_change(f"translations [{lang or 'unknown'}]", ts, changed, apply_changes)

# ---------------- 5) Optional branch-aligned enhancements (best-effort) ----------------

def patch_desire_helper(apply_changes: bool):
  p = ROOT / "selfdrive/controls/lib/desire_helper.py"
  if not p.exists(): print(f"[SKIP] missing: {p}"); return
  src = read(p)
  if re.search(r'\bself\.aalc_delay_timer\b', src) is None:
    src = re.sub(r'(self\.aalc_active\s*=\s*False\s*\n)',
                 r'\1    self.aalc_delay_timer = 0.0\n', src, count=1)
  # Insert simple 10s gate around aalc trigger (non-destructive best-effort)
  if "aalc_delay_timer" not in src:
    src = src.replace("self.aalc_active = True", "self.aalc_active = True\n      self.aalc_delay_timer = 10.0")
  if "aalc_delay_timer = max(0.0" not in src:
    src = src.replace("self.aalc_active = False", "self.aalc_active = False\n    self.aalc_delay_timer = 0.0")
  ch = write_with_backup(p, src, apply_changes)
  print_change("desire_helper.py (delay timer)", p, ch, apply_changes)

def patch_frogpilot_planner(apply_changes: bool):
  p = ROOT / "selfdrive/frogpilot/controls/frogpilot_planner.py"
  if not p.exists(): print(f"[SKIP] missing: {p}"); return
  src = read(p)
  new = re.sub(r'\(\s*v_ego\s*-\s*v_lead\s*\)\s*>=\s*15\s*\*\s*CV\.KPH_TO_MS',
               r'(v_ego - v_lead) >= 20 * CV.KPH_TO_MS', src)
  ch = (new != src)
  if ch: write_with_backup(p, new, apply_changes)
  print_change("frogpilot_planner.py (20kph threshold)", p, ch, apply_changes)

def patch_annotated_camera_h(apply_changes: bool):
  p = ROOT / "selfdrive/ui/qt/onroad/annotated_camera.h"
  if not p.exists(): print(f"[SKIP] missing: {p}"); return
  src = read(p)
  if "#include <QElapsedTimer>" not in src:
    src = src.replace("#include <QVBoxLayout>", "#include <QVBoxLayout>\n#include <QElapsedTimer>")
  if re.search(r'\bdouble\s+aalcCountdown\b;', src) is None:
    src = re.sub(r'(bool\s+aalcActive;\s*)',
                 r'\1\n  double aalcCountdown;\n  QElapsedTimer aalcTimer;', src)
  ch = write_with_backup(p, src, apply_changes)
  print_change("annotated_camera.h (timer fields)", p, ch, apply_changes)

def patch_annotated_camera_cc(apply_changes: bool):
  p = ROOT / "selfdrive/ui/qt/onroad/annotated_camera.cc"
  if not p.exists(): print(f"[SKIP] missing: {p}"); return
  src = read(p)
  if "aalcCountdown = 0.0;" not in src:
    src = re.sub(r'(addWidget\(map_settings_btn[^\n]*\);\s*)',
                 r'\1\n  aalcCountdown = 0.0;\n', src, count=1)
  if "aalcTimer.start();" not in src:
    src = re.sub(
      r'(aalwaysOnLateralActive\s*=\s*scene\.always_on_lateral_active;)',
      r'\1\n  if (scene.aalc_active && !aalcActive) { aalcTimer.start(); aalcCountdown = 10.0; }\n'
      r'  else if (!scene.aalc_active) { aalcCountdown = 0.0; }\n'
      r'  aalcActive = scene.aalc_active;\n'
      r'  if (aalcActive && aalcCountdown > 0.0) { aalcCountdown = fmax(0.0, 10.0 - (aalcTimer.elapsed() / 1000.0)); }\n',
      src, count=1)
  if "AALC ACTIVE" not in src:
    src = re.sub(
      r'(void\s+AnnotatedCameraWidget::paintFrogPilotWidgets\(QPainter\s*&painter\)\s*\{[\s\S]*?)(painter\.restore\(\);\s*\})',
      r'''
\1
  if (aalcActive) {
    painter.save();
    painter.setPen(Qt::white);
    painter.setFont(InterFont(40, QFont::Bold));
    QString msg = aalcCountdown > 0.0 ? tr("AALC in %1s").arg(QString::number(std::ceil(aalcCountdown))) : tr("AALC ACTIVE");
    painter.drawText(QRect(width()/2, 100, width()/2 - 60, 50), Qt::AlignRight, msg);
    painter.restore();
  }
\2''', src, flags=re.S)
  # Visual blink if AALC active (without forcing car's physical blinker)
  if re.search(r'turnSignalAnimation\s*&&\s*\(turnSignalLeft\s*\|\|\s*turnSignalRight\)', src):
    src = re.sub(
      r'(if\s*\(turnSignalAnimation\s*&&\s*\(turnSignalLeft\s*\|\|\s*turnSignalRight\)[^\)]*\)\s*\{)',
      r'if (turnSignalAnimation && (turnSignalLeft || turnSignalRight || aalcActive) && !bigMapOpen && ((!mapOpen && standstillDuration == 0) || signalStyle != "static")) {',
      src)
    if "tempLeft" not in src:
      src = re.sub(
        r'(\{\s*\n\s*if\s*\(!animationTimer->isActive\(\)\)\s*\{)',
        r'''{
    bool tempLeft = turnSignalLeft;
    bool tempRight = turnSignalRight;
    if (aalcActive && !turnSignalLeft && !turnSignalRight) { turnSignalLeft = true; }
\1''', src)
      src = re.sub(
        r'(drawTurnSignals\(painter\);\s*\n\s*)',
        r'\g<1>    turnSignalLeft = tempLeft;\n    turnSignalRight = tempRight;\n',
        src)
  ch = write_with_backup(p, src, apply_changes)
  print_change("annotated_camera.cc (countdown+visual blink)", p, ch, apply_changes)

# ---------------- 6) Build ----------------

def maybe_build(do_build: bool):
  if not do_build:
    print("[SKIP] build (use --build)"); return
  try:
    print("[BUILD] scons -j$(nproc)")
    subprocess.check_call(["bash","-lc","scons -j$(nproc)"])
    print("[BUILD] OK")
  except Exception as e:
    print(f"[BUILD] failed: {e}")

# ---------------- CLI ----------------
ap = argparse.ArgumentParser()
ap.add_argument("--branch", default="", help="target branch to verify/checkout (e.g., fsdpilot-aalc-3)")
ap.add_argument("--apply", action="store_true", help="write changes (default: dry-run)")
ap.add_argument("--build", action="store_true", help="run scons after applying fixes")
args = ap.parse_args()

ensure_branch(args.branch, args.apply)
ensure_aalc_py(args.apply)
ensure_ctl_py(args.apply)
patch_lane_change(args.apply)
patch_settings_cc(args.apply)
patch_translations(args.apply)

# Optional, best-effort enhancements (won't fail if files missing)
patch_desire_helper(args.apply)
patch_frogpilot_planner(args.apply)
patch_annotated_camera_h(args.apply)
patch_annotated_camera_cc(args.apply)

maybe_build(args.apply and args.build)

print("\nSummary:")
print(" - Dry-run = NO changes written" if not args.apply else " - Changes written (+ .bak_fix backups).")
print(" - Build skipped" if not args.build else " - Build attempted.")
