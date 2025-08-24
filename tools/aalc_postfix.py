#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/aalc_postfix.py
Post-fix script to harden AALC integration and avoid prior issues.

What it does:
  - Ensure: selfdrive/controls/lib/aalc.py (SPDX: MIT) + tools/aalc_ctl.py
  - Patch:  selfdrive/controls/lib/lane_change.py  (import + preLaneChange hook + periodic)
  - Patch:  selfdrive/ui/qt/offroad/settings.cc    (Qt includes + fix connect 3-args -> 4-args)
  - Translations:
      * Update ONLY main_en.ts + main_ar.ts with AALC strings (create context if missing)
      * Remove AALC strings from other languages to reduce noise
  - Optional: build with scons

Usage:
  python3 tools/aalc_postfix.py --dry-run
  python3 tools/aalc_postfix.py --apply [--build]
"""

from __future__ import annotations
import argparse, re, stat, subprocess
from pathlib import Path
from typing import Dict, List

ROOT_MARKERS = ["SConstruct", "selfdrive", "common", "opendbc"]

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
AALC_CONTEXT = "OffroadSettings"

# ---------------- Core helpers ----------------
def repo_root() -> Path:
  cur = Path.cwd()
  for _ in range(8):
    if all((cur / m).exists() for m in ROOT_MARKERS):
      return cur
    cur = cur.parent
  raise SystemExit("Run inside repo (must contain: SConstruct, selfdrive, common, opendbc)")

ROOT = repo_root()

def read(p: Path) -> str:
  return p.read_text(encoding="utf-8", errors="ignore")

def write_with_backup(p: Path, txt: str, apply_changes: bool) -> bool:
  if not apply_changes:
    return False
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
  v=P.get(k); 
  if v is None: return d
  s=(v.decode() if isinstance(v,(bytes,bytearray)) else str(v)).strip().lower()
  return s in ("1","true","yes","on")
def _get_int(k:str,d=0)->int:
  v=P.get(k); 
  if v is None: return d
  try:
    s=v.decode() if isinstance(v,(bytes,bytearray)) else v
    return int(s)
  except Exception: return d
def _get_str(k:str,d="")->str:
  v=P.get(k); 
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
    if "SPDX-License-Identifier: MIT" not in src:
      src = "# SPDX-License-Identifier: MIT\n" + src
      changed = write_with_backup(path, src, args.apply)
  print_change("aalc.py", path, changed, args.apply)
  if path.exists():
    ensure_exec(path)

def ensure_ctl_py(apply_changes: bool):
  path = ROOT / "tools/aalc_ctl.py"
  if path.exists():
    print("[OK] tools/aalc_ctl.py present")
    return
  template = """#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
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
  print_change("aalc_ctl.py", path, changed, args.apply)
  if path.exists():
    ensure_exec(path)

# ---------------- 2) lane_change.py hook ----------------
def insert_once(path: Path, begin_marker: str, end_marker: str, payload: str, after_regex: str|None=None) -> bool:
  if not path.exists():
    print(f"[SKIP] missing: {path}"); return False
  src = read(path)
  if begin_marker in src and end_marker in src:
    print(f"[OK] already patched: {path}")
    return False
  idx = len(src)
  if after_regex:
    m = re.search(after_regex, src, flags=re.M|re.S)
    if m: idx = m.end()
  new_src = src[:idx] + f"\n{begin_marker}\n{payload.rstrip()}\n{end_marker}\n" + src[idx:]
  return write_with_backup(path, new_src, args.apply)

def patch_lane_change(apply_changes: bool):
  p = ROOT / "selfdrive/controls/lib/lane_change.py"
  changed = False
  changed |= insert_once(
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
  changed |= insert_once(
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
  print_change("lane_change.py", p, changed, apply_changes)

# ---------------- 3) settings.cc: includes + fix connect lambdas ----------------
CONNECT_LAMBDA_3ARGS = re.compile(
  r"""
  (?P<prefix>\b(?:QObject::)?connect\s*\(\s*
      [^,()]+?\s*,\s*
      (?:&\s*[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*::[A-Za-z_]\w*
        |QOverload<[^>]+>::of\(\s*&\s*[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*::[A-Za-z_]\w*\s*\)
      )
      \s*,\s*
  )
  (?![A-Za-z_]\w*\s*,)         # not already 4-arg
  (?P<lambda_intro>\[[^\]]*\])
  (?P<rest>\s*\()
  """, re.VERBOSE | re.MULTILINE | re.DOTALL,
)
ALREADY_4ARGS_NEARBY = re.compile(r",\s*[A-Za-z_]\w*\s*,\s*\[")

def patch_settings_cc(apply_changes: bool):
  p = ROOT / "selfdrive/ui/qt/offroad/settings.cc"
  if not p.exists():
    print(f"[SKIP] missing: {p}"); return
  src = read(p)

  # Ensure includes
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

  # Fix 3-arg connect -> 4-arg (insert 'this,' before lambda)
  def _repl(m):
    start = m.start()
    probe_start = max(0, start-80)
    if ALREADY_4ARGS_NEARBY.search(src[probe_start:start]):
      return m.group(0)
    return f"{m.group('prefix')}this, {m.group('lambda_intro')}{m.group('rest')}"
  new_src = CONNECT_LAMBDA_3ARGS.sub(_repl, src)
  changed = (new_src != src)
  if changed:
    write_with_backup(p, new_src, apply_changes)
  print_change("settings.cc (includes+connect fix)", p, changed, apply_changes)

# ---------------- 4) Translations: update EN/AR only, remove others ----------------
def find_ts_files(dirp: Path) -> List[Path]:
  return [q for q in sorted(dirp.glob("*.ts")) if q.is_file()]

def ensure_context(ts_txt: str, ctx: str) -> str:
  if re.search(rf'<context>\s*<name>\s*{re.escape(ctx)}\s*</name>', ts_txt):
    return ts_txt
  block = f'\n  <context>\n    <name>{ctx}</name>\n  </context>\n'
  return re.sub(r'</TS>\s*$', block + '</TS>', ts_txt)

def ensure_message(ts_txt: str, ctx: str, source: str, translation: str) -> str:
  parts = re.split(r'(<context>.*?</context>)', ts_txt, flags=re.S)
  out, found_ctx = [], False
  for part in parts:
    if not part.startswith("<context>"):
      out.append(part); continue
    if re.search(rf'<name>\s*{re.escape(ctx)}\s*</name>', part):
      found_ctx = True
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
  if not found_ctx:
    ts_txt = re.sub(r'</TS>\s*$', ("  <context>\n"
                                   f"    <name>{ctx}</name>\n"
                                   "  </context>\n</TS>"), ts_txt)
    return ensure_message(ts_txt, ctx, source, translation)
  return ''.join(out)

def remove_aalc_msgs(ts_txt: str, ctx: str) -> str:
  def strip_ctx(block: str) -> str:
    for src in AALC_STRS.keys():
      block = re.sub(
        rf'\s*<message>\s*<source>\s*{re.escape(src)}\s*</source>.*?</message>\s*',
        '\n', block, flags=re.S)
    return block
  parts = re.split(r'(<context>.*?</context>)', ts_txt, flags=re.S)
  out=[]
  for part in parts:
    if not part.startswith("<context>"):
      out.append(part); continue
    if re.search(rf'<name>\s*{re.escape(ctx)}\s*</name>', part):
      out.append(strip_ctx(part))
    else:
      out.append(part)
  return ''.join(out)

def patch_translations(apply_changes: bool):
  tdir = ROOT / "selfdrive" / "ui" / "translations"
  if not tdir.exists():
    print(f"[WARN] translations dir missing: {tdir}")
    return
  files = find_ts_files(tdir)
  if not files:
    print("[WARN] no .ts files found")
    return
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
    if changed:
      write_with_backup(ts, txt, apply_changes)
    print_change(f"translations [{lang or 'unknown'}]", ts, changed, apply_changes)

# ---------------- 5) optional build ----------------
def maybe_build(do_build: bool):
  if not do_build:
    print("[SKIP] build (use --build to run scons)")
    return
  try:
    print("[BUILD] running: scons -j$(nproc)")
    subprocess.check_call(["bash","-lc","scons -j$(nproc)"])
    print("[BUILD] OK")
  except Exception as e:
    print(f"[BUILD] failed: {e}")

# ---------------- CLI ----------------
ap = argparse.ArgumentParser()
ap.add_argument("--apply", action="store_true", help="write changes (default: dry-run)")
ap.add_argument("--build", action="store_true", help="run scons after applying fixes")
args = ap.parse_args()

# Execute
ensure_aalc_py(args.apply)
ensure_ctl_py(args.apply)
patch_lane_change(args.apply)
patch_settings_cc(args.apply)
patch_translations(args.apply)
maybe_build(args.apply and args.build)

print("\nSummary:")
print(" - Dry-run = NO changes written" if not args.apply else " - Changes written (+ .bak_fix backups).")
print(" - Build skipped" if not args.build else " - Build attempted via scons.")

