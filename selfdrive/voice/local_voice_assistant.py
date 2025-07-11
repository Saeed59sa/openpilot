#!/usr/bin/env python3
"""Local voice assistant for simple offline voice commands."""

from __future__ import annotations

import json
import queue
from datetime import datetime
from pathlib import Path

import sounddevice as sd
from vosk import KaldiRecognizer, Model

LOG_FILE = Path("/data/voice_commands.log")
SAMPLE_RATE = 16000
MODEL_PATH = "/models/vosk"


class LocalVoiceAssistant:
    def __init__(self, model_path: str = MODEL_PATH) -> None:
        self.model = Model(model_path)
        self.recognizer = KaldiRecognizer(self.model, SAMPLE_RATE)
        self.q: queue.Queue[bytes] = queue.Queue()
        self.running = False

    def _callback(self, indata, frames, time, status) -> None:  # pylint: disable=unused-argument
        self.q.put(bytes(indata))

    def _log(self, message: str) -> None:
        LOG_FILE.parent.mkdir(parents=True, exist_ok=True)
        with LOG_FILE.open("a") as f:
            f.write(f"{datetime.utcnow().isoformat()} {message}\n")

    def _confirm(self) -> bool:
        print("Say 'yes' to confirm")
        self._log("Awaiting confirmation")
        while True:
            data = self.q.get()
            if self.recognizer.AcceptWaveform(data):
                res = json.loads(self.recognizer.Result())
                text = res.get("text", "").lower()
                if text:
                    self._log(f"Confirmation heard: {text}")
                    return "yes" in text

    def _start_system(self) -> None:
        print("[ACTION] start system")
        self._log("Action executed: start_system")

    def _stop_system(self) -> None:
        print("[ACTION] stop system")
        self._log("Action executed: stop_system")

    def _increase_speed(self) -> None:
        print("[ACTION] increase speed")
        self._log("Action executed: increase_speed")

    def _decrease_speed(self) -> None:
        print("[ACTION] decrease speed")
        self._log("Action executed: decrease_speed")

    def _turn_left(self) -> None:
        print("[ACTION] turn left")
        self._log("Action executed: turn_left")

    def _turn_right(self) -> None:
        print("[ACTION] turn right")
        self._log("Action executed: turn_right")

    def _handle_command(self, text: str) -> None:
        if "start system" in text:
            self._log(f"Command recognized: {text}")
            self._start_system()
        elif "stop system" in text:
            self._log(f"Command recognized: {text}")
            if self._confirm():
                self._stop_system()
        elif "increase speed" in text:
            self._log(f"Command recognized: {text}")
            self._increase_speed()
        elif "decrease speed" in text:
            self._log(f"Command recognized: {text}")
            self._decrease_speed()
        elif "turn left" in text:
            self._log(f"Command recognized: {text}")
            if self._confirm():
                self._turn_left()
        elif "turn right" in text:
            self._log(f"Command recognized: {text}")
            if self._confirm():
                self._turn_right()

    def run(self) -> None:
        self.running = True
        with sd.RawInputStream(samplerate=SAMPLE_RATE, blocksize=8000, dtype="int16", channels=1, callback=self._callback):
            while self.running:
                data = self.q.get()
                if self.recognizer.AcceptWaveform(data):
                    result = json.loads(self.recognizer.Result())
                    text = result.get("text", "").lower()
                    if text:
                        self._handle_command(text)


def main() -> None:
    LocalVoiceAssistant().run()


if __name__ == "__main__":
    main()
