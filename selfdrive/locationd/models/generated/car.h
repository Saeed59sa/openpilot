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
void car_err_fun(double *nom_x, double *delta_x, double *out_2227833840791821946);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8724774052849675959);
void car_H_mod_fun(double *state, double *out_3191613160569953268);
void car_f_fun(double *state, double dt, double *out_5838798502625821508);
void car_F_fun(double *state, double dt, double *out_7763539008565691217);
void car_h_25(double *state, double *unused, double *out_4764091193332901981);
void car_H_25(double *state, double *unused, double *out_8095304985754618108);
void car_h_24(double *state, double *unused, double *out_9199075439441592283);
void car_H_24(double *state, double *unused, double *out_4541944211553945379);
void car_h_30(double *state, double *unused, double *out_5919387623555977194);
void car_H_30(double *state, double *unused, double *out_7833106129447684881);
void car_h_26(double *state, double *unused, double *out_917714355895131549);
void car_H_26(double *state, double *unused, double *out_4353801666880561884);
void car_h_27(double *state, double *unused, double *out_3039788240462773654);
void car_H_27(double *state, double *unused, double *out_8438874632461441824);
void car_h_29(double *state, double *unused, double *out_7525683182492728777);
void car_H_29(double *state, double *unused, double *out_7322874785133292697);
void car_h_28(double *state, double *unused, double *out_2096278387561111219);
void car_H_28(double *state, double *unused, double *out_6041470271506728345);
void car_h_31(double *state, double *unused, double *out_511253634569188824);
void car_H_31(double *state, double *unused, double *out_3727593564647210408);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}