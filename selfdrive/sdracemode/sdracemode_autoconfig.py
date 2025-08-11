def run_autoconfig():
  """
  Inspect carParams/fingerprint (placeholder) and set defaults:
   - SDR_EV, SDR_DualMotor, SDR_HasDrift, SDR_HasTorqueSplit, SDR_HasFrontMotorControl
   - SDR_Unit (kW for EV, HP for ICE)
  """
  is_ev = True   # TODO: detect from carParams
  dual = True    # TODO: detect dual motor
  flags = {
    "SDR_EV": "1" if is_ev else "0",
    "SDR_DualMotor": "1" if dual else "0",
    "SDR_HasDrift": "1" if is_ev else "0",
    "SDR_HasTorqueSplit": "1" if is_ev else "0",
    "SDR_HasFrontMotorControl": "1" if (is_ev and dual) else "0",
    "SDR_Unit": "kW" if is_ev else "HP"
  }
  try:
    from openpilot.common.params import Params
    p=Params()
    for k,v in flags.items(): p.put(k,v)
  except Exception:
    pass
  return flags
