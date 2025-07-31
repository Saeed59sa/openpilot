#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_5928268790671582895);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5067122510833187171);
void pose_H_mod_fun(double *state, double *out_2154722604321560680);
void pose_f_fun(double *state, double dt, double *out_5197520704410989952);
void pose_F_fun(double *state, double dt, double *out_8407003081910629671);
void pose_h_4(double *state, double *unused, double *out_1815771948845189763);
void pose_H_4(double *state, double *unused, double *out_8002804411073519401);
void pose_h_10(double *state, double *unused, double *out_3017578568057486318);
void pose_H_10(double *state, double *unused, double *out_8560344590081885983);
void pose_h_13(double *state, double *unused, double *out_2428159707165076335);
void pose_H_13(double *state, double *unused, double *out_4790530585741186600);
void pose_h_14(double *state, double *unused, double *out_8197873605832283710);
void pose_H_14(double *state, double *unused, double *out_4039563554734034872);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}