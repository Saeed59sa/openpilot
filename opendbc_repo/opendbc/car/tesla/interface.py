from opendbc.car import get_safety_config, structs
from opendbc.car.interfaces import CarInterfaceBase
from opendbc.car.tesla.carcontroller import CarController
from opendbc.car.tesla.carstate import CarState
from opendbc.car.tesla.values import TeslaSafetyFlags


class CarInterface(CarInterfaceBase):
  CarState = CarState
  CarController = CarController

  @staticmethod
  def _get_params(ret: structs.CarParams, candidate, fingerprint, car_fw, alpha_long):
    # --- brand / safety ---
    ret.brand = "tesla"
    ret.safetyConfigs = [get_safety_config(structs.CarParams.SafetyModel.tesla)]

    # --- steering dyn ---
    ret.steerLimitTimer = 0.4
    ret.steerActuatorDelay = 0.1
    ret.steerAtStandstill = True
    ret.steerControlType = structs.CarParams.SteerControlType.angle

    # --- radar ---
    ret.radarUnavailable = True

    # --- SPEED LIMITS ---
    if not hasattr(ret, "vCruiseMin"):
      ret.vCruiseMin = 30
    ret.vCruiseMax = 160  # Updated max cruise speed

    # --- longitudinal control ---
    ret.alphaLongitudinalAvailable = True
    if alpha_long:
      ret.openpilotLongitudinalControl = True
      ret.safetyConfigs[0].safetyParam |= TeslaSafetyFlags.LONG_CONTROL.value

      ret.vEgoStopping = 0.1
      ret.vEgoStarting = 0.1
      ret.stoppingDecelRate = 0.3

    return ret
