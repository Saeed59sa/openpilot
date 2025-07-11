import os
import sys
import time
import json
import logging
import subprocess
import importlib

from openpilot.common.params import Params

REQUIRED_PACKAGES = ["vosk", "sounddevice", "numpy", "pyttsx3"]

# Auto install missing packages
for pkg in REQUIRED_PACKAGES:
    if importlib.util.find_spec(pkg) is None:
        print(f"Installing required package: {pkg}...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", pkg])

import sounddevice as sd  # type: ignore
import numpy as np  # type: ignore
from vosk import Model, KaldiRecognizer  # type: ignore
import pyttsx3  # type: ignore

LOG_PATH = "/data/voice_commands.log"
logging.basicConfig(filename=LOG_PATH, level=logging.INFO,
                    format="%(asctime)s %(message)s")

MODEL_PATH = os.getenv("VOICE_MODEL_PATH", "/data/vosk-model-small-en-us-0.15")
SAMPLE_RATE = 16000

params = Params()


def speak(text: str) -> None:
    try:
        engine = pyttsx3.init()
        engine.say(text)
        engine.runAndWait()
    except Exception as e:
        logging.error("TTS error: %s", e)


def check_microphone() -> bool:
    devices = sd.query_devices()
    mics = [d for d in devices if d.get('max_input_channels', 0) > 0]
    if not mics:
        logging.warning("No microphone detected")
        speak("Warning. No microphone detected")
        return False
    return True


def listen_once(timeout: float = 5.0) -> str:
    """Listen for a single phrase and return recognized text."""
    with sd.RawInputStream(samplerate=SAMPLE_RATE, blocksize=8000, dtype='int16',
                           channels=1) as stream:
        rec = KaldiRecognizer(Model(MODEL_PATH), SAMPLE_RATE)
        start = time.time()
        while time.time() - start < timeout:
            data, _ = stream.read(4000)
            if rec.AcceptWaveform(data):
                res = json.loads(rec.Result())
                return res.get('text', '')
        return ""


def confirm(prompt: str) -> bool:
    speak(prompt)
    text = listen_once(8.0)
    return text.lower().strip() in ["yes", "confirm", "yep"]


def handle_command(cmd: str) -> None:
    cmd = cmd.lower().strip()
    logging.info("Command: %s", cmd)
    if cmd == "start system":
        params.put_bool("OpenpilotEnabledToggle", True)
        speak("System started")
    elif cmd == "stop system":
        if confirm("Are you sure you want to stop?"):
            params.put_bool("OpenpilotEnabledToggle", False)
            speak("System stopped")
        else:
            speak("Canceling")
    elif cmd == "increase speed":
        speak("Increasing speed")
    elif cmd == "decrease speed":
        speak("Decreasing speed")
    elif cmd == "turn left":
        if confirm("Turn left confirmed?"):
            speak("Turning left")
    elif cmd == "turn right":
        if confirm("Turn right confirmed?"):
            speak("Turning right")
    elif cmd == "status check":
        status = "onroad" if params.get_bool("IsOnroad") else "offroad"
        speak(f"System status {status}")
    elif cmd == "microphone check":
        if check_microphone():
            speak("Microphone is working")
    elif cmd == "log check":
        try:
            with open(LOG_PATH) as f:
                last = list(f)[-1].strip()
            speak("Last log entry" + last)
        except Exception:
            speak("No log available")
    else:
        speak("Unknown command")


def main() -> None:
    if not os.path.isdir(MODEL_PATH):
        logging.error("Vosk model not found at %s", MODEL_PATH)
        speak("Voice model missing")
        return
    check_microphone()
    speak("Voice assistant started")

    model = Model(MODEL_PATH)
    rec = KaldiRecognizer(model, SAMPLE_RATE)

    with sd.RawInputStream(samplerate=SAMPLE_RATE, blocksize=8000, dtype='int16', channels=1) as stream:
        while True:
            data, _ = stream.read(4000)
            if rec.AcceptWaveform(data):
                res = json.loads(rec.Result())
                text = res.get('text', '')
                if text:
                    handle_command(text)


if __name__ == "__main__":
    main()
