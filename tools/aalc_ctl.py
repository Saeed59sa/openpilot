#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Author: Saeed ALmansoori (SDpilot)

import json
import sys

HELP = """AALC CLI
Usage:
  aalc_ctl.py enable|disable
  aalc_ctl.py set min_gap <meters>
  aalc_ctl.py set delta <kph>
  aalc_ctl.py set max_accel <mps2>
  aalc_ctl.py status
"""

def read_params():
  try:
    from openpilot.common.params import Params
    p = Params()
    def g(k, d=None):
      v = p.get(k)
      return v.decode() if v is not None else d
    return p, g
  except Exception as e:
    print("Params unavailable:", e)
    sys.exit(1)

def main():
  if len(sys.argv) < 2:
    print(HELP)
    return
  p, g = read_params()
  cmd = sys.argv[1]

  if cmd == "enable":
    p.put_bool("AALCEnabled", True)
    print("AALCEnabled = true")
    return
  if cmd == "disable":
    p.put_bool("AALCEnabled", False)
    print("AALCEnabled = false")
    return
  if cmd == "set":
    if len(sys.argv) < 4:
      print(HELP)
      return
    what, val = sys.argv[2], sys.argv[3]
    if what == "min_gap":
      p.put("AALCMinGapM", val)
    elif what == "delta":
      p.put("AALCSpeedDeltaKph", val)
    elif what == "max_accel":
      p.put("AALCMaxAccelMps2", val)
    else:
      print(HELP)
      return
    print(f"Set {what} = {val}")
    return
  if cmd == "status":
    st = {
      "AALCEnabled": g("AALCEnabled", "false"),
      "AALCModeAuto": g("AALCModeAuto", "true"),
      "AALCMinGapM": g("AALCMinGapM", "25.0"),
      "AALCSpeedDeltaKph": g("AALCSpeedDeltaKph", "15.0"),
      "AALCMaxAccelMps2": g("AALCMaxAccelMps2", "1.5"),
    }
    print(json.dumps(st, indent=2))
    return
  print(HELP)

if __name__ == "__main__":
  main()
