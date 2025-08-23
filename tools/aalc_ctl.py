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
