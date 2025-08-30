from typing import Any, Dict

CAR = Any
Ecu = Any
car = Any  # noqa: F841

FINGERPRINTS: Dict[Any, Any] = {
  # Genesis GV80 (CAN-FD) — fill FW strings after running the FW dump tool on the car.
  CAR.GENESIS_GV80_1ST_GEN: {
    # Expected ECUs on HKG CAN-FD:
    #  - Forward Radar: 0x7D0
    #  - Forward Camera (MFC): 0x7C4
    (Ecu.fwdRadar, 0x7d0, None): [
      # TODO: insert exact FW string(s) after dump
    ],
    (Ecu.fwdCamera, 0x7c4, None): [
      # TODO: insert exact FW string(s) after dump
    ],
  },
}
