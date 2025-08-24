#!/usr/bin/env python3
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
