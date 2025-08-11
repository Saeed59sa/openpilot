from .sdr_bootstrap import main as _boot
from .sdracemode_disclaimer import ensure_disclaimer_accepted
from .sdracemode_autoconfig import run_autoconfig
from .sdracemode_settings import get_settings
from .sdracemode_evcontrols import enable_drift_mode, set_torque_split, disable_front_motor, traction_off
from .sdracemode_recorder import start_recording, stop_recording
from .sdracemode_ui import open_sdracemode_ui, update_hud, show_results

def init_once():
  try: _boot()
  except Exception as e: print("[SDR] bootstrap warn:", e)

def start_session():
  if not ensure_disclaimer_accepted():
    print("[SDR] disclaimer rejected"); return
  flags = run_autoconfig()
  cfg = get_settings()
  # Apply EV controls (stubs)
  if flags.get("SDR_EV")=="1":
    if cfg.get("drift"): enable_drift_mode()
    set_torque_split(cfg.get("torque_split_front",50)/100.0)
    if cfg.get("disable_front_motor"): disable_front_motor()
    if cfg.get("traction_off"): traction_off()
  open_sdracemode_ui()
  # Start recording
  rec_path = start_recording(with_audio=(cfg.get("record_audio","Auto")!="Off"))
  # Simulated run
  for v in (0,30,60,90,120,150,180,200):
    update_hud(speed=float(v), g=0.15, pwr=120.0, trq=300.0)
  stop_recording()
  stats = {"0-60_kmh": 3.9, "0-100_kmh": 7.2, "100-200_kmh": 10.4, "video": rec_path}
  show_results(stats)

# auto-init on import (safe for scaffolding)
init_once()
