from openpilot.selfdrive.car import CarSpecs, PlatformConfig, Platforms


class CAR(Platforms):
  MOCK = PlatformConfig(
    [],
    CarSpecs(mass=1700, wheelbase=2.7, steerRatio=13),
    {}
  )

  MOCK_CUSTOM = PlatformConfig(
    [],
    CarSpecs(mass=1500, wheelbase=2.6, steerRatio=12),
    {}
  )
