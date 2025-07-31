#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_227942031587100209);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_6904622864079995273);
void pose_H_mod_fun(double *state, double *out_3022304305314020797);
void pose_f_fun(double *state, double dt, double *out_5347359958206634853);
void pose_F_fun(double *state, double dt, double *out_2486298903049178646);
void pose_h_4(double *state, double *unused, double *out_4503598515101124135);
void pose_H_4(double *state, double *unused, double *out_6296526384104970050);
void pose_h_10(double *state, double *unused, double *out_660870634115408416);
void pose_H_10(double *state, double *unused, double *out_2423980848173730331);
void pose_h_13(double *state, double *unused, double *out_4084654337287946880);
void pose_H_13(double *state, double *unused, double *out_3084252558772637249);
void pose_h_14(double *state, double *unused, double *out_4981368864544738461);
void pose_H_14(double *state, double *unused, double *out_2333285527765485521);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}