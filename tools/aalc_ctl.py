#!/usr/bin/env python3
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
