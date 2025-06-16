from openpilot.selfdrive.car.docs import CarDocs

from .values import CAR


def get_all_tesla_cars() -> list[CarDocs]:
    return [
        CarDocs("Tesla Model S", model=CAR.MODEL_S.value),
        CarDocs("Tesla Model 3", model=CAR.MODEL_3.value),
        CarDocs("Tesla Model X", model=CAR.MODEL_X.value),
        CarDocs("Tesla Model Y", model=CAR.MODEL_Y.value),
    ]
