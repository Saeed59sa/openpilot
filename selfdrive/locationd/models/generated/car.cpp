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
void err_fun(double *nom_x, double *delta_x, double *out_2227833840791821946) {
   out_2227833840791821946[0] = delta_x[0] + nom_x[0];
   out_2227833840791821946[1] = delta_x[1] + nom_x[1];
   out_2227833840791821946[2] = delta_x[2] + nom_x[2];
   out_2227833840791821946[3] = delta_x[3] + nom_x[3];
   out_2227833840791821946[4] = delta_x[4] + nom_x[4];
   out_2227833840791821946[5] = delta_x[5] + nom_x[5];
   out_2227833840791821946[6] = delta_x[6] + nom_x[6];
   out_2227833840791821946[7] = delta_x[7] + nom_x[7];
   out_2227833840791821946[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_8724774052849675959) {
   out_8724774052849675959[0] = -nom_x[0] + true_x[0];
   out_8724774052849675959[1] = -nom_x[1] + true_x[1];
   out_8724774052849675959[2] = -nom_x[2] + true_x[2];
   out_8724774052849675959[3] = -nom_x[3] + true_x[3];
   out_8724774052849675959[4] = -nom_x[4] + true_x[4];
   out_8724774052849675959[5] = -nom_x[5] + true_x[5];
   out_8724774052849675959[6] = -nom_x[6] + true_x[6];
   out_8724774052849675959[7] = -nom_x[7] + true_x[7];
   out_8724774052849675959[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_3191613160569953268) {
   out_3191613160569953268[0] = 1.0;
   out_3191613160569953268[1] = 0.0;
   out_3191613160569953268[2] = 0.0;
   out_3191613160569953268[3] = 0.0;
   out_3191613160569953268[4] = 0.0;
   out_3191613160569953268[5] = 0.0;
   out_3191613160569953268[6] = 0.0;
   out_3191613160569953268[7] = 0.0;
   out_3191613160569953268[8] = 0.0;
   out_3191613160569953268[9] = 0.0;
   out_3191613160569953268[10] = 1.0;
   out_3191613160569953268[11] = 0.0;
   out_3191613160569953268[12] = 0.0;
   out_3191613160569953268[13] = 0.0;
   out_3191613160569953268[14] = 0.0;
   out_3191613160569953268[15] = 0.0;
   out_3191613160569953268[16] = 0.0;
   out_3191613160569953268[17] = 0.0;
   out_3191613160569953268[18] = 0.0;
   out_3191613160569953268[19] = 0.0;
   out_3191613160569953268[20] = 1.0;
   out_3191613160569953268[21] = 0.0;
   out_3191613160569953268[22] = 0.0;
   out_3191613160569953268[23] = 0.0;
   out_3191613160569953268[24] = 0.0;
   out_3191613160569953268[25] = 0.0;
   out_3191613160569953268[26] = 0.0;
   out_3191613160569953268[27] = 0.0;
   out_3191613160569953268[28] = 0.0;
   out_3191613160569953268[29] = 0.0;
   out_3191613160569953268[30] = 1.0;
   out_3191613160569953268[31] = 0.0;
   out_3191613160569953268[32] = 0.0;
   out_3191613160569953268[33] = 0.0;
   out_3191613160569953268[34] = 0.0;
   out_3191613160569953268[35] = 0.0;
   out_3191613160569953268[36] = 0.0;
   out_3191613160569953268[37] = 0.0;
   out_3191613160569953268[38] = 0.0;
   out_3191613160569953268[39] = 0.0;
   out_3191613160569953268[40] = 1.0;
   out_3191613160569953268[41] = 0.0;
   out_3191613160569953268[42] = 0.0;
   out_3191613160569953268[43] = 0.0;
   out_3191613160569953268[44] = 0.0;
   out_3191613160569953268[45] = 0.0;
   out_3191613160569953268[46] = 0.0;
   out_3191613160569953268[47] = 0.0;
   out_3191613160569953268[48] = 0.0;
   out_3191613160569953268[49] = 0.0;
   out_3191613160569953268[50] = 1.0;
   out_3191613160569953268[51] = 0.0;
   out_3191613160569953268[52] = 0.0;
   out_3191613160569953268[53] = 0.0;
   out_3191613160569953268[54] = 0.0;
   out_3191613160569953268[55] = 0.0;
   out_3191613160569953268[56] = 0.0;
   out_3191613160569953268[57] = 0.0;
   out_3191613160569953268[58] = 0.0;
   out_3191613160569953268[59] = 0.0;
   out_3191613160569953268[60] = 1.0;
   out_3191613160569953268[61] = 0.0;
   out_3191613160569953268[62] = 0.0;
   out_3191613160569953268[63] = 0.0;
   out_3191613160569953268[64] = 0.0;
   out_3191613160569953268[65] = 0.0;
   out_3191613160569953268[66] = 0.0;
   out_3191613160569953268[67] = 0.0;
   out_3191613160569953268[68] = 0.0;
   out_3191613160569953268[69] = 0.0;
   out_3191613160569953268[70] = 1.0;
   out_3191613160569953268[71] = 0.0;
   out_3191613160569953268[72] = 0.0;
   out_3191613160569953268[73] = 0.0;
   out_3191613160569953268[74] = 0.0;
   out_3191613160569953268[75] = 0.0;
   out_3191613160569953268[76] = 0.0;
   out_3191613160569953268[77] = 0.0;
   out_3191613160569953268[78] = 0.0;
   out_3191613160569953268[79] = 0.0;
   out_3191613160569953268[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_5838798502625821508) {
   out_5838798502625821508[0] = state[0];
   out_5838798502625821508[1] = state[1];
   out_5838798502625821508[2] = state[2];
   out_5838798502625821508[3] = state[3];
   out_5838798502625821508[4] = state[4];
   out_5838798502625821508[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_5838798502625821508[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_5838798502625821508[7] = state[7];
   out_5838798502625821508[8] = state[8];
}
void F_fun(double *state, double dt, double *out_7763539008565691217) {
   out_7763539008565691217[0] = 1;
   out_7763539008565691217[1] = 0;
   out_7763539008565691217[2] = 0;
   out_7763539008565691217[3] = 0;
   out_7763539008565691217[4] = 0;
   out_7763539008565691217[5] = 0;
   out_7763539008565691217[6] = 0;
   out_7763539008565691217[7] = 0;
   out_7763539008565691217[8] = 0;
   out_7763539008565691217[9] = 0;
   out_7763539008565691217[10] = 1;
   out_7763539008565691217[11] = 0;
   out_7763539008565691217[12] = 0;
   out_7763539008565691217[13] = 0;
   out_7763539008565691217[14] = 0;
   out_7763539008565691217[15] = 0;
   out_7763539008565691217[16] = 0;
   out_7763539008565691217[17] = 0;
   out_7763539008565691217[18] = 0;
   out_7763539008565691217[19] = 0;
   out_7763539008565691217[20] = 1;
   out_7763539008565691217[21] = 0;
   out_7763539008565691217[22] = 0;
   out_7763539008565691217[23] = 0;
   out_7763539008565691217[24] = 0;
   out_7763539008565691217[25] = 0;
   out_7763539008565691217[26] = 0;
   out_7763539008565691217[27] = 0;
   out_7763539008565691217[28] = 0;
   out_7763539008565691217[29] = 0;
   out_7763539008565691217[30] = 1;
   out_7763539008565691217[31] = 0;
   out_7763539008565691217[32] = 0;
   out_7763539008565691217[33] = 0;
   out_7763539008565691217[34] = 0;
   out_7763539008565691217[35] = 0;
   out_7763539008565691217[36] = 0;
   out_7763539008565691217[37] = 0;
   out_7763539008565691217[38] = 0;
   out_7763539008565691217[39] = 0;
   out_7763539008565691217[40] = 1;
   out_7763539008565691217[41] = 0;
   out_7763539008565691217[42] = 0;
   out_7763539008565691217[43] = 0;
   out_7763539008565691217[44] = 0;
   out_7763539008565691217[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_7763539008565691217[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_7763539008565691217[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_7763539008565691217[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_7763539008565691217[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_7763539008565691217[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_7763539008565691217[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_7763539008565691217[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_7763539008565691217[53] = -9.8000000000000007*dt;
   out_7763539008565691217[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_7763539008565691217[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_7763539008565691217[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7763539008565691217[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7763539008565691217[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_7763539008565691217[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_7763539008565691217[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_7763539008565691217[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7763539008565691217[62] = 0;
   out_7763539008565691217[63] = 0;
   out_7763539008565691217[64] = 0;
   out_7763539008565691217[65] = 0;
   out_7763539008565691217[66] = 0;
   out_7763539008565691217[67] = 0;
   out_7763539008565691217[68] = 0;
   out_7763539008565691217[69] = 0;
   out_7763539008565691217[70] = 1;
   out_7763539008565691217[71] = 0;
   out_7763539008565691217[72] = 0;
   out_7763539008565691217[73] = 0;
   out_7763539008565691217[74] = 0;
   out_7763539008565691217[75] = 0;
   out_7763539008565691217[76] = 0;
   out_7763539008565691217[77] = 0;
   out_7763539008565691217[78] = 0;
   out_7763539008565691217[79] = 0;
   out_7763539008565691217[80] = 1;
}
void h_25(double *state, double *unused, double *out_4764091193332901981) {
   out_4764091193332901981[0] = state[6];
}
void H_25(double *state, double *unused, double *out_8095304985754618108) {
   out_8095304985754618108[0] = 0;
   out_8095304985754618108[1] = 0;
   out_8095304985754618108[2] = 0;
   out_8095304985754618108[3] = 0;
   out_8095304985754618108[4] = 0;
   out_8095304985754618108[5] = 0;
   out_8095304985754618108[6] = 1;
   out_8095304985754618108[7] = 0;
   out_8095304985754618108[8] = 0;
}
void h_24(double *state, double *unused, double *out_9199075439441592283) {
   out_9199075439441592283[0] = state[4];
   out_9199075439441592283[1] = state[5];
}
void H_24(double *state, double *unused, double *out_4541944211553945379) {
   out_4541944211553945379[0] = 0;
   out_4541944211553945379[1] = 0;
   out_4541944211553945379[2] = 0;
   out_4541944211553945379[3] = 0;
   out_4541944211553945379[4] = 1;
   out_4541944211553945379[5] = 0;
   out_4541944211553945379[6] = 0;
   out_4541944211553945379[7] = 0;
   out_4541944211553945379[8] = 0;
   out_4541944211553945379[9] = 0;
   out_4541944211553945379[10] = 0;
   out_4541944211553945379[11] = 0;
   out_4541944211553945379[12] = 0;
   out_4541944211553945379[13] = 0;
   out_4541944211553945379[14] = 1;
   out_4541944211553945379[15] = 0;
   out_4541944211553945379[16] = 0;
   out_4541944211553945379[17] = 0;
}
void h_30(double *state, double *unused, double *out_5919387623555977194) {
   out_5919387623555977194[0] = state[4];
}
void H_30(double *state, double *unused, double *out_7833106129447684881) {
   out_7833106129447684881[0] = 0;
   out_7833106129447684881[1] = 0;
   out_7833106129447684881[2] = 0;
   out_7833106129447684881[3] = 0;
   out_7833106129447684881[4] = 1;
   out_7833106129447684881[5] = 0;
   out_7833106129447684881[6] = 0;
   out_7833106129447684881[7] = 0;
   out_7833106129447684881[8] = 0;
}
void h_26(double *state, double *unused, double *out_917714355895131549) {
   out_917714355895131549[0] = state[7];
}
void H_26(double *state, double *unused, double *out_4353801666880561884) {
   out_4353801666880561884[0] = 0;
   out_4353801666880561884[1] = 0;
   out_4353801666880561884[2] = 0;
   out_4353801666880561884[3] = 0;
   out_4353801666880561884[4] = 0;
   out_4353801666880561884[5] = 0;
   out_4353801666880561884[6] = 0;
   out_4353801666880561884[7] = 1;
   out_4353801666880561884[8] = 0;
}
void h_27(double *state, double *unused, double *out_3039788240462773654) {
   out_3039788240462773654[0] = state[3];
}
void H_27(double *state, double *unused, double *out_8438874632461441824) {
   out_8438874632461441824[0] = 0;
   out_8438874632461441824[1] = 0;
   out_8438874632461441824[2] = 0;
   out_8438874632461441824[3] = 1;
   out_8438874632461441824[4] = 0;
   out_8438874632461441824[5] = 0;
   out_8438874632461441824[6] = 0;
   out_8438874632461441824[7] = 0;
   out_8438874632461441824[8] = 0;
}
void h_29(double *state, double *unused, double *out_7525683182492728777) {
   out_7525683182492728777[0] = state[1];
}
void H_29(double *state, double *unused, double *out_7322874785133292697) {
   out_7322874785133292697[0] = 0;
   out_7322874785133292697[1] = 1;
   out_7322874785133292697[2] = 0;
   out_7322874785133292697[3] = 0;
   out_7322874785133292697[4] = 0;
   out_7322874785133292697[5] = 0;
   out_7322874785133292697[6] = 0;
   out_7322874785133292697[7] = 0;
   out_7322874785133292697[8] = 0;
}
void h_28(double *state, double *unused, double *out_2096278387561111219) {
   out_2096278387561111219[0] = state[0];
}
void H_28(double *state, double *unused, double *out_6041470271506728345) {
   out_6041470271506728345[0] = 1;
   out_6041470271506728345[1] = 0;
   out_6041470271506728345[2] = 0;
   out_6041470271506728345[3] = 0;
   out_6041470271506728345[4] = 0;
   out_6041470271506728345[5] = 0;
   out_6041470271506728345[6] = 0;
   out_6041470271506728345[7] = 0;
   out_6041470271506728345[8] = 0;
}
void h_31(double *state, double *unused, double *out_511253634569188824) {
   out_511253634569188824[0] = state[8];
}
void H_31(double *state, double *unused, double *out_3727593564647210408) {
   out_3727593564647210408[0] = 0;
   out_3727593564647210408[1] = 0;
   out_3727593564647210408[2] = 0;
   out_3727593564647210408[3] = 0;
   out_3727593564647210408[4] = 0;
   out_3727593564647210408[5] = 0;
   out_3727593564647210408[6] = 0;
   out_3727593564647210408[7] = 0;
   out_3727593564647210408[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_2227833840791821946) {
  err_fun(nom_x, delta_x, out_2227833840791821946);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8724774052849675959) {
  inv_err_fun(nom_x, true_x, out_8724774052849675959);
}
void car_H_mod_fun(double *state, double *out_3191613160569953268) {
  H_mod_fun(state, out_3191613160569953268);
}
void car_f_fun(double *state, double dt, double *out_5838798502625821508) {
  f_fun(state,  dt, out_5838798502625821508);
}
void car_F_fun(double *state, double dt, double *out_7763539008565691217) {
  F_fun(state,  dt, out_7763539008565691217);
}
void car_h_25(double *state, double *unused, double *out_4764091193332901981) {
  h_25(state, unused, out_4764091193332901981);
}
void car_H_25(double *state, double *unused, double *out_8095304985754618108) {
  H_25(state, unused, out_8095304985754618108);
}
void car_h_24(double *state, double *unused, double *out_9199075439441592283) {
  h_24(state, unused, out_9199075439441592283);
}
void car_H_24(double *state, double *unused, double *out_4541944211553945379) {
  H_24(state, unused, out_4541944211553945379);
}
void car_h_30(double *state, double *unused, double *out_5919387623555977194) {
  h_30(state, unused, out_5919387623555977194);
}
void car_H_30(double *state, double *unused, double *out_7833106129447684881) {
  H_30(state, unused, out_7833106129447684881);
}
void car_h_26(double *state, double *unused, double *out_917714355895131549) {
  h_26(state, unused, out_917714355895131549);
}
void car_H_26(double *state, double *unused, double *out_4353801666880561884) {
  H_26(state, unused, out_4353801666880561884);
}
void car_h_27(double *state, double *unused, double *out_3039788240462773654) {
  h_27(state, unused, out_3039788240462773654);
}
void car_H_27(double *state, double *unused, double *out_8438874632461441824) {
  H_27(state, unused, out_8438874632461441824);
}
void car_h_29(double *state, double *unused, double *out_7525683182492728777) {
  h_29(state, unused, out_7525683182492728777);
}
void car_H_29(double *state, double *unused, double *out_7322874785133292697) {
  H_29(state, unused, out_7322874785133292697);
}
void car_h_28(double *state, double *unused, double *out_2096278387561111219) {
  h_28(state, unused, out_2096278387561111219);
}
void car_H_28(double *state, double *unused, double *out_6041470271506728345) {
  H_28(state, unused, out_6041470271506728345);
}
void car_h_31(double *state, double *unused, double *out_511253634569188824) {
  h_31(state, unused, out_511253634569188824);
}
void car_H_31(double *state, double *unused, double *out_3727593564647210408) {
  H_31(state, unused, out_3727593564647210408);
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
