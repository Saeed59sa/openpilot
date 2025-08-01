import time
import zmq

import cereal.messaging as messaging


def main() -> None:
    """Publish HUD control data on ZMQ for SDpilot."""
    sm = messaging.SubMaster(['carControl'])

    ctx = zmq.Context.instance()
    pub = ctx.socket(zmq.PUB)
    pub.bind("tcp://*:8008")

    try:
        while True:
            sm.update(0)
            cc = sm['carControl']
            hud = cc.hudControl
            data = {
                'speedVisible': bool(hud.speedVisible),
                'setSpeed': float(hud.setSpeed),
                'lanesVisible': bool(hud.lanesVisible),
                'leadVisible': bool(hud.leadVisible),
                'leftLaneVisible': bool(hud.leftLaneVisible),
                'rightLaneVisible': bool(hud.rightLaneVisible),
                'leftLaneDepart': bool(hud.leftLaneDepart),
                'rightLaneDepart': bool(hud.rightLaneDepart),
                'leadDistanceBars': int(hud.leadDistanceBars),
                'visualAlert': int(hud.visualAlert),
                'audibleAlert': int(hud.audibleAlert),
            }
            pub.send_json(data)
            time.sleep(0.05)
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
