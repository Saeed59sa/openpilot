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
void car_err_fun(double *nom_x, double *delta_x, double *out_1200337178142554805);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_1611978325017266416);
void car_H_mod_fun(double *state, double *out_3374521719817818527);
void car_f_fun(double *state, double dt, double *out_4046569618460800488);
void car_F_fun(double *state, double dt, double *out_7113534447421158864);
void car_h_25(double *state, double *unused, double *out_5061181279587505341);
void car_H_25(double *state, double *unused, double *out_7326532735660278844);
void car_h_24(double *state, double *unused, double *out_3860013169625026907);
void car_H_24(double *state, double *unused, double *out_8947561739043773206);
void car_h_30(double *state, double *unused, double *out_2702083977233167100);
void car_H_30(double *state, double *unused, double *out_4808199777153030217);
void car_h_26(double *state, double *unused, double *out_334702074211165585);
void car_H_26(double *state, double *unused, double *out_4022006765899478243);
void car_h_27(double *state, double *unused, double *out_1337860237826343805);
void car_H_27(double *state, double *unused, double *out_6982963088953455128);
void car_h_29(double *state, double *unused, double *out_2299589290427678545);
void car_H_29(double *state, double *unused, double *out_4297968432838638033);
void car_h_28(double *state, double *unused, double *out_1080885375241510221);
void car_H_28(double *state, double *unused, double *out_9066376623801383009);
void car_h_31(double *state, double *unused, double *out_4915661455366850370);
void car_H_31(double *state, double *unused, double *out_4648214868132829719);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}