SETTINGS = {
  "enabled": False,
  "unit": "Auto",           # Auto/HP/kW
  "record_audio": "Auto",   # Auto/On/Off
  "video_quality": "High",  # Low/Medium/High
  "auto_start": True,
  "show_g": True,
  # EV-only:
  "drift": False,
  "torque_split_front": 50, # 0..100
  "disable_front_motor": False,
  "traction_off": False,
}

def open_sdr_settings():
  print("[SDR-SET] open settings (placeholder)")

def get_settings():
  return dict(SETTINGS)
