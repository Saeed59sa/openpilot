def ensure_disclaimer_accepted():
  """
  Blocking legal gate (placeholder):
  Real UI should show:
    SDRaceMode is intended for legal, private, or track use only.
    You accept full responsibility. This mode may disable safety systems.
    Do NOT use on public roads. Comply with local laws.
  """
  try:
    from openpilot.common.params import Params
    p=Params()
    if p.get("SDRaceModeAccepted")==b"1":
      return True
    # In real UI: show modal and wait user input. Here we auto-accept for scaffolding.
    p.put("SDRaceModeAccepted","1")
    return True
  except Exception:
    return True
