from enum import Enum

# Placeholder Tesla car values
class CarControllerParams:
    pass

class CAR(Enum):
    MODEL_S = "TESLA MODEL S"
    MODEL_3 = "TESLA MODEL 3"
    MODEL_X = "TESLA MODEL X"
    MODEL_Y = "TESLA MODEL Y"

FINGERPRINTS = {
    CAR.MODEL_S: [{}],
    CAR.MODEL_3: [{}],
    CAR.MODEL_X: [{}],
    CAR.MODEL_Y: [{}],
}

DBC = {
    CAR.MODEL_S: None,
    CAR.MODEL_3: None,
    CAR.MODEL_X: None,
    CAR.MODEL_Y: None,
}
