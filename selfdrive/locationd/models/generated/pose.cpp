#include "pose.h"

namespace {
#define DIM 18
#define EDIM 18
#define MEDIM 18
typedef void (*Hfun)(double *, double *, double *);
const static double MAHA_THRESH_4 = 7.814727903251177;
const static double MAHA_THRESH_10 = 7.814727903251177;
const static double MAHA_THRESH_13 = 7.814727903251177;
const static double MAHA_THRESH_14 = 7.814727903251177;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_7302109445259898292) {
   out_7302109445259898292[0] = delta_x[0] + nom_x[0];
   out_7302109445259898292[1] = delta_x[1] + nom_x[1];
   out_7302109445259898292[2] = delta_x[2] + nom_x[2];
   out_7302109445259898292[3] = delta_x[3] + nom_x[3];
   out_7302109445259898292[4] = delta_x[4] + nom_x[4];
   out_7302109445259898292[5] = delta_x[5] + nom_x[5];
   out_7302109445259898292[6] = delta_x[6] + nom_x[6];
   out_7302109445259898292[7] = delta_x[7] + nom_x[7];
   out_7302109445259898292[8] = delta_x[8] + nom_x[8];
   out_7302109445259898292[9] = delta_x[9] + nom_x[9];
   out_7302109445259898292[10] = delta_x[10] + nom_x[10];
   out_7302109445259898292[11] = delta_x[11] + nom_x[11];
   out_7302109445259898292[12] = delta_x[12] + nom_x[12];
   out_7302109445259898292[13] = delta_x[13] + nom_x[13];
   out_7302109445259898292[14] = delta_x[14] + nom_x[14];
   out_7302109445259898292[15] = delta_x[15] + nom_x[15];
   out_7302109445259898292[16] = delta_x[16] + nom_x[16];
   out_7302109445259898292[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_1070101451304488163) {
   out_1070101451304488163[0] = -nom_x[0] + true_x[0];
   out_1070101451304488163[1] = -nom_x[1] + true_x[1];
   out_1070101451304488163[2] = -nom_x[2] + true_x[2];
   out_1070101451304488163[3] = -nom_x[3] + true_x[3];
   out_1070101451304488163[4] = -nom_x[4] + true_x[4];
   out_1070101451304488163[5] = -nom_x[5] + true_x[5];
   out_1070101451304488163[6] = -nom_x[6] + true_x[6];
   out_1070101451304488163[7] = -nom_x[7] + true_x[7];
   out_1070101451304488163[8] = -nom_x[8] + true_x[8];
   out_1070101451304488163[9] = -nom_x[9] + true_x[9];
   out_1070101451304488163[10] = -nom_x[10] + true_x[10];
   out_1070101451304488163[11] = -nom_x[11] + true_x[11];
   out_1070101451304488163[12] = -nom_x[12] + true_x[12];
   out_1070101451304488163[13] = -nom_x[13] + true_x[13];
   out_1070101451304488163[14] = -nom_x[14] + true_x[14];
   out_1070101451304488163[15] = -nom_x[15] + true_x[15];
   out_1070101451304488163[16] = -nom_x[16] + true_x[16];
   out_1070101451304488163[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_6857672758186073064) {
   out_6857672758186073064[0] = 1.0;
   out_6857672758186073064[1] = 0.0;
   out_6857672758186073064[2] = 0.0;
   out_6857672758186073064[3] = 0.0;
   out_6857672758186073064[4] = 0.0;
   out_6857672758186073064[5] = 0.0;
   out_6857672758186073064[6] = 0.0;
   out_6857672758186073064[7] = 0.0;
   out_6857672758186073064[8] = 0.0;
   out_6857672758186073064[9] = 0.0;
   out_6857672758186073064[10] = 0.0;
   out_6857672758186073064[11] = 0.0;
   out_6857672758186073064[12] = 0.0;
   out_6857672758186073064[13] = 0.0;
   out_6857672758186073064[14] = 0.0;
   out_6857672758186073064[15] = 0.0;
   out_6857672758186073064[16] = 0.0;
   out_6857672758186073064[17] = 0.0;
   out_6857672758186073064[18] = 0.0;
   out_6857672758186073064[19] = 1.0;
   out_6857672758186073064[20] = 0.0;
   out_6857672758186073064[21] = 0.0;
   out_6857672758186073064[22] = 0.0;
   out_6857672758186073064[23] = 0.0;
   out_6857672758186073064[24] = 0.0;
   out_6857672758186073064[25] = 0.0;
   out_6857672758186073064[26] = 0.0;
   out_6857672758186073064[27] = 0.0;
   out_6857672758186073064[28] = 0.0;
   out_6857672758186073064[29] = 0.0;
   out_6857672758186073064[30] = 0.0;
   out_6857672758186073064[31] = 0.0;
   out_6857672758186073064[32] = 0.0;
   out_6857672758186073064[33] = 0.0;
   out_6857672758186073064[34] = 0.0;
   out_6857672758186073064[35] = 0.0;
   out_6857672758186073064[36] = 0.0;
   out_6857672758186073064[37] = 0.0;
   out_6857672758186073064[38] = 1.0;
   out_6857672758186073064[39] = 0.0;
   out_6857672758186073064[40] = 0.0;
   out_6857672758186073064[41] = 0.0;
   out_6857672758186073064[42] = 0.0;
   out_6857672758186073064[43] = 0.0;
   out_6857672758186073064[44] = 0.0;
   out_6857672758186073064[45] = 0.0;
   out_6857672758186073064[46] = 0.0;
   out_6857672758186073064[47] = 0.0;
   out_6857672758186073064[48] = 0.0;
   out_6857672758186073064[49] = 0.0;
   out_6857672758186073064[50] = 0.0;
   out_6857672758186073064[51] = 0.0;
   out_6857672758186073064[52] = 0.0;
   out_6857672758186073064[53] = 0.0;
   out_6857672758186073064[54] = 0.0;
   out_6857672758186073064[55] = 0.0;
   out_6857672758186073064[56] = 0.0;
   out_6857672758186073064[57] = 1.0;
   out_6857672758186073064[58] = 0.0;
   out_6857672758186073064[59] = 0.0;
   out_6857672758186073064[60] = 0.0;
   out_6857672758186073064[61] = 0.0;
   out_6857672758186073064[62] = 0.0;
   out_6857672758186073064[63] = 0.0;
   out_6857672758186073064[64] = 0.0;
   out_6857672758186073064[65] = 0.0;
   out_6857672758186073064[66] = 0.0;
   out_6857672758186073064[67] = 0.0;
   out_6857672758186073064[68] = 0.0;
   out_6857672758186073064[69] = 0.0;
   out_6857672758186073064[70] = 0.0;
   out_6857672758186073064[71] = 0.0;
   out_6857672758186073064[72] = 0.0;
   out_6857672758186073064[73] = 0.0;
   out_6857672758186073064[74] = 0.0;
   out_6857672758186073064[75] = 0.0;
   out_6857672758186073064[76] = 1.0;
   out_6857672758186073064[77] = 0.0;
   out_6857672758186073064[78] = 0.0;
   out_6857672758186073064[79] = 0.0;
   out_6857672758186073064[80] = 0.0;
   out_6857672758186073064[81] = 0.0;
   out_6857672758186073064[82] = 0.0;
   out_6857672758186073064[83] = 0.0;
   out_6857672758186073064[84] = 0.0;
   out_6857672758186073064[85] = 0.0;
   out_6857672758186073064[86] = 0.0;
   out_6857672758186073064[87] = 0.0;
   out_6857672758186073064[88] = 0.0;
   out_6857672758186073064[89] = 0.0;
   out_6857672758186073064[90] = 0.0;
   out_6857672758186073064[91] = 0.0;
   out_6857672758186073064[92] = 0.0;
   out_6857672758186073064[93] = 0.0;
   out_6857672758186073064[94] = 0.0;
   out_6857672758186073064[95] = 1.0;
   out_6857672758186073064[96] = 0.0;
   out_6857672758186073064[97] = 0.0;
   out_6857672758186073064[98] = 0.0;
   out_6857672758186073064[99] = 0.0;
   out_6857672758186073064[100] = 0.0;
   out_6857672758186073064[101] = 0.0;
   out_6857672758186073064[102] = 0.0;
   out_6857672758186073064[103] = 0.0;
   out_6857672758186073064[104] = 0.0;
   out_6857672758186073064[105] = 0.0;
   out_6857672758186073064[106] = 0.0;
   out_6857672758186073064[107] = 0.0;
   out_6857672758186073064[108] = 0.0;
   out_6857672758186073064[109] = 0.0;
   out_6857672758186073064[110] = 0.0;
   out_6857672758186073064[111] = 0.0;
   out_6857672758186073064[112] = 0.0;
   out_6857672758186073064[113] = 0.0;
   out_6857672758186073064[114] = 1.0;
   out_6857672758186073064[115] = 0.0;
   out_6857672758186073064[116] = 0.0;
   out_6857672758186073064[117] = 0.0;
   out_6857672758186073064[118] = 0.0;
   out_6857672758186073064[119] = 0.0;
   out_6857672758186073064[120] = 0.0;
   out_6857672758186073064[121] = 0.0;
   out_6857672758186073064[122] = 0.0;
   out_6857672758186073064[123] = 0.0;
   out_6857672758186073064[124] = 0.0;
   out_6857672758186073064[125] = 0.0;
   out_6857672758186073064[126] = 0.0;
   out_6857672758186073064[127] = 0.0;
   out_6857672758186073064[128] = 0.0;
   out_6857672758186073064[129] = 0.0;
   out_6857672758186073064[130] = 0.0;
   out_6857672758186073064[131] = 0.0;
   out_6857672758186073064[132] = 0.0;
   out_6857672758186073064[133] = 1.0;
   out_6857672758186073064[134] = 0.0;
   out_6857672758186073064[135] = 0.0;
   out_6857672758186073064[136] = 0.0;
   out_6857672758186073064[137] = 0.0;
   out_6857672758186073064[138] = 0.0;
   out_6857672758186073064[139] = 0.0;
   out_6857672758186073064[140] = 0.0;
   out_6857672758186073064[141] = 0.0;
   out_6857672758186073064[142] = 0.0;
   out_6857672758186073064[143] = 0.0;
   out_6857672758186073064[144] = 0.0;
   out_6857672758186073064[145] = 0.0;
   out_6857672758186073064[146] = 0.0;
   out_6857672758186073064[147] = 0.0;
   out_6857672758186073064[148] = 0.0;
   out_6857672758186073064[149] = 0.0;
   out_6857672758186073064[150] = 0.0;
   out_6857672758186073064[151] = 0.0;
   out_6857672758186073064[152] = 1.0;
   out_6857672758186073064[153] = 0.0;
   out_6857672758186073064[154] = 0.0;
   out_6857672758186073064[155] = 0.0;
   out_6857672758186073064[156] = 0.0;
   out_6857672758186073064[157] = 0.0;
   out_6857672758186073064[158] = 0.0;
   out_6857672758186073064[159] = 0.0;
   out_6857672758186073064[160] = 0.0;
   out_6857672758186073064[161] = 0.0;
   out_6857672758186073064[162] = 0.0;
   out_6857672758186073064[163] = 0.0;
   out_6857672758186073064[164] = 0.0;
   out_6857672758186073064[165] = 0.0;
   out_6857672758186073064[166] = 0.0;
   out_6857672758186073064[167] = 0.0;
   out_6857672758186073064[168] = 0.0;
   out_6857672758186073064[169] = 0.0;
   out_6857672758186073064[170] = 0.0;
   out_6857672758186073064[171] = 1.0;
   out_6857672758186073064[172] = 0.0;
   out_6857672758186073064[173] = 0.0;
   out_6857672758186073064[174] = 0.0;
   out_6857672758186073064[175] = 0.0;
   out_6857672758186073064[176] = 0.0;
   out_6857672758186073064[177] = 0.0;
   out_6857672758186073064[178] = 0.0;
   out_6857672758186073064[179] = 0.0;
   out_6857672758186073064[180] = 0.0;
   out_6857672758186073064[181] = 0.0;
   out_6857672758186073064[182] = 0.0;
   out_6857672758186073064[183] = 0.0;
   out_6857672758186073064[184] = 0.0;
   out_6857672758186073064[185] = 0.0;
   out_6857672758186073064[186] = 0.0;
   out_6857672758186073064[187] = 0.0;
   out_6857672758186073064[188] = 0.0;
   out_6857672758186073064[189] = 0.0;
   out_6857672758186073064[190] = 1.0;
   out_6857672758186073064[191] = 0.0;
   out_6857672758186073064[192] = 0.0;
   out_6857672758186073064[193] = 0.0;
   out_6857672758186073064[194] = 0.0;
   out_6857672758186073064[195] = 0.0;
   out_6857672758186073064[196] = 0.0;
   out_6857672758186073064[197] = 0.0;
   out_6857672758186073064[198] = 0.0;
   out_6857672758186073064[199] = 0.0;
   out_6857672758186073064[200] = 0.0;
   out_6857672758186073064[201] = 0.0;
   out_6857672758186073064[202] = 0.0;
   out_6857672758186073064[203] = 0.0;
   out_6857672758186073064[204] = 0.0;
   out_6857672758186073064[205] = 0.0;
   out_6857672758186073064[206] = 0.0;
   out_6857672758186073064[207] = 0.0;
   out_6857672758186073064[208] = 0.0;
   out_6857672758186073064[209] = 1.0;
   out_6857672758186073064[210] = 0.0;
   out_6857672758186073064[211] = 0.0;
   out_6857672758186073064[212] = 0.0;
   out_6857672758186073064[213] = 0.0;
   out_6857672758186073064[214] = 0.0;
   out_6857672758186073064[215] = 0.0;
   out_6857672758186073064[216] = 0.0;
   out_6857672758186073064[217] = 0.0;
   out_6857672758186073064[218] = 0.0;
   out_6857672758186073064[219] = 0.0;
   out_6857672758186073064[220] = 0.0;
   out_6857672758186073064[221] = 0.0;
   out_6857672758186073064[222] = 0.0;
   out_6857672758186073064[223] = 0.0;
   out_6857672758186073064[224] = 0.0;
   out_6857672758186073064[225] = 0.0;
   out_6857672758186073064[226] = 0.0;
   out_6857672758186073064[227] = 0.0;
   out_6857672758186073064[228] = 1.0;
   out_6857672758186073064[229] = 0.0;
   out_6857672758186073064[230] = 0.0;
   out_6857672758186073064[231] = 0.0;
   out_6857672758186073064[232] = 0.0;
   out_6857672758186073064[233] = 0.0;
   out_6857672758186073064[234] = 0.0;
   out_6857672758186073064[235] = 0.0;
   out_6857672758186073064[236] = 0.0;
   out_6857672758186073064[237] = 0.0;
   out_6857672758186073064[238] = 0.0;
   out_6857672758186073064[239] = 0.0;
   out_6857672758186073064[240] = 0.0;
   out_6857672758186073064[241] = 0.0;
   out_6857672758186073064[242] = 0.0;
   out_6857672758186073064[243] = 0.0;
   out_6857672758186073064[244] = 0.0;
   out_6857672758186073064[245] = 0.0;
   out_6857672758186073064[246] = 0.0;
   out_6857672758186073064[247] = 1.0;
   out_6857672758186073064[248] = 0.0;
   out_6857672758186073064[249] = 0.0;
   out_6857672758186073064[250] = 0.0;
   out_6857672758186073064[251] = 0.0;
   out_6857672758186073064[252] = 0.0;
   out_6857672758186073064[253] = 0.0;
   out_6857672758186073064[254] = 0.0;
   out_6857672758186073064[255] = 0.0;
   out_6857672758186073064[256] = 0.0;
   out_6857672758186073064[257] = 0.0;
   out_6857672758186073064[258] = 0.0;
   out_6857672758186073064[259] = 0.0;
   out_6857672758186073064[260] = 0.0;
   out_6857672758186073064[261] = 0.0;
   out_6857672758186073064[262] = 0.0;
   out_6857672758186073064[263] = 0.0;
   out_6857672758186073064[264] = 0.0;
   out_6857672758186073064[265] = 0.0;
   out_6857672758186073064[266] = 1.0;
   out_6857672758186073064[267] = 0.0;
   out_6857672758186073064[268] = 0.0;
   out_6857672758186073064[269] = 0.0;
   out_6857672758186073064[270] = 0.0;
   out_6857672758186073064[271] = 0.0;
   out_6857672758186073064[272] = 0.0;
   out_6857672758186073064[273] = 0.0;
   out_6857672758186073064[274] = 0.0;
   out_6857672758186073064[275] = 0.0;
   out_6857672758186073064[276] = 0.0;
   out_6857672758186073064[277] = 0.0;
   out_6857672758186073064[278] = 0.0;
   out_6857672758186073064[279] = 0.0;
   out_6857672758186073064[280] = 0.0;
   out_6857672758186073064[281] = 0.0;
   out_6857672758186073064[282] = 0.0;
   out_6857672758186073064[283] = 0.0;
   out_6857672758186073064[284] = 0.0;
   out_6857672758186073064[285] = 1.0;
   out_6857672758186073064[286] = 0.0;
   out_6857672758186073064[287] = 0.0;
   out_6857672758186073064[288] = 0.0;
   out_6857672758186073064[289] = 0.0;
   out_6857672758186073064[290] = 0.0;
   out_6857672758186073064[291] = 0.0;
   out_6857672758186073064[292] = 0.0;
   out_6857672758186073064[293] = 0.0;
   out_6857672758186073064[294] = 0.0;
   out_6857672758186073064[295] = 0.0;
   out_6857672758186073064[296] = 0.0;
   out_6857672758186073064[297] = 0.0;
   out_6857672758186073064[298] = 0.0;
   out_6857672758186073064[299] = 0.0;
   out_6857672758186073064[300] = 0.0;
   out_6857672758186073064[301] = 0.0;
   out_6857672758186073064[302] = 0.0;
   out_6857672758186073064[303] = 0.0;
   out_6857672758186073064[304] = 1.0;
   out_6857672758186073064[305] = 0.0;
   out_6857672758186073064[306] = 0.0;
   out_6857672758186073064[307] = 0.0;
   out_6857672758186073064[308] = 0.0;
   out_6857672758186073064[309] = 0.0;
   out_6857672758186073064[310] = 0.0;
   out_6857672758186073064[311] = 0.0;
   out_6857672758186073064[312] = 0.0;
   out_6857672758186073064[313] = 0.0;
   out_6857672758186073064[314] = 0.0;
   out_6857672758186073064[315] = 0.0;
   out_6857672758186073064[316] = 0.0;
   out_6857672758186073064[317] = 0.0;
   out_6857672758186073064[318] = 0.0;
   out_6857672758186073064[319] = 0.0;
   out_6857672758186073064[320] = 0.0;
   out_6857672758186073064[321] = 0.0;
   out_6857672758186073064[322] = 0.0;
   out_6857672758186073064[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_5527106423239587798) {
   out_5527106423239587798[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_5527106423239587798[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_5527106423239587798[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_5527106423239587798[3] = dt*state[12] + state[3];
   out_5527106423239587798[4] = dt*state[13] + state[4];
   out_5527106423239587798[5] = dt*state[14] + state[5];
   out_5527106423239587798[6] = state[6];
   out_5527106423239587798[7] = state[7];
   out_5527106423239587798[8] = state[8];
   out_5527106423239587798[9] = state[9];
   out_5527106423239587798[10] = state[10];
   out_5527106423239587798[11] = state[11];
   out_5527106423239587798[12] = state[12];
   out_5527106423239587798[13] = state[13];
   out_5527106423239587798[14] = state[14];
   out_5527106423239587798[15] = state[15];
   out_5527106423239587798[16] = state[16];
   out_5527106423239587798[17] = state[17];
}
void F_fun(double *state, double dt, double *out_6344819500783556065) {
   out_6344819500783556065[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6344819500783556065[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6344819500783556065[2] = 0;
   out_6344819500783556065[3] = 0;
   out_6344819500783556065[4] = 0;
   out_6344819500783556065[5] = 0;
   out_6344819500783556065[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6344819500783556065[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6344819500783556065[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6344819500783556065[9] = 0;
   out_6344819500783556065[10] = 0;
   out_6344819500783556065[11] = 0;
   out_6344819500783556065[12] = 0;
   out_6344819500783556065[13] = 0;
   out_6344819500783556065[14] = 0;
   out_6344819500783556065[15] = 0;
   out_6344819500783556065[16] = 0;
   out_6344819500783556065[17] = 0;
   out_6344819500783556065[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6344819500783556065[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6344819500783556065[20] = 0;
   out_6344819500783556065[21] = 0;
   out_6344819500783556065[22] = 0;
   out_6344819500783556065[23] = 0;
   out_6344819500783556065[24] = 0;
   out_6344819500783556065[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6344819500783556065[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6344819500783556065[27] = 0;
   out_6344819500783556065[28] = 0;
   out_6344819500783556065[29] = 0;
   out_6344819500783556065[30] = 0;
   out_6344819500783556065[31] = 0;
   out_6344819500783556065[32] = 0;
   out_6344819500783556065[33] = 0;
   out_6344819500783556065[34] = 0;
   out_6344819500783556065[35] = 0;
   out_6344819500783556065[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6344819500783556065[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6344819500783556065[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6344819500783556065[39] = 0;
   out_6344819500783556065[40] = 0;
   out_6344819500783556065[41] = 0;
   out_6344819500783556065[42] = 0;
   out_6344819500783556065[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6344819500783556065[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6344819500783556065[45] = 0;
   out_6344819500783556065[46] = 0;
   out_6344819500783556065[47] = 0;
   out_6344819500783556065[48] = 0;
   out_6344819500783556065[49] = 0;
   out_6344819500783556065[50] = 0;
   out_6344819500783556065[51] = 0;
   out_6344819500783556065[52] = 0;
   out_6344819500783556065[53] = 0;
   out_6344819500783556065[54] = 0;
   out_6344819500783556065[55] = 0;
   out_6344819500783556065[56] = 0;
   out_6344819500783556065[57] = 1;
   out_6344819500783556065[58] = 0;
   out_6344819500783556065[59] = 0;
   out_6344819500783556065[60] = 0;
   out_6344819500783556065[61] = 0;
   out_6344819500783556065[62] = 0;
   out_6344819500783556065[63] = 0;
   out_6344819500783556065[64] = 0;
   out_6344819500783556065[65] = 0;
   out_6344819500783556065[66] = dt;
   out_6344819500783556065[67] = 0;
   out_6344819500783556065[68] = 0;
   out_6344819500783556065[69] = 0;
   out_6344819500783556065[70] = 0;
   out_6344819500783556065[71] = 0;
   out_6344819500783556065[72] = 0;
   out_6344819500783556065[73] = 0;
   out_6344819500783556065[74] = 0;
   out_6344819500783556065[75] = 0;
   out_6344819500783556065[76] = 1;
   out_6344819500783556065[77] = 0;
   out_6344819500783556065[78] = 0;
   out_6344819500783556065[79] = 0;
   out_6344819500783556065[80] = 0;
   out_6344819500783556065[81] = 0;
   out_6344819500783556065[82] = 0;
   out_6344819500783556065[83] = 0;
   out_6344819500783556065[84] = 0;
   out_6344819500783556065[85] = dt;
   out_6344819500783556065[86] = 0;
   out_6344819500783556065[87] = 0;
   out_6344819500783556065[88] = 0;
   out_6344819500783556065[89] = 0;
   out_6344819500783556065[90] = 0;
   out_6344819500783556065[91] = 0;
   out_6344819500783556065[92] = 0;
   out_6344819500783556065[93] = 0;
   out_6344819500783556065[94] = 0;
   out_6344819500783556065[95] = 1;
   out_6344819500783556065[96] = 0;
   out_6344819500783556065[97] = 0;
   out_6344819500783556065[98] = 0;
   out_6344819500783556065[99] = 0;
   out_6344819500783556065[100] = 0;
   out_6344819500783556065[101] = 0;
   out_6344819500783556065[102] = 0;
   out_6344819500783556065[103] = 0;
   out_6344819500783556065[104] = dt;
   out_6344819500783556065[105] = 0;
   out_6344819500783556065[106] = 0;
   out_6344819500783556065[107] = 0;
   out_6344819500783556065[108] = 0;
   out_6344819500783556065[109] = 0;
   out_6344819500783556065[110] = 0;
   out_6344819500783556065[111] = 0;
   out_6344819500783556065[112] = 0;
   out_6344819500783556065[113] = 0;
   out_6344819500783556065[114] = 1;
   out_6344819500783556065[115] = 0;
   out_6344819500783556065[116] = 0;
   out_6344819500783556065[117] = 0;
   out_6344819500783556065[118] = 0;
   out_6344819500783556065[119] = 0;
   out_6344819500783556065[120] = 0;
   out_6344819500783556065[121] = 0;
   out_6344819500783556065[122] = 0;
   out_6344819500783556065[123] = 0;
   out_6344819500783556065[124] = 0;
   out_6344819500783556065[125] = 0;
   out_6344819500783556065[126] = 0;
   out_6344819500783556065[127] = 0;
   out_6344819500783556065[128] = 0;
   out_6344819500783556065[129] = 0;
   out_6344819500783556065[130] = 0;
   out_6344819500783556065[131] = 0;
   out_6344819500783556065[132] = 0;
   out_6344819500783556065[133] = 1;
   out_6344819500783556065[134] = 0;
   out_6344819500783556065[135] = 0;
   out_6344819500783556065[136] = 0;
   out_6344819500783556065[137] = 0;
   out_6344819500783556065[138] = 0;
   out_6344819500783556065[139] = 0;
   out_6344819500783556065[140] = 0;
   out_6344819500783556065[141] = 0;
   out_6344819500783556065[142] = 0;
   out_6344819500783556065[143] = 0;
   out_6344819500783556065[144] = 0;
   out_6344819500783556065[145] = 0;
   out_6344819500783556065[146] = 0;
   out_6344819500783556065[147] = 0;
   out_6344819500783556065[148] = 0;
   out_6344819500783556065[149] = 0;
   out_6344819500783556065[150] = 0;
   out_6344819500783556065[151] = 0;
   out_6344819500783556065[152] = 1;
   out_6344819500783556065[153] = 0;
   out_6344819500783556065[154] = 0;
   out_6344819500783556065[155] = 0;
   out_6344819500783556065[156] = 0;
   out_6344819500783556065[157] = 0;
   out_6344819500783556065[158] = 0;
   out_6344819500783556065[159] = 0;
   out_6344819500783556065[160] = 0;
   out_6344819500783556065[161] = 0;
   out_6344819500783556065[162] = 0;
   out_6344819500783556065[163] = 0;
   out_6344819500783556065[164] = 0;
   out_6344819500783556065[165] = 0;
   out_6344819500783556065[166] = 0;
   out_6344819500783556065[167] = 0;
   out_6344819500783556065[168] = 0;
   out_6344819500783556065[169] = 0;
   out_6344819500783556065[170] = 0;
   out_6344819500783556065[171] = 1;
   out_6344819500783556065[172] = 0;
   out_6344819500783556065[173] = 0;
   out_6344819500783556065[174] = 0;
   out_6344819500783556065[175] = 0;
   out_6344819500783556065[176] = 0;
   out_6344819500783556065[177] = 0;
   out_6344819500783556065[178] = 0;
   out_6344819500783556065[179] = 0;
   out_6344819500783556065[180] = 0;
   out_6344819500783556065[181] = 0;
   out_6344819500783556065[182] = 0;
   out_6344819500783556065[183] = 0;
   out_6344819500783556065[184] = 0;
   out_6344819500783556065[185] = 0;
   out_6344819500783556065[186] = 0;
   out_6344819500783556065[187] = 0;
   out_6344819500783556065[188] = 0;
   out_6344819500783556065[189] = 0;
   out_6344819500783556065[190] = 1;
   out_6344819500783556065[191] = 0;
   out_6344819500783556065[192] = 0;
   out_6344819500783556065[193] = 0;
   out_6344819500783556065[194] = 0;
   out_6344819500783556065[195] = 0;
   out_6344819500783556065[196] = 0;
   out_6344819500783556065[197] = 0;
   out_6344819500783556065[198] = 0;
   out_6344819500783556065[199] = 0;
   out_6344819500783556065[200] = 0;
   out_6344819500783556065[201] = 0;
   out_6344819500783556065[202] = 0;
   out_6344819500783556065[203] = 0;
   out_6344819500783556065[204] = 0;
   out_6344819500783556065[205] = 0;
   out_6344819500783556065[206] = 0;
   out_6344819500783556065[207] = 0;
   out_6344819500783556065[208] = 0;
   out_6344819500783556065[209] = 1;
   out_6344819500783556065[210] = 0;
   out_6344819500783556065[211] = 0;
   out_6344819500783556065[212] = 0;
   out_6344819500783556065[213] = 0;
   out_6344819500783556065[214] = 0;
   out_6344819500783556065[215] = 0;
   out_6344819500783556065[216] = 0;
   out_6344819500783556065[217] = 0;
   out_6344819500783556065[218] = 0;
   out_6344819500783556065[219] = 0;
   out_6344819500783556065[220] = 0;
   out_6344819500783556065[221] = 0;
   out_6344819500783556065[222] = 0;
   out_6344819500783556065[223] = 0;
   out_6344819500783556065[224] = 0;
   out_6344819500783556065[225] = 0;
   out_6344819500783556065[226] = 0;
   out_6344819500783556065[227] = 0;
   out_6344819500783556065[228] = 1;
   out_6344819500783556065[229] = 0;
   out_6344819500783556065[230] = 0;
   out_6344819500783556065[231] = 0;
   out_6344819500783556065[232] = 0;
   out_6344819500783556065[233] = 0;
   out_6344819500783556065[234] = 0;
   out_6344819500783556065[235] = 0;
   out_6344819500783556065[236] = 0;
   out_6344819500783556065[237] = 0;
   out_6344819500783556065[238] = 0;
   out_6344819500783556065[239] = 0;
   out_6344819500783556065[240] = 0;
   out_6344819500783556065[241] = 0;
   out_6344819500783556065[242] = 0;
   out_6344819500783556065[243] = 0;
   out_6344819500783556065[244] = 0;
   out_6344819500783556065[245] = 0;
   out_6344819500783556065[246] = 0;
   out_6344819500783556065[247] = 1;
   out_6344819500783556065[248] = 0;
   out_6344819500783556065[249] = 0;
   out_6344819500783556065[250] = 0;
   out_6344819500783556065[251] = 0;
   out_6344819500783556065[252] = 0;
   out_6344819500783556065[253] = 0;
   out_6344819500783556065[254] = 0;
   out_6344819500783556065[255] = 0;
   out_6344819500783556065[256] = 0;
   out_6344819500783556065[257] = 0;
   out_6344819500783556065[258] = 0;
   out_6344819500783556065[259] = 0;
   out_6344819500783556065[260] = 0;
   out_6344819500783556065[261] = 0;
   out_6344819500783556065[262] = 0;
   out_6344819500783556065[263] = 0;
   out_6344819500783556065[264] = 0;
   out_6344819500783556065[265] = 0;
   out_6344819500783556065[266] = 1;
   out_6344819500783556065[267] = 0;
   out_6344819500783556065[268] = 0;
   out_6344819500783556065[269] = 0;
   out_6344819500783556065[270] = 0;
   out_6344819500783556065[271] = 0;
   out_6344819500783556065[272] = 0;
   out_6344819500783556065[273] = 0;
   out_6344819500783556065[274] = 0;
   out_6344819500783556065[275] = 0;
   out_6344819500783556065[276] = 0;
   out_6344819500783556065[277] = 0;
   out_6344819500783556065[278] = 0;
   out_6344819500783556065[279] = 0;
   out_6344819500783556065[280] = 0;
   out_6344819500783556065[281] = 0;
   out_6344819500783556065[282] = 0;
   out_6344819500783556065[283] = 0;
   out_6344819500783556065[284] = 0;
   out_6344819500783556065[285] = 1;
   out_6344819500783556065[286] = 0;
   out_6344819500783556065[287] = 0;
   out_6344819500783556065[288] = 0;
   out_6344819500783556065[289] = 0;
   out_6344819500783556065[290] = 0;
   out_6344819500783556065[291] = 0;
   out_6344819500783556065[292] = 0;
   out_6344819500783556065[293] = 0;
   out_6344819500783556065[294] = 0;
   out_6344819500783556065[295] = 0;
   out_6344819500783556065[296] = 0;
   out_6344819500783556065[297] = 0;
   out_6344819500783556065[298] = 0;
   out_6344819500783556065[299] = 0;
   out_6344819500783556065[300] = 0;
   out_6344819500783556065[301] = 0;
   out_6344819500783556065[302] = 0;
   out_6344819500783556065[303] = 0;
   out_6344819500783556065[304] = 1;
   out_6344819500783556065[305] = 0;
   out_6344819500783556065[306] = 0;
   out_6344819500783556065[307] = 0;
   out_6344819500783556065[308] = 0;
   out_6344819500783556065[309] = 0;
   out_6344819500783556065[310] = 0;
   out_6344819500783556065[311] = 0;
   out_6344819500783556065[312] = 0;
   out_6344819500783556065[313] = 0;
   out_6344819500783556065[314] = 0;
   out_6344819500783556065[315] = 0;
   out_6344819500783556065[316] = 0;
   out_6344819500783556065[317] = 0;
   out_6344819500783556065[318] = 0;
   out_6344819500783556065[319] = 0;
   out_6344819500783556065[320] = 0;
   out_6344819500783556065[321] = 0;
   out_6344819500783556065[322] = 0;
   out_6344819500783556065[323] = 1;
}
void h_4(double *state, double *unused, double *out_3040995270472347620) {
   out_3040995270472347620[0] = state[6] + state[9];
   out_3040995270472347620[1] = state[7] + state[10];
   out_3040995270472347620[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_6103511756131360457) {
   out_6103511756131360457[0] = 0;
   out_6103511756131360457[1] = 0;
   out_6103511756131360457[2] = 0;
   out_6103511756131360457[3] = 0;
   out_6103511756131360457[4] = 0;
   out_6103511756131360457[5] = 0;
   out_6103511756131360457[6] = 1;
   out_6103511756131360457[7] = 0;
   out_6103511756131360457[8] = 0;
   out_6103511756131360457[9] = 1;
   out_6103511756131360457[10] = 0;
   out_6103511756131360457[11] = 0;
   out_6103511756131360457[12] = 0;
   out_6103511756131360457[13] = 0;
   out_6103511756131360457[14] = 0;
   out_6103511756131360457[15] = 0;
   out_6103511756131360457[16] = 0;
   out_6103511756131360457[17] = 0;
   out_6103511756131360457[18] = 0;
   out_6103511756131360457[19] = 0;
   out_6103511756131360457[20] = 0;
   out_6103511756131360457[21] = 0;
   out_6103511756131360457[22] = 0;
   out_6103511756131360457[23] = 0;
   out_6103511756131360457[24] = 0;
   out_6103511756131360457[25] = 1;
   out_6103511756131360457[26] = 0;
   out_6103511756131360457[27] = 0;
   out_6103511756131360457[28] = 1;
   out_6103511756131360457[29] = 0;
   out_6103511756131360457[30] = 0;
   out_6103511756131360457[31] = 0;
   out_6103511756131360457[32] = 0;
   out_6103511756131360457[33] = 0;
   out_6103511756131360457[34] = 0;
   out_6103511756131360457[35] = 0;
   out_6103511756131360457[36] = 0;
   out_6103511756131360457[37] = 0;
   out_6103511756131360457[38] = 0;
   out_6103511756131360457[39] = 0;
   out_6103511756131360457[40] = 0;
   out_6103511756131360457[41] = 0;
   out_6103511756131360457[42] = 0;
   out_6103511756131360457[43] = 0;
   out_6103511756131360457[44] = 1;
   out_6103511756131360457[45] = 0;
   out_6103511756131360457[46] = 0;
   out_6103511756131360457[47] = 1;
   out_6103511756131360457[48] = 0;
   out_6103511756131360457[49] = 0;
   out_6103511756131360457[50] = 0;
   out_6103511756131360457[51] = 0;
   out_6103511756131360457[52] = 0;
   out_6103511756131360457[53] = 0;
}
void h_10(double *state, double *unused, double *out_6036451979876331777) {
   out_6036451979876331777[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_6036451979876331777[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_6036451979876331777[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_4735514628208408881) {
   out_4735514628208408881[0] = 0;
   out_4735514628208408881[1] = 9.8100000000000005*cos(state[1]);
   out_4735514628208408881[2] = 0;
   out_4735514628208408881[3] = 0;
   out_4735514628208408881[4] = -state[8];
   out_4735514628208408881[5] = state[7];
   out_4735514628208408881[6] = 0;
   out_4735514628208408881[7] = state[5];
   out_4735514628208408881[8] = -state[4];
   out_4735514628208408881[9] = 0;
   out_4735514628208408881[10] = 0;
   out_4735514628208408881[11] = 0;
   out_4735514628208408881[12] = 1;
   out_4735514628208408881[13] = 0;
   out_4735514628208408881[14] = 0;
   out_4735514628208408881[15] = 1;
   out_4735514628208408881[16] = 0;
   out_4735514628208408881[17] = 0;
   out_4735514628208408881[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_4735514628208408881[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_4735514628208408881[20] = 0;
   out_4735514628208408881[21] = state[8];
   out_4735514628208408881[22] = 0;
   out_4735514628208408881[23] = -state[6];
   out_4735514628208408881[24] = -state[5];
   out_4735514628208408881[25] = 0;
   out_4735514628208408881[26] = state[3];
   out_4735514628208408881[27] = 0;
   out_4735514628208408881[28] = 0;
   out_4735514628208408881[29] = 0;
   out_4735514628208408881[30] = 0;
   out_4735514628208408881[31] = 1;
   out_4735514628208408881[32] = 0;
   out_4735514628208408881[33] = 0;
   out_4735514628208408881[34] = 1;
   out_4735514628208408881[35] = 0;
   out_4735514628208408881[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_4735514628208408881[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_4735514628208408881[38] = 0;
   out_4735514628208408881[39] = -state[7];
   out_4735514628208408881[40] = state[6];
   out_4735514628208408881[41] = 0;
   out_4735514628208408881[42] = state[4];
   out_4735514628208408881[43] = -state[3];
   out_4735514628208408881[44] = 0;
   out_4735514628208408881[45] = 0;
   out_4735514628208408881[46] = 0;
   out_4735514628208408881[47] = 0;
   out_4735514628208408881[48] = 0;
   out_4735514628208408881[49] = 0;
   out_4735514628208408881[50] = 1;
   out_4735514628208408881[51] = 0;
   out_4735514628208408881[52] = 0;
   out_4735514628208408881[53] = 1;
}
void h_13(double *state, double *unused, double *out_7880905284562057581) {
   out_7880905284562057581[0] = state[3];
   out_7880905284562057581[1] = state[4];
   out_7880905284562057581[2] = state[5];
}
void H_13(double *state, double *unused, double *out_1507119452185340472) {
   out_1507119452185340472[0] = 0;
   out_1507119452185340472[1] = 0;
   out_1507119452185340472[2] = 0;
   out_1507119452185340472[3] = 1;
   out_1507119452185340472[4] = 0;
   out_1507119452185340472[5] = 0;
   out_1507119452185340472[6] = 0;
   out_1507119452185340472[7] = 0;
   out_1507119452185340472[8] = 0;
   out_1507119452185340472[9] = 0;
   out_1507119452185340472[10] = 0;
   out_1507119452185340472[11] = 0;
   out_1507119452185340472[12] = 0;
   out_1507119452185340472[13] = 0;
   out_1507119452185340472[14] = 0;
   out_1507119452185340472[15] = 0;
   out_1507119452185340472[16] = 0;
   out_1507119452185340472[17] = 0;
   out_1507119452185340472[18] = 0;
   out_1507119452185340472[19] = 0;
   out_1507119452185340472[20] = 0;
   out_1507119452185340472[21] = 0;
   out_1507119452185340472[22] = 1;
   out_1507119452185340472[23] = 0;
   out_1507119452185340472[24] = 0;
   out_1507119452185340472[25] = 0;
   out_1507119452185340472[26] = 0;
   out_1507119452185340472[27] = 0;
   out_1507119452185340472[28] = 0;
   out_1507119452185340472[29] = 0;
   out_1507119452185340472[30] = 0;
   out_1507119452185340472[31] = 0;
   out_1507119452185340472[32] = 0;
   out_1507119452185340472[33] = 0;
   out_1507119452185340472[34] = 0;
   out_1507119452185340472[35] = 0;
   out_1507119452185340472[36] = 0;
   out_1507119452185340472[37] = 0;
   out_1507119452185340472[38] = 0;
   out_1507119452185340472[39] = 0;
   out_1507119452185340472[40] = 0;
   out_1507119452185340472[41] = 1;
   out_1507119452185340472[42] = 0;
   out_1507119452185340472[43] = 0;
   out_1507119452185340472[44] = 0;
   out_1507119452185340472[45] = 0;
   out_1507119452185340472[46] = 0;
   out_1507119452185340472[47] = 0;
   out_1507119452185340472[48] = 0;
   out_1507119452185340472[49] = 0;
   out_1507119452185340472[50] = 0;
   out_1507119452185340472[51] = 0;
   out_1507119452185340472[52] = 0;
   out_1507119452185340472[53] = 0;
}
void h_14(double *state, double *unused, double *out_5461180862416464840) {
   out_5461180862416464840[0] = state[6];
   out_5461180862416464840[1] = state[7];
   out_5461180862416464840[2] = state[8];
}
void H_14(double *state, double *unused, double *out_2140270899791875928) {
   out_2140270899791875928[0] = 0;
   out_2140270899791875928[1] = 0;
   out_2140270899791875928[2] = 0;
   out_2140270899791875928[3] = 0;
   out_2140270899791875928[4] = 0;
   out_2140270899791875928[5] = 0;
   out_2140270899791875928[6] = 1;
   out_2140270899791875928[7] = 0;
   out_2140270899791875928[8] = 0;
   out_2140270899791875928[9] = 0;
   out_2140270899791875928[10] = 0;
   out_2140270899791875928[11] = 0;
   out_2140270899791875928[12] = 0;
   out_2140270899791875928[13] = 0;
   out_2140270899791875928[14] = 0;
   out_2140270899791875928[15] = 0;
   out_2140270899791875928[16] = 0;
   out_2140270899791875928[17] = 0;
   out_2140270899791875928[18] = 0;
   out_2140270899791875928[19] = 0;
   out_2140270899791875928[20] = 0;
   out_2140270899791875928[21] = 0;
   out_2140270899791875928[22] = 0;
   out_2140270899791875928[23] = 0;
   out_2140270899791875928[24] = 0;
   out_2140270899791875928[25] = 1;
   out_2140270899791875928[26] = 0;
   out_2140270899791875928[27] = 0;
   out_2140270899791875928[28] = 0;
   out_2140270899791875928[29] = 0;
   out_2140270899791875928[30] = 0;
   out_2140270899791875928[31] = 0;
   out_2140270899791875928[32] = 0;
   out_2140270899791875928[33] = 0;
   out_2140270899791875928[34] = 0;
   out_2140270899791875928[35] = 0;
   out_2140270899791875928[36] = 0;
   out_2140270899791875928[37] = 0;
   out_2140270899791875928[38] = 0;
   out_2140270899791875928[39] = 0;
   out_2140270899791875928[40] = 0;
   out_2140270899791875928[41] = 0;
   out_2140270899791875928[42] = 0;
   out_2140270899791875928[43] = 0;
   out_2140270899791875928[44] = 1;
   out_2140270899791875928[45] = 0;
   out_2140270899791875928[46] = 0;
   out_2140270899791875928[47] = 0;
   out_2140270899791875928[48] = 0;
   out_2140270899791875928[49] = 0;
   out_2140270899791875928[50] = 0;
   out_2140270899791875928[51] = 0;
   out_2140270899791875928[52] = 0;
   out_2140270899791875928[53] = 0;
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

void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_4, H_4, NULL, in_z, in_R, in_ea, MAHA_THRESH_4);
}
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_10, H_10, NULL, in_z, in_R, in_ea, MAHA_THRESH_10);
}
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_13, H_13, NULL, in_z, in_R, in_ea, MAHA_THRESH_13);
}
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_14, H_14, NULL, in_z, in_R, in_ea, MAHA_THRESH_14);
}
void pose_err_fun(double *nom_x, double *delta_x, double *out_7302109445259898292) {
  err_fun(nom_x, delta_x, out_7302109445259898292);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_1070101451304488163) {
  inv_err_fun(nom_x, true_x, out_1070101451304488163);
}
void pose_H_mod_fun(double *state, double *out_6857672758186073064) {
  H_mod_fun(state, out_6857672758186073064);
}
void pose_f_fun(double *state, double dt, double *out_5527106423239587798) {
  f_fun(state,  dt, out_5527106423239587798);
}
void pose_F_fun(double *state, double dt, double *out_6344819500783556065) {
  F_fun(state,  dt, out_6344819500783556065);
}
void pose_h_4(double *state, double *unused, double *out_3040995270472347620) {
  h_4(state, unused, out_3040995270472347620);
}
void pose_H_4(double *state, double *unused, double *out_6103511756131360457) {
  H_4(state, unused, out_6103511756131360457);
}
void pose_h_10(double *state, double *unused, double *out_6036451979876331777) {
  h_10(state, unused, out_6036451979876331777);
}
void pose_H_10(double *state, double *unused, double *out_4735514628208408881) {
  H_10(state, unused, out_4735514628208408881);
}
void pose_h_13(double *state, double *unused, double *out_7880905284562057581) {
  h_13(state, unused, out_7880905284562057581);
}
void pose_H_13(double *state, double *unused, double *out_1507119452185340472) {
  H_13(state, unused, out_1507119452185340472);
}
void pose_h_14(double *state, double *unused, double *out_5461180862416464840) {
  h_14(state, unused, out_5461180862416464840);
}
void pose_H_14(double *state, double *unused, double *out_2140270899791875928) {
  H_14(state, unused, out_2140270899791875928);
}
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
}

const EKF pose = {
  .name = "pose",
  .kinds = { 4, 10, 13, 14 },
  .feature_kinds = {  },
  .f_fun = pose_f_fun,
  .F_fun = pose_F_fun,
  .err_fun = pose_err_fun,
  .inv_err_fun = pose_inv_err_fun,
  .H_mod_fun = pose_H_mod_fun,
  .predict = pose_predict,
  .hs = {
    { 4, pose_h_4 },
    { 10, pose_h_10 },
    { 13, pose_h_13 },
    { 14, pose_h_14 },
  },
  .Hs = {
    { 4, pose_H_4 },
    { 10, pose_H_10 },
    { 13, pose_H_13 },
    { 14, pose_H_14 },
  },
  .updates = {
    { 4, pose_update_4 },
    { 10, pose_update_10 },
    { 13, pose_update_13 },
    { 14, pose_update_14 },
  },
  .Hes = {
  },
  .sets = {
  },
  .extra_routines = {
  },
};

ekf_lib_init(pose)
