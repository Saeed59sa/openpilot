from cereal import car
from openpilot.selfdrive.car import CarSpecs, PlatformConfig, Platforms
from openpilot.selfdrive.car.fw_query_definitions import FwQueryConfig, Request, StdQueries

Ecu = car.CarParams.Ecu


class CAR(Platforms):
  MOCK = PlatformConfig(
    [],
    CarSpecs(mass=1700, wheelbase=2.7, steerRatio=13),
    {}
  )


FW_QUERY_CONFIG = FwQueryConfig(
  requests=[
    Request(
      [StdQueries.TESTER_PRESENT_REQUEST, StdQueries.UDS_VERSION_REQUEST],
      [StdQueries.TESTER_PRESENT_RESPONSE, StdQueries.UDS_VERSION_RESPONSE],
      whitelist_ecus=[Ecu.eps],
      bus=0,
    ),
  ],
)
