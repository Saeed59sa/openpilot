from __future__ import annotations

from opendbc.car.interfaces import CarInterfaceBase, dbc_dict

from .values import CAR


class CarInterface(CarInterfaceBase):
    @staticmethod
    def get_pid_accel_limits(CP, current_speed):
        return 0.0, 1.0

    @staticmethod
    def get_params(car_model: str, fingerprint=None, car_fw=None, experimental_long=False, docs=False):
        params = CarInterfaceBase.CP
        return params

    def apply(self, car_ctrl, now_nanos):
        return []
