#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_7302109445259898292);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1070101451304488163);
void pose_H_mod_fun(double *state, double *out_6857672758186073064);
void pose_f_fun(double *state, double dt, double *out_5527106423239587798);
void pose_F_fun(double *state, double dt, double *out_6344819500783556065);
void pose_h_4(double *state, double *unused, double *out_3040995270472347620);
void pose_H_4(double *state, double *unused, double *out_6103511756131360457);
void pose_h_10(double *state, double *unused, double *out_6036451979876331777);
void pose_H_10(double *state, double *unused, double *out_4735514628208408881);
void pose_h_13(double *state, double *unused, double *out_7880905284562057581);
void pose_H_13(double *state, double *unused, double *out_1507119452185340472);
void pose_h_14(double *state, double *unused, double *out_5461180862416464840);
void pose_H_14(double *state, double *unused, double *out_2140270899791875928);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}