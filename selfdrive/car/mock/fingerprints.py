from cereal import car
from openpilot.selfdrive.car.mock.values import CAR

Ecu = car.CarParams.Ecu

FW_VERSIONS = {
  CAR.MOCK_CUSTOM: {
    (Ecu.eps, 0x730, None): [
      b'TeM3_SP_XP002p2_0.0.0 (23),SPP003.6.0',
    ],
  },
}
