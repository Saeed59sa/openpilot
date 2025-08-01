#!/usr/bin/env python3
import time
import zmq

import cereal.messaging as messaging


PUB_ADDRESS = "tcp://*:8008"


def main() -> None:
  context = zmq.Context()
  sock = context.socket(zmq.PUB)
  sock.bind(PUB_ADDRESS)

  sm = messaging.SubMaster(["carControl"])

  try:
    while True:
      sm.update(0)
      if sm.updated["carControl"]:
        hud = sm["carControl"].hudControl
        hud_data = {
          "setSpeed": hud.setSpeed,
          "speedVisible": hud.speedVisible,
          "lanesVisible": hud.lanesVisible,
          "leadVisible": hud.leadVisible,
          "leftLaneVisible": hud.leftLaneVisible,
          "rightLaneVisible": hud.rightLaneVisible,
          "leftLaneDepart": hud.leftLaneDepart,
          "rightLaneDepart": hud.rightLaneDepart,
          "leadDistanceBars": hud.leadDistanceBars,
          "visualAlert": int(hud.visualAlert),
          "audibleAlert": int(hud.audibleAlert),
        }
        sock.send_json(hud_data)
      time.sleep(0.05)
  finally:
    sock.close()
    context.term()


if __name__ == "__main__":
  main()
