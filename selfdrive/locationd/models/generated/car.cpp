#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_8057987757479905568) {
   out_8057987757479905568[0] = delta_x[0] + nom_x[0];
   out_8057987757479905568[1] = delta_x[1] + nom_x[1];
   out_8057987757479905568[2] = delta_x[2] + nom_x[2];
   out_8057987757479905568[3] = delta_x[3] + nom_x[3];
   out_8057987757479905568[4] = delta_x[4] + nom_x[4];
   out_8057987757479905568[5] = delta_x[5] + nom_x[5];
   out_8057987757479905568[6] = delta_x[6] + nom_x[6];
   out_8057987757479905568[7] = delta_x[7] + nom_x[7];
   out_8057987757479905568[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_231936603410019898) {
   out_231936603410019898[0] = -nom_x[0] + true_x[0];
   out_231936603410019898[1] = -nom_x[1] + true_x[1];
   out_231936603410019898[2] = -nom_x[2] + true_x[2];
   out_231936603410019898[3] = -nom_x[3] + true_x[3];
   out_231936603410019898[4] = -nom_x[4] + true_x[4];
   out_231936603410019898[5] = -nom_x[5] + true_x[5];
   out_231936603410019898[6] = -nom_x[6] + true_x[6];
   out_231936603410019898[7] = -nom_x[7] + true_x[7];
   out_231936603410019898[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_8227657145295783489) {
   out_8227657145295783489[0] = 1.0;
   out_8227657145295783489[1] = 0.0;
   out_8227657145295783489[2] = 0.0;
   out_8227657145295783489[3] = 0.0;
   out_8227657145295783489[4] = 0.0;
   out_8227657145295783489[5] = 0.0;
   out_8227657145295783489[6] = 0.0;
   out_8227657145295783489[7] = 0.0;
   out_8227657145295783489[8] = 0.0;
   out_8227657145295783489[9] = 0.0;
   out_8227657145295783489[10] = 1.0;
   out_8227657145295783489[11] = 0.0;
   out_8227657145295783489[12] = 0.0;
   out_8227657145295783489[13] = 0.0;
   out_8227657145295783489[14] = 0.0;
   out_8227657145295783489[15] = 0.0;
   out_8227657145295783489[16] = 0.0;
   out_8227657145295783489[17] = 0.0;
   out_8227657145295783489[18] = 0.0;
   out_8227657145295783489[19] = 0.0;
   out_8227657145295783489[20] = 1.0;
   out_8227657145295783489[21] = 0.0;
   out_8227657145295783489[22] = 0.0;
   out_8227657145295783489[23] = 0.0;
   out_8227657145295783489[24] = 0.0;
   out_8227657145295783489[25] = 0.0;
   out_8227657145295783489[26] = 0.0;
   out_8227657145295783489[27] = 0.0;
   out_8227657145295783489[28] = 0.0;
   out_8227657145295783489[29] = 0.0;
   out_8227657145295783489[30] = 1.0;
   out_8227657145295783489[31] = 0.0;
   out_8227657145295783489[32] = 0.0;
   out_8227657145295783489[33] = 0.0;
   out_8227657145295783489[34] = 0.0;
   out_8227657145295783489[35] = 0.0;
   out_8227657145295783489[36] = 0.0;
   out_8227657145295783489[37] = 0.0;
   out_8227657145295783489[38] = 0.0;
   out_8227657145295783489[39] = 0.0;
   out_8227657145295783489[40] = 1.0;
   out_8227657145295783489[41] = 0.0;
   out_8227657145295783489[42] = 0.0;
   out_8227657145295783489[43] = 0.0;
   out_8227657145295783489[44] = 0.0;
   out_8227657145295783489[45] = 0.0;
   out_8227657145295783489[46] = 0.0;
   out_8227657145295783489[47] = 0.0;
   out_8227657145295783489[48] = 0.0;
   out_8227657145295783489[49] = 0.0;
   out_8227657145295783489[50] = 1.0;
   out_8227657145295783489[51] = 0.0;
   out_8227657145295783489[52] = 0.0;
   out_8227657145295783489[53] = 0.0;
   out_8227657145295783489[54] = 0.0;
   out_8227657145295783489[55] = 0.0;
   out_8227657145295783489[56] = 0.0;
   out_8227657145295783489[57] = 0.0;
   out_8227657145295783489[58] = 0.0;
   out_8227657145295783489[59] = 0.0;
   out_8227657145295783489[60] = 1.0;
   out_8227657145295783489[61] = 0.0;
   out_8227657145295783489[62] = 0.0;
   out_8227657145295783489[63] = 0.0;
   out_8227657145295783489[64] = 0.0;
   out_8227657145295783489[65] = 0.0;
   out_8227657145295783489[66] = 0.0;
   out_8227657145295783489[67] = 0.0;
   out_8227657145295783489[68] = 0.0;
   out_8227657145295783489[69] = 0.0;
   out_8227657145295783489[70] = 1.0;
   out_8227657145295783489[71] = 0.0;
   out_8227657145295783489[72] = 0.0;
   out_8227657145295783489[73] = 0.0;
   out_8227657145295783489[74] = 0.0;
   out_8227657145295783489[75] = 0.0;
   out_8227657145295783489[76] = 0.0;
   out_8227657145295783489[77] = 0.0;
   out_8227657145295783489[78] = 0.0;
   out_8227657145295783489[79] = 0.0;
   out_8227657145295783489[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_1609788325542728301) {
   out_1609788325542728301[0] = state[0];
   out_1609788325542728301[1] = state[1];
   out_1609788325542728301[2] = state[2];
   out_1609788325542728301[3] = state[3];
   out_1609788325542728301[4] = state[4];
   out_1609788325542728301[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_1609788325542728301[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_1609788325542728301[7] = state[7];
   out_1609788325542728301[8] = state[8];
}
void F_fun(double *state, double dt, double *out_5809796408412314636) {
   out_5809796408412314636[0] = 1;
   out_5809796408412314636[1] = 0;
   out_5809796408412314636[2] = 0;
   out_5809796408412314636[3] = 0;
   out_5809796408412314636[4] = 0;
   out_5809796408412314636[5] = 0;
   out_5809796408412314636[6] = 0;
   out_5809796408412314636[7] = 0;
   out_5809796408412314636[8] = 0;
   out_5809796408412314636[9] = 0;
   out_5809796408412314636[10] = 1;
   out_5809796408412314636[11] = 0;
   out_5809796408412314636[12] = 0;
   out_5809796408412314636[13] = 0;
   out_5809796408412314636[14] = 0;
   out_5809796408412314636[15] = 0;
   out_5809796408412314636[16] = 0;
   out_5809796408412314636[17] = 0;
   out_5809796408412314636[18] = 0;
   out_5809796408412314636[19] = 0;
   out_5809796408412314636[20] = 1;
   out_5809796408412314636[21] = 0;
   out_5809796408412314636[22] = 0;
   out_5809796408412314636[23] = 0;
   out_5809796408412314636[24] = 0;
   out_5809796408412314636[25] = 0;
   out_5809796408412314636[26] = 0;
   out_5809796408412314636[27] = 0;
   out_5809796408412314636[28] = 0;
   out_5809796408412314636[29] = 0;
   out_5809796408412314636[30] = 1;
   out_5809796408412314636[31] = 0;
   out_5809796408412314636[32] = 0;
   out_5809796408412314636[33] = 0;
   out_5809796408412314636[34] = 0;
   out_5809796408412314636[35] = 0;
   out_5809796408412314636[36] = 0;
   out_5809796408412314636[37] = 0;
   out_5809796408412314636[38] = 0;
   out_5809796408412314636[39] = 0;
   out_5809796408412314636[40] = 1;
   out_5809796408412314636[41] = 0;
   out_5809796408412314636[42] = 0;
   out_5809796408412314636[43] = 0;
   out_5809796408412314636[44] = 0;
   out_5809796408412314636[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_5809796408412314636[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_5809796408412314636[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_5809796408412314636[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_5809796408412314636[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_5809796408412314636[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_5809796408412314636[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_5809796408412314636[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_5809796408412314636[53] = -9.8000000000000007*dt;
   out_5809796408412314636[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_5809796408412314636[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_5809796408412314636[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_5809796408412314636[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_5809796408412314636[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_5809796408412314636[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_5809796408412314636[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_5809796408412314636[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_5809796408412314636[62] = 0;
   out_5809796408412314636[63] = 0;
   out_5809796408412314636[64] = 0;
   out_5809796408412314636[65] = 0;
   out_5809796408412314636[66] = 0;
   out_5809796408412314636[67] = 0;
   out_5809796408412314636[68] = 0;
   out_5809796408412314636[69] = 0;
   out_5809796408412314636[70] = 1;
   out_5809796408412314636[71] = 0;
   out_5809796408412314636[72] = 0;
   out_5809796408412314636[73] = 0;
   out_5809796408412314636[74] = 0;
   out_5809796408412314636[75] = 0;
   out_5809796408412314636[76] = 0;
   out_5809796408412314636[77] = 0;
   out_5809796408412314636[78] = 0;
   out_5809796408412314636[79] = 0;
   out_5809796408412314636[80] = 1;
}
void h_25(double *state, double *unused, double *out_3242383925663692499) {
   out_3242383925663692499[0] = state[6];
}
void H_25(double *state, double *unused, double *out_6696106278424276450) {
   out_6696106278424276450[0] = 0;
   out_6696106278424276450[1] = 0;
   out_6696106278424276450[2] = 0;
   out_6696106278424276450[3] = 0;
   out_6696106278424276450[4] = 0;
   out_6696106278424276450[5] = 0;
   out_6696106278424276450[6] = 1;
   out_6696106278424276450[7] = 0;
   out_6696106278424276450[8] = 0;
}
void h_24(double *state, double *unused, double *out_9105614165936397051) {
   out_9105614165936397051[0] = state[4];
   out_9105614165936397051[1] = state[5];
}
void H_24(double *state, double *unused, double *out_8868755877429776016) {
   out_8868755877429776016[0] = 0;
   out_8868755877429776016[1] = 0;
   out_8868755877429776016[2] = 0;
   out_8868755877429776016[3] = 0;
   out_8868755877429776016[4] = 1;
   out_8868755877429776016[5] = 0;
   out_8868755877429776016[6] = 0;
   out_8868755877429776016[7] = 0;
   out_8868755877429776016[8] = 0;
   out_8868755877429776016[9] = 0;
   out_8868755877429776016[10] = 0;
   out_8868755877429776016[11] = 0;
   out_8868755877429776016[12] = 0;
   out_8868755877429776016[13] = 0;
   out_8868755877429776016[14] = 1;
   out_8868755877429776016[15] = 0;
   out_8868755877429776016[16] = 0;
   out_8868755877429776016[17] = 0;
}
void h_30(double *state, double *unused, double *out_4638888725090356686) {
   out_4638888725090356686[0] = state[4];
}
void H_30(double *state, double *unused, double *out_4177773319917027823) {
   out_4177773319917027823[0] = 0;
   out_4177773319917027823[1] = 0;
   out_4177773319917027823[2] = 0;
   out_4177773319917027823[3] = 0;
   out_4177773319917027823[4] = 1;
   out_4177773319917027823[5] = 0;
   out_4177773319917027823[6] = 0;
   out_4177773319917027823[7] = 0;
   out_4177773319917027823[8] = 0;
}
void h_26(double *state, double *unused, double *out_362784542570488959) {
   out_362784542570488959[0] = state[7];
}
void H_26(double *state, double *unused, double *out_8009134476411218942) {
   out_8009134476411218942[0] = 0;
   out_8009134476411218942[1] = 0;
   out_8009134476411218942[2] = 0;
   out_8009134476411218942[3] = 0;
   out_8009134476411218942[4] = 0;
   out_8009134476411218942[5] = 0;
   out_8009134476411218942[6] = 0;
   out_8009134476411218942[7] = 1;
   out_8009134476411218942[8] = 0;
}
void h_27(double *state, double *unused, double *out_1759289341997153146) {
   out_1759289341997153146[0] = state[3];
}
void H_27(double *state, double *unused, double *out_1954179248733084606) {
   out_1954179248733084606[0] = 0;
   out_1954179248733084606[1] = 0;
   out_1954179248733084606[2] = 0;
   out_1954179248733084606[3] = 1;
   out_1954179248733084606[4] = 0;
   out_1954179248733084606[5] = 0;
   out_1954179248733084606[6] = 0;
   out_1954179248733084606[7] = 0;
   out_1954179248733084606[8] = 0;
}
void h_29(double *state, double *unused, double *out_5620972818425654798) {
   out_5620972818425654798[0] = state[1];
}
void H_29(double *state, double *unused, double *out_3667541975602635639) {
   out_3667541975602635639[0] = 0;
   out_3667541975602635639[1] = 1;
   out_3667541975602635639[2] = 0;
   out_3667541975602635639[3] = 0;
   out_3667541975602635639[4] = 0;
   out_3667541975602635639[5] = 0;
   out_3667541975602635639[6] = 0;
   out_3667541975602635639[7] = 0;
   out_3667541975602635639[8] = 0;
}
void h_28(double *state, double *unused, double *out_3582577893888877417) {
   out_3582577893888877417[0] = state[0];
}
void H_28(double *state, double *unused, double *out_8749940992672166213) {
   out_8749940992672166213[0] = 1;
   out_8749940992672166213[1] = 0;
   out_8749940992672166213[2] = 0;
   out_8749940992672166213[3] = 0;
   out_8749940992672166213[4] = 0;
   out_8749940992672166213[5] = 0;
   out_8749940992672166213[6] = 0;
   out_8749940992672166213[7] = 0;
   out_8749940992672166213[8] = 0;
}
void h_31(double *state, double *unused, double *out_6879833453917714849) {
   out_6879833453917714849[0] = state[8];
}
void H_31(double *state, double *unused, double *out_6665460316547316022) {
   out_6665460316547316022[0] = 0;
   out_6665460316547316022[1] = 0;
   out_6665460316547316022[2] = 0;
   out_6665460316547316022[3] = 0;
   out_6665460316547316022[4] = 0;
   out_6665460316547316022[5] = 0;
   out_6665460316547316022[6] = 0;
   out_6665460316547316022[7] = 0;
   out_6665460316547316022[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_8057987757479905568) {
  err_fun(nom_x, delta_x, out_8057987757479905568);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_231936603410019898) {
  inv_err_fun(nom_x, true_x, out_231936603410019898);
}
void car_H_mod_fun(double *state, double *out_8227657145295783489) {
  H_mod_fun(state, out_8227657145295783489);
}
void car_f_fun(double *state, double dt, double *out_1609788325542728301) {
  f_fun(state,  dt, out_1609788325542728301);
}
void car_F_fun(double *state, double dt, double *out_5809796408412314636) {
  F_fun(state,  dt, out_5809796408412314636);
}
void car_h_25(double *state, double *unused, double *out_3242383925663692499) {
  h_25(state, unused, out_3242383925663692499);
}
void car_H_25(double *state, double *unused, double *out_6696106278424276450) {
  H_25(state, unused, out_6696106278424276450);
}
void car_h_24(double *state, double *unused, double *out_9105614165936397051) {
  h_24(state, unused, out_9105614165936397051);
}
void car_H_24(double *state, double *unused, double *out_8868755877429776016) {
  H_24(state, unused, out_8868755877429776016);
}
void car_h_30(double *state, double *unused, double *out_4638888725090356686) {
  h_30(state, unused, out_4638888725090356686);
}
void car_H_30(double *state, double *unused, double *out_4177773319917027823) {
  H_30(state, unused, out_4177773319917027823);
}
void car_h_26(double *state, double *unused, double *out_362784542570488959) {
  h_26(state, unused, out_362784542570488959);
}
void car_H_26(double *state, double *unused, double *out_8009134476411218942) {
  H_26(state, unused, out_8009134476411218942);
}
void car_h_27(double *state, double *unused, double *out_1759289341997153146) {
  h_27(state, unused, out_1759289341997153146);
}
void car_H_27(double *state, double *unused, double *out_1954179248733084606) {
  H_27(state, unused, out_1954179248733084606);
}
void car_h_29(double *state, double *unused, double *out_5620972818425654798) {
  h_29(state, unused, out_5620972818425654798);
}
void car_H_29(double *state, double *unused, double *out_3667541975602635639) {
  H_29(state, unused, out_3667541975602635639);
}
void car_h_28(double *state, double *unused, double *out_3582577893888877417) {
  h_28(state, unused, out_3582577893888877417);
}
void car_H_28(double *state, double *unused, double *out_8749940992672166213) {
  H_28(state, unused, out_8749940992672166213);
}
void car_h_31(double *state, double *unused, double *out_6879833453917714849) {
  h_31(state, unused, out_6879833453917714849);
}
void car_H_31(double *state, double *unused, double *out_6665460316547316022) {
  H_31(state, unused, out_6665460316547316022);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
