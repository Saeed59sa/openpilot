from .sdr_autolog import get_autolog_snapshot

def open_sdracemode_ui():
  info = get_autolog_snapshot()
  print("[SDR-UI] open SDR UI (stub). AUTO LOG:", info)

def update_hud(speed=0.0, g=0.0, pwr=0.0, trq=0.0):
  print(f"[SDR-UI] HUD v={speed:.1f} g={g:.2f} P={pwr:.0f} T={trq:.0f}")

def show_results(stats: dict):
  print("[SDR-UI] Results:", stats)
