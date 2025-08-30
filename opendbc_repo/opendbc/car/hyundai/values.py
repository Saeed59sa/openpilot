from typing import Any

HyundaiCanFDPlatformConfig = Any
HyundaiCarDocs = Any
CarParts = Any
CarHarness = Any
CarSpecs = Any

# --- Hyundai CAN-FD platforms above ---

# Genesis GV80 (1st Gen, CAN-FD)
GENESIS_GV80_1ST_GEN = HyundaiCanFDPlatformConfig(
  [HyundaiCarDocs("Genesis GV80 (1st Gen, CAN-FD)", "All", car_parts=CarParts.common([CarHarness.hyundai_a]))],
  CarSpecs(
    mass=2200,        # TODO: refine after first drive / official spec
    wheelbase=2.95,   # TODO: refine
    steerRatio=13.0,  # from liveParameters (log)
    tireStiffnessFactor=0.70,
  ),
)
