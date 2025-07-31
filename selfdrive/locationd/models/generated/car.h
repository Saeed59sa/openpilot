#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_err_fun(double *nom_x, double *delta_x, double *out_8057987757479905568);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_231936603410019898);
void car_H_mod_fun(double *state, double *out_8227657145295783489);
void car_f_fun(double *state, double dt, double *out_1609788325542728301);
void car_F_fun(double *state, double dt, double *out_5809796408412314636);
void car_h_25(double *state, double *unused, double *out_3242383925663692499);
void car_H_25(double *state, double *unused, double *out_6696106278424276450);
void car_h_24(double *state, double *unused, double *out_9105614165936397051);
void car_H_24(double *state, double *unused, double *out_8868755877429776016);
void car_h_30(double *state, double *unused, double *out_4638888725090356686);
void car_H_30(double *state, double *unused, double *out_4177773319917027823);
void car_h_26(double *state, double *unused, double *out_362784542570488959);
void car_H_26(double *state, double *unused, double *out_8009134476411218942);
void car_h_27(double *state, double *unused, double *out_1759289341997153146);
void car_H_27(double *state, double *unused, double *out_1954179248733084606);
void car_h_29(double *state, double *unused, double *out_5620972818425654798);
void car_H_29(double *state, double *unused, double *out_3667541975602635639);
void car_h_28(double *state, double *unused, double *out_3582577893888877417);
void car_H_28(double *state, double *unused, double *out_8749940992672166213);
void car_h_31(double *state, double *unused, double *out_6879833453917714849);
void car_H_31(double *state, double *unused, double *out_6665460316547316022);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}