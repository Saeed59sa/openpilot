import os
OUT_DIR = "/data/sdracemode/videos"
def start_recording(with_audio=True):
  os.makedirs(OUT_DIR, exist_ok=True)
  path = os.path.join(OUT_DIR, "sdr_run.mp4")
  print("[SDR-REC] start (stub), audio=", with_audio, "->", path)
  return path
def stop_recording():
  print("[SDR-REC] stop (stub)")
