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
void err_fun(double *nom_x, double *delta_x, double *out_5928268790671582895) {
   out_5928268790671582895[0] = delta_x[0] + nom_x[0];
   out_5928268790671582895[1] = delta_x[1] + nom_x[1];
   out_5928268790671582895[2] = delta_x[2] + nom_x[2];
   out_5928268790671582895[3] = delta_x[3] + nom_x[3];
   out_5928268790671582895[4] = delta_x[4] + nom_x[4];
   out_5928268790671582895[5] = delta_x[5] + nom_x[5];
   out_5928268790671582895[6] = delta_x[6] + nom_x[6];
   out_5928268790671582895[7] = delta_x[7] + nom_x[7];
   out_5928268790671582895[8] = delta_x[8] + nom_x[8];
   out_5928268790671582895[9] = delta_x[9] + nom_x[9];
   out_5928268790671582895[10] = delta_x[10] + nom_x[10];
   out_5928268790671582895[11] = delta_x[11] + nom_x[11];
   out_5928268790671582895[12] = delta_x[12] + nom_x[12];
   out_5928268790671582895[13] = delta_x[13] + nom_x[13];
   out_5928268790671582895[14] = delta_x[14] + nom_x[14];
   out_5928268790671582895[15] = delta_x[15] + nom_x[15];
   out_5928268790671582895[16] = delta_x[16] + nom_x[16];
   out_5928268790671582895[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_5067122510833187171) {
   out_5067122510833187171[0] = -nom_x[0] + true_x[0];
   out_5067122510833187171[1] = -nom_x[1] + true_x[1];
   out_5067122510833187171[2] = -nom_x[2] + true_x[2];
   out_5067122510833187171[3] = -nom_x[3] + true_x[3];
   out_5067122510833187171[4] = -nom_x[4] + true_x[4];
   out_5067122510833187171[5] = -nom_x[5] + true_x[5];
   out_5067122510833187171[6] = -nom_x[6] + true_x[6];
   out_5067122510833187171[7] = -nom_x[7] + true_x[7];
   out_5067122510833187171[8] = -nom_x[8] + true_x[8];
   out_5067122510833187171[9] = -nom_x[9] + true_x[9];
   out_5067122510833187171[10] = -nom_x[10] + true_x[10];
   out_5067122510833187171[11] = -nom_x[11] + true_x[11];
   out_5067122510833187171[12] = -nom_x[12] + true_x[12];
   out_5067122510833187171[13] = -nom_x[13] + true_x[13];
   out_5067122510833187171[14] = -nom_x[14] + true_x[14];
   out_5067122510833187171[15] = -nom_x[15] + true_x[15];
   out_5067122510833187171[16] = -nom_x[16] + true_x[16];
   out_5067122510833187171[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_2154722604321560680) {
   out_2154722604321560680[0] = 1.0;
   out_2154722604321560680[1] = 0.0;
   out_2154722604321560680[2] = 0.0;
   out_2154722604321560680[3] = 0.0;
   out_2154722604321560680[4] = 0.0;
   out_2154722604321560680[5] = 0.0;
   out_2154722604321560680[6] = 0.0;
   out_2154722604321560680[7] = 0.0;
   out_2154722604321560680[8] = 0.0;
   out_2154722604321560680[9] = 0.0;
   out_2154722604321560680[10] = 0.0;
   out_2154722604321560680[11] = 0.0;
   out_2154722604321560680[12] = 0.0;
   out_2154722604321560680[13] = 0.0;
   out_2154722604321560680[14] = 0.0;
   out_2154722604321560680[15] = 0.0;
   out_2154722604321560680[16] = 0.0;
   out_2154722604321560680[17] = 0.0;
   out_2154722604321560680[18] = 0.0;
   out_2154722604321560680[19] = 1.0;
   out_2154722604321560680[20] = 0.0;
   out_2154722604321560680[21] = 0.0;
   out_2154722604321560680[22] = 0.0;
   out_2154722604321560680[23] = 0.0;
   out_2154722604321560680[24] = 0.0;
   out_2154722604321560680[25] = 0.0;
   out_2154722604321560680[26] = 0.0;
   out_2154722604321560680[27] = 0.0;
   out_2154722604321560680[28] = 0.0;
   out_2154722604321560680[29] = 0.0;
   out_2154722604321560680[30] = 0.0;
   out_2154722604321560680[31] = 0.0;
   out_2154722604321560680[32] = 0.0;
   out_2154722604321560680[33] = 0.0;
   out_2154722604321560680[34] = 0.0;
   out_2154722604321560680[35] = 0.0;
   out_2154722604321560680[36] = 0.0;
   out_2154722604321560680[37] = 0.0;
   out_2154722604321560680[38] = 1.0;
   out_2154722604321560680[39] = 0.0;
   out_2154722604321560680[40] = 0.0;
   out_2154722604321560680[41] = 0.0;
   out_2154722604321560680[42] = 0.0;
   out_2154722604321560680[43] = 0.0;
   out_2154722604321560680[44] = 0.0;
   out_2154722604321560680[45] = 0.0;
   out_2154722604321560680[46] = 0.0;
   out_2154722604321560680[47] = 0.0;
   out_2154722604321560680[48] = 0.0;
   out_2154722604321560680[49] = 0.0;
   out_2154722604321560680[50] = 0.0;
   out_2154722604321560680[51] = 0.0;
   out_2154722604321560680[52] = 0.0;
   out_2154722604321560680[53] = 0.0;
   out_2154722604321560680[54] = 0.0;
   out_2154722604321560680[55] = 0.0;
   out_2154722604321560680[56] = 0.0;
   out_2154722604321560680[57] = 1.0;
   out_2154722604321560680[58] = 0.0;
   out_2154722604321560680[59] = 0.0;
   out_2154722604321560680[60] = 0.0;
   out_2154722604321560680[61] = 0.0;
   out_2154722604321560680[62] = 0.0;
   out_2154722604321560680[63] = 0.0;
   out_2154722604321560680[64] = 0.0;
   out_2154722604321560680[65] = 0.0;
   out_2154722604321560680[66] = 0.0;
   out_2154722604321560680[67] = 0.0;
   out_2154722604321560680[68] = 0.0;
   out_2154722604321560680[69] = 0.0;
   out_2154722604321560680[70] = 0.0;
   out_2154722604321560680[71] = 0.0;
   out_2154722604321560680[72] = 0.0;
   out_2154722604321560680[73] = 0.0;
   out_2154722604321560680[74] = 0.0;
   out_2154722604321560680[75] = 0.0;
   out_2154722604321560680[76] = 1.0;
   out_2154722604321560680[77] = 0.0;
   out_2154722604321560680[78] = 0.0;
   out_2154722604321560680[79] = 0.0;
   out_2154722604321560680[80] = 0.0;
   out_2154722604321560680[81] = 0.0;
   out_2154722604321560680[82] = 0.0;
   out_2154722604321560680[83] = 0.0;
   out_2154722604321560680[84] = 0.0;
   out_2154722604321560680[85] = 0.0;
   out_2154722604321560680[86] = 0.0;
   out_2154722604321560680[87] = 0.0;
   out_2154722604321560680[88] = 0.0;
   out_2154722604321560680[89] = 0.0;
   out_2154722604321560680[90] = 0.0;
   out_2154722604321560680[91] = 0.0;
   out_2154722604321560680[92] = 0.0;
   out_2154722604321560680[93] = 0.0;
   out_2154722604321560680[94] = 0.0;
   out_2154722604321560680[95] = 1.0;
   out_2154722604321560680[96] = 0.0;
   out_2154722604321560680[97] = 0.0;
   out_2154722604321560680[98] = 0.0;
   out_2154722604321560680[99] = 0.0;
   out_2154722604321560680[100] = 0.0;
   out_2154722604321560680[101] = 0.0;
   out_2154722604321560680[102] = 0.0;
   out_2154722604321560680[103] = 0.0;
   out_2154722604321560680[104] = 0.0;
   out_2154722604321560680[105] = 0.0;
   out_2154722604321560680[106] = 0.0;
   out_2154722604321560680[107] = 0.0;
   out_2154722604321560680[108] = 0.0;
   out_2154722604321560680[109] = 0.0;
   out_2154722604321560680[110] = 0.0;
   out_2154722604321560680[111] = 0.0;
   out_2154722604321560680[112] = 0.0;
   out_2154722604321560680[113] = 0.0;
   out_2154722604321560680[114] = 1.0;
   out_2154722604321560680[115] = 0.0;
   out_2154722604321560680[116] = 0.0;
   out_2154722604321560680[117] = 0.0;
   out_2154722604321560680[118] = 0.0;
   out_2154722604321560680[119] = 0.0;
   out_2154722604321560680[120] = 0.0;
   out_2154722604321560680[121] = 0.0;
   out_2154722604321560680[122] = 0.0;
   out_2154722604321560680[123] = 0.0;
   out_2154722604321560680[124] = 0.0;
   out_2154722604321560680[125] = 0.0;
   out_2154722604321560680[126] = 0.0;
   out_2154722604321560680[127] = 0.0;
   out_2154722604321560680[128] = 0.0;
   out_2154722604321560680[129] = 0.0;
   out_2154722604321560680[130] = 0.0;
   out_2154722604321560680[131] = 0.0;
   out_2154722604321560680[132] = 0.0;
   out_2154722604321560680[133] = 1.0;
   out_2154722604321560680[134] = 0.0;
   out_2154722604321560680[135] = 0.0;
   out_2154722604321560680[136] = 0.0;
   out_2154722604321560680[137] = 0.0;
   out_2154722604321560680[138] = 0.0;
   out_2154722604321560680[139] = 0.0;
   out_2154722604321560680[140] = 0.0;
   out_2154722604321560680[141] = 0.0;
   out_2154722604321560680[142] = 0.0;
   out_2154722604321560680[143] = 0.0;
   out_2154722604321560680[144] = 0.0;
   out_2154722604321560680[145] = 0.0;
   out_2154722604321560680[146] = 0.0;
   out_2154722604321560680[147] = 0.0;
   out_2154722604321560680[148] = 0.0;
   out_2154722604321560680[149] = 0.0;
   out_2154722604321560680[150] = 0.0;
   out_2154722604321560680[151] = 0.0;
   out_2154722604321560680[152] = 1.0;
   out_2154722604321560680[153] = 0.0;
   out_2154722604321560680[154] = 0.0;
   out_2154722604321560680[155] = 0.0;
   out_2154722604321560680[156] = 0.0;
   out_2154722604321560680[157] = 0.0;
   out_2154722604321560680[158] = 0.0;
   out_2154722604321560680[159] = 0.0;
   out_2154722604321560680[160] = 0.0;
   out_2154722604321560680[161] = 0.0;
   out_2154722604321560680[162] = 0.0;
   out_2154722604321560680[163] = 0.0;
   out_2154722604321560680[164] = 0.0;
   out_2154722604321560680[165] = 0.0;
   out_2154722604321560680[166] = 0.0;
   out_2154722604321560680[167] = 0.0;
   out_2154722604321560680[168] = 0.0;
   out_2154722604321560680[169] = 0.0;
   out_2154722604321560680[170] = 0.0;
   out_2154722604321560680[171] = 1.0;
   out_2154722604321560680[172] = 0.0;
   out_2154722604321560680[173] = 0.0;
   out_2154722604321560680[174] = 0.0;
   out_2154722604321560680[175] = 0.0;
   out_2154722604321560680[176] = 0.0;
   out_2154722604321560680[177] = 0.0;
   out_2154722604321560680[178] = 0.0;
   out_2154722604321560680[179] = 0.0;
   out_2154722604321560680[180] = 0.0;
   out_2154722604321560680[181] = 0.0;
   out_2154722604321560680[182] = 0.0;
   out_2154722604321560680[183] = 0.0;
   out_2154722604321560680[184] = 0.0;
   out_2154722604321560680[185] = 0.0;
   out_2154722604321560680[186] = 0.0;
   out_2154722604321560680[187] = 0.0;
   out_2154722604321560680[188] = 0.0;
   out_2154722604321560680[189] = 0.0;
   out_2154722604321560680[190] = 1.0;
   out_2154722604321560680[191] = 0.0;
   out_2154722604321560680[192] = 0.0;
   out_2154722604321560680[193] = 0.0;
   out_2154722604321560680[194] = 0.0;
   out_2154722604321560680[195] = 0.0;
   out_2154722604321560680[196] = 0.0;
   out_2154722604321560680[197] = 0.0;
   out_2154722604321560680[198] = 0.0;
   out_2154722604321560680[199] = 0.0;
   out_2154722604321560680[200] = 0.0;
   out_2154722604321560680[201] = 0.0;
   out_2154722604321560680[202] = 0.0;
   out_2154722604321560680[203] = 0.0;
   out_2154722604321560680[204] = 0.0;
   out_2154722604321560680[205] = 0.0;
   out_2154722604321560680[206] = 0.0;
   out_2154722604321560680[207] = 0.0;
   out_2154722604321560680[208] = 0.0;
   out_2154722604321560680[209] = 1.0;
   out_2154722604321560680[210] = 0.0;
   out_2154722604321560680[211] = 0.0;
   out_2154722604321560680[212] = 0.0;
   out_2154722604321560680[213] = 0.0;
   out_2154722604321560680[214] = 0.0;
   out_2154722604321560680[215] = 0.0;
   out_2154722604321560680[216] = 0.0;
   out_2154722604321560680[217] = 0.0;
   out_2154722604321560680[218] = 0.0;
   out_2154722604321560680[219] = 0.0;
   out_2154722604321560680[220] = 0.0;
   out_2154722604321560680[221] = 0.0;
   out_2154722604321560680[222] = 0.0;
   out_2154722604321560680[223] = 0.0;
   out_2154722604321560680[224] = 0.0;
   out_2154722604321560680[225] = 0.0;
   out_2154722604321560680[226] = 0.0;
   out_2154722604321560680[227] = 0.0;
   out_2154722604321560680[228] = 1.0;
   out_2154722604321560680[229] = 0.0;
   out_2154722604321560680[230] = 0.0;
   out_2154722604321560680[231] = 0.0;
   out_2154722604321560680[232] = 0.0;
   out_2154722604321560680[233] = 0.0;
   out_2154722604321560680[234] = 0.0;
   out_2154722604321560680[235] = 0.0;
   out_2154722604321560680[236] = 0.0;
   out_2154722604321560680[237] = 0.0;
   out_2154722604321560680[238] = 0.0;
   out_2154722604321560680[239] = 0.0;
   out_2154722604321560680[240] = 0.0;
   out_2154722604321560680[241] = 0.0;
   out_2154722604321560680[242] = 0.0;
   out_2154722604321560680[243] = 0.0;
   out_2154722604321560680[244] = 0.0;
   out_2154722604321560680[245] = 0.0;
   out_2154722604321560680[246] = 0.0;
   out_2154722604321560680[247] = 1.0;
   out_2154722604321560680[248] = 0.0;
   out_2154722604321560680[249] = 0.0;
   out_2154722604321560680[250] = 0.0;
   out_2154722604321560680[251] = 0.0;
   out_2154722604321560680[252] = 0.0;
   out_2154722604321560680[253] = 0.0;
   out_2154722604321560680[254] = 0.0;
   out_2154722604321560680[255] = 0.0;
   out_2154722604321560680[256] = 0.0;
   out_2154722604321560680[257] = 0.0;
   out_2154722604321560680[258] = 0.0;
   out_2154722604321560680[259] = 0.0;
   out_2154722604321560680[260] = 0.0;
   out_2154722604321560680[261] = 0.0;
   out_2154722604321560680[262] = 0.0;
   out_2154722604321560680[263] = 0.0;
   out_2154722604321560680[264] = 0.0;
   out_2154722604321560680[265] = 0.0;
   out_2154722604321560680[266] = 1.0;
   out_2154722604321560680[267] = 0.0;
   out_2154722604321560680[268] = 0.0;
   out_2154722604321560680[269] = 0.0;
   out_2154722604321560680[270] = 0.0;
   out_2154722604321560680[271] = 0.0;
   out_2154722604321560680[272] = 0.0;
   out_2154722604321560680[273] = 0.0;
   out_2154722604321560680[274] = 0.0;
   out_2154722604321560680[275] = 0.0;
   out_2154722604321560680[276] = 0.0;
   out_2154722604321560680[277] = 0.0;
   out_2154722604321560680[278] = 0.0;
   out_2154722604321560680[279] = 0.0;
   out_2154722604321560680[280] = 0.0;
   out_2154722604321560680[281] = 0.0;
   out_2154722604321560680[282] = 0.0;
   out_2154722604321560680[283] = 0.0;
   out_2154722604321560680[284] = 0.0;
   out_2154722604321560680[285] = 1.0;
   out_2154722604321560680[286] = 0.0;
   out_2154722604321560680[287] = 0.0;
   out_2154722604321560680[288] = 0.0;
   out_2154722604321560680[289] = 0.0;
   out_2154722604321560680[290] = 0.0;
   out_2154722604321560680[291] = 0.0;
   out_2154722604321560680[292] = 0.0;
   out_2154722604321560680[293] = 0.0;
   out_2154722604321560680[294] = 0.0;
   out_2154722604321560680[295] = 0.0;
   out_2154722604321560680[296] = 0.0;
   out_2154722604321560680[297] = 0.0;
   out_2154722604321560680[298] = 0.0;
   out_2154722604321560680[299] = 0.0;
   out_2154722604321560680[300] = 0.0;
   out_2154722604321560680[301] = 0.0;
   out_2154722604321560680[302] = 0.0;
   out_2154722604321560680[303] = 0.0;
   out_2154722604321560680[304] = 1.0;
   out_2154722604321560680[305] = 0.0;
   out_2154722604321560680[306] = 0.0;
   out_2154722604321560680[307] = 0.0;
   out_2154722604321560680[308] = 0.0;
   out_2154722604321560680[309] = 0.0;
   out_2154722604321560680[310] = 0.0;
   out_2154722604321560680[311] = 0.0;
   out_2154722604321560680[312] = 0.0;
   out_2154722604321560680[313] = 0.0;
   out_2154722604321560680[314] = 0.0;
   out_2154722604321560680[315] = 0.0;
   out_2154722604321560680[316] = 0.0;
   out_2154722604321560680[317] = 0.0;
   out_2154722604321560680[318] = 0.0;
   out_2154722604321560680[319] = 0.0;
   out_2154722604321560680[320] = 0.0;
   out_2154722604321560680[321] = 0.0;
   out_2154722604321560680[322] = 0.0;
   out_2154722604321560680[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_5197520704410989952) {
   out_5197520704410989952[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_5197520704410989952[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_5197520704410989952[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_5197520704410989952[3] = dt*state[12] + state[3];
   out_5197520704410989952[4] = dt*state[13] + state[4];
   out_5197520704410989952[5] = dt*state[14] + state[5];
   out_5197520704410989952[6] = state[6];
   out_5197520704410989952[7] = state[7];
   out_5197520704410989952[8] = state[8];
   out_5197520704410989952[9] = state[9];
   out_5197520704410989952[10] = state[10];
   out_5197520704410989952[11] = state[11];
   out_5197520704410989952[12] = state[12];
   out_5197520704410989952[13] = state[13];
   out_5197520704410989952[14] = state[14];
   out_5197520704410989952[15] = state[15];
   out_5197520704410989952[16] = state[16];
   out_5197520704410989952[17] = state[17];
}
void F_fun(double *state, double dt, double *out_8407003081910629671) {
   out_8407003081910629671[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8407003081910629671[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8407003081910629671[2] = 0;
   out_8407003081910629671[3] = 0;
   out_8407003081910629671[4] = 0;
   out_8407003081910629671[5] = 0;
   out_8407003081910629671[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8407003081910629671[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8407003081910629671[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_8407003081910629671[9] = 0;
   out_8407003081910629671[10] = 0;
   out_8407003081910629671[11] = 0;
   out_8407003081910629671[12] = 0;
   out_8407003081910629671[13] = 0;
   out_8407003081910629671[14] = 0;
   out_8407003081910629671[15] = 0;
   out_8407003081910629671[16] = 0;
   out_8407003081910629671[17] = 0;
   out_8407003081910629671[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8407003081910629671[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8407003081910629671[20] = 0;
   out_8407003081910629671[21] = 0;
   out_8407003081910629671[22] = 0;
   out_8407003081910629671[23] = 0;
   out_8407003081910629671[24] = 0;
   out_8407003081910629671[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8407003081910629671[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_8407003081910629671[27] = 0;
   out_8407003081910629671[28] = 0;
   out_8407003081910629671[29] = 0;
   out_8407003081910629671[30] = 0;
   out_8407003081910629671[31] = 0;
   out_8407003081910629671[32] = 0;
   out_8407003081910629671[33] = 0;
   out_8407003081910629671[34] = 0;
   out_8407003081910629671[35] = 0;
   out_8407003081910629671[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8407003081910629671[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8407003081910629671[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8407003081910629671[39] = 0;
   out_8407003081910629671[40] = 0;
   out_8407003081910629671[41] = 0;
   out_8407003081910629671[42] = 0;
   out_8407003081910629671[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8407003081910629671[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_8407003081910629671[45] = 0;
   out_8407003081910629671[46] = 0;
   out_8407003081910629671[47] = 0;
   out_8407003081910629671[48] = 0;
   out_8407003081910629671[49] = 0;
   out_8407003081910629671[50] = 0;
   out_8407003081910629671[51] = 0;
   out_8407003081910629671[52] = 0;
   out_8407003081910629671[53] = 0;
   out_8407003081910629671[54] = 0;
   out_8407003081910629671[55] = 0;
   out_8407003081910629671[56] = 0;
   out_8407003081910629671[57] = 1;
   out_8407003081910629671[58] = 0;
   out_8407003081910629671[59] = 0;
   out_8407003081910629671[60] = 0;
   out_8407003081910629671[61] = 0;
   out_8407003081910629671[62] = 0;
   out_8407003081910629671[63] = 0;
   out_8407003081910629671[64] = 0;
   out_8407003081910629671[65] = 0;
   out_8407003081910629671[66] = dt;
   out_8407003081910629671[67] = 0;
   out_8407003081910629671[68] = 0;
   out_8407003081910629671[69] = 0;
   out_8407003081910629671[70] = 0;
   out_8407003081910629671[71] = 0;
   out_8407003081910629671[72] = 0;
   out_8407003081910629671[73] = 0;
   out_8407003081910629671[74] = 0;
   out_8407003081910629671[75] = 0;
   out_8407003081910629671[76] = 1;
   out_8407003081910629671[77] = 0;
   out_8407003081910629671[78] = 0;
   out_8407003081910629671[79] = 0;
   out_8407003081910629671[80] = 0;
   out_8407003081910629671[81] = 0;
   out_8407003081910629671[82] = 0;
   out_8407003081910629671[83] = 0;
   out_8407003081910629671[84] = 0;
   out_8407003081910629671[85] = dt;
   out_8407003081910629671[86] = 0;
   out_8407003081910629671[87] = 0;
   out_8407003081910629671[88] = 0;
   out_8407003081910629671[89] = 0;
   out_8407003081910629671[90] = 0;
   out_8407003081910629671[91] = 0;
   out_8407003081910629671[92] = 0;
   out_8407003081910629671[93] = 0;
   out_8407003081910629671[94] = 0;
   out_8407003081910629671[95] = 1;
   out_8407003081910629671[96] = 0;
   out_8407003081910629671[97] = 0;
   out_8407003081910629671[98] = 0;
   out_8407003081910629671[99] = 0;
   out_8407003081910629671[100] = 0;
   out_8407003081910629671[101] = 0;
   out_8407003081910629671[102] = 0;
   out_8407003081910629671[103] = 0;
   out_8407003081910629671[104] = dt;
   out_8407003081910629671[105] = 0;
   out_8407003081910629671[106] = 0;
   out_8407003081910629671[107] = 0;
   out_8407003081910629671[108] = 0;
   out_8407003081910629671[109] = 0;
   out_8407003081910629671[110] = 0;
   out_8407003081910629671[111] = 0;
   out_8407003081910629671[112] = 0;
   out_8407003081910629671[113] = 0;
   out_8407003081910629671[114] = 1;
   out_8407003081910629671[115] = 0;
   out_8407003081910629671[116] = 0;
   out_8407003081910629671[117] = 0;
   out_8407003081910629671[118] = 0;
   out_8407003081910629671[119] = 0;
   out_8407003081910629671[120] = 0;
   out_8407003081910629671[121] = 0;
   out_8407003081910629671[122] = 0;
   out_8407003081910629671[123] = 0;
   out_8407003081910629671[124] = 0;
   out_8407003081910629671[125] = 0;
   out_8407003081910629671[126] = 0;
   out_8407003081910629671[127] = 0;
   out_8407003081910629671[128] = 0;
   out_8407003081910629671[129] = 0;
   out_8407003081910629671[130] = 0;
   out_8407003081910629671[131] = 0;
   out_8407003081910629671[132] = 0;
   out_8407003081910629671[133] = 1;
   out_8407003081910629671[134] = 0;
   out_8407003081910629671[135] = 0;
   out_8407003081910629671[136] = 0;
   out_8407003081910629671[137] = 0;
   out_8407003081910629671[138] = 0;
   out_8407003081910629671[139] = 0;
   out_8407003081910629671[140] = 0;
   out_8407003081910629671[141] = 0;
   out_8407003081910629671[142] = 0;
   out_8407003081910629671[143] = 0;
   out_8407003081910629671[144] = 0;
   out_8407003081910629671[145] = 0;
   out_8407003081910629671[146] = 0;
   out_8407003081910629671[147] = 0;
   out_8407003081910629671[148] = 0;
   out_8407003081910629671[149] = 0;
   out_8407003081910629671[150] = 0;
   out_8407003081910629671[151] = 0;
   out_8407003081910629671[152] = 1;
   out_8407003081910629671[153] = 0;
   out_8407003081910629671[154] = 0;
   out_8407003081910629671[155] = 0;
   out_8407003081910629671[156] = 0;
   out_8407003081910629671[157] = 0;
   out_8407003081910629671[158] = 0;
   out_8407003081910629671[159] = 0;
   out_8407003081910629671[160] = 0;
   out_8407003081910629671[161] = 0;
   out_8407003081910629671[162] = 0;
   out_8407003081910629671[163] = 0;
   out_8407003081910629671[164] = 0;
   out_8407003081910629671[165] = 0;
   out_8407003081910629671[166] = 0;
   out_8407003081910629671[167] = 0;
   out_8407003081910629671[168] = 0;
   out_8407003081910629671[169] = 0;
   out_8407003081910629671[170] = 0;
   out_8407003081910629671[171] = 1;
   out_8407003081910629671[172] = 0;
   out_8407003081910629671[173] = 0;
   out_8407003081910629671[174] = 0;
   out_8407003081910629671[175] = 0;
   out_8407003081910629671[176] = 0;
   out_8407003081910629671[177] = 0;
   out_8407003081910629671[178] = 0;
   out_8407003081910629671[179] = 0;
   out_8407003081910629671[180] = 0;
   out_8407003081910629671[181] = 0;
   out_8407003081910629671[182] = 0;
   out_8407003081910629671[183] = 0;
   out_8407003081910629671[184] = 0;
   out_8407003081910629671[185] = 0;
   out_8407003081910629671[186] = 0;
   out_8407003081910629671[187] = 0;
   out_8407003081910629671[188] = 0;
   out_8407003081910629671[189] = 0;
   out_8407003081910629671[190] = 1;
   out_8407003081910629671[191] = 0;
   out_8407003081910629671[192] = 0;
   out_8407003081910629671[193] = 0;
   out_8407003081910629671[194] = 0;
   out_8407003081910629671[195] = 0;
   out_8407003081910629671[196] = 0;
   out_8407003081910629671[197] = 0;
   out_8407003081910629671[198] = 0;
   out_8407003081910629671[199] = 0;
   out_8407003081910629671[200] = 0;
   out_8407003081910629671[201] = 0;
   out_8407003081910629671[202] = 0;
   out_8407003081910629671[203] = 0;
   out_8407003081910629671[204] = 0;
   out_8407003081910629671[205] = 0;
   out_8407003081910629671[206] = 0;
   out_8407003081910629671[207] = 0;
   out_8407003081910629671[208] = 0;
   out_8407003081910629671[209] = 1;
   out_8407003081910629671[210] = 0;
   out_8407003081910629671[211] = 0;
   out_8407003081910629671[212] = 0;
   out_8407003081910629671[213] = 0;
   out_8407003081910629671[214] = 0;
   out_8407003081910629671[215] = 0;
   out_8407003081910629671[216] = 0;
   out_8407003081910629671[217] = 0;
   out_8407003081910629671[218] = 0;
   out_8407003081910629671[219] = 0;
   out_8407003081910629671[220] = 0;
   out_8407003081910629671[221] = 0;
   out_8407003081910629671[222] = 0;
   out_8407003081910629671[223] = 0;
   out_8407003081910629671[224] = 0;
   out_8407003081910629671[225] = 0;
   out_8407003081910629671[226] = 0;
   out_8407003081910629671[227] = 0;
   out_8407003081910629671[228] = 1;
   out_8407003081910629671[229] = 0;
   out_8407003081910629671[230] = 0;
   out_8407003081910629671[231] = 0;
   out_8407003081910629671[232] = 0;
   out_8407003081910629671[233] = 0;
   out_8407003081910629671[234] = 0;
   out_8407003081910629671[235] = 0;
   out_8407003081910629671[236] = 0;
   out_8407003081910629671[237] = 0;
   out_8407003081910629671[238] = 0;
   out_8407003081910629671[239] = 0;
   out_8407003081910629671[240] = 0;
   out_8407003081910629671[241] = 0;
   out_8407003081910629671[242] = 0;
   out_8407003081910629671[243] = 0;
   out_8407003081910629671[244] = 0;
   out_8407003081910629671[245] = 0;
   out_8407003081910629671[246] = 0;
   out_8407003081910629671[247] = 1;
   out_8407003081910629671[248] = 0;
   out_8407003081910629671[249] = 0;
   out_8407003081910629671[250] = 0;
   out_8407003081910629671[251] = 0;
   out_8407003081910629671[252] = 0;
   out_8407003081910629671[253] = 0;
   out_8407003081910629671[254] = 0;
   out_8407003081910629671[255] = 0;
   out_8407003081910629671[256] = 0;
   out_8407003081910629671[257] = 0;
   out_8407003081910629671[258] = 0;
   out_8407003081910629671[259] = 0;
   out_8407003081910629671[260] = 0;
   out_8407003081910629671[261] = 0;
   out_8407003081910629671[262] = 0;
   out_8407003081910629671[263] = 0;
   out_8407003081910629671[264] = 0;
   out_8407003081910629671[265] = 0;
   out_8407003081910629671[266] = 1;
   out_8407003081910629671[267] = 0;
   out_8407003081910629671[268] = 0;
   out_8407003081910629671[269] = 0;
   out_8407003081910629671[270] = 0;
   out_8407003081910629671[271] = 0;
   out_8407003081910629671[272] = 0;
   out_8407003081910629671[273] = 0;
   out_8407003081910629671[274] = 0;
   out_8407003081910629671[275] = 0;
   out_8407003081910629671[276] = 0;
   out_8407003081910629671[277] = 0;
   out_8407003081910629671[278] = 0;
   out_8407003081910629671[279] = 0;
   out_8407003081910629671[280] = 0;
   out_8407003081910629671[281] = 0;
   out_8407003081910629671[282] = 0;
   out_8407003081910629671[283] = 0;
   out_8407003081910629671[284] = 0;
   out_8407003081910629671[285] = 1;
   out_8407003081910629671[286] = 0;
   out_8407003081910629671[287] = 0;
   out_8407003081910629671[288] = 0;
   out_8407003081910629671[289] = 0;
   out_8407003081910629671[290] = 0;
   out_8407003081910629671[291] = 0;
   out_8407003081910629671[292] = 0;
   out_8407003081910629671[293] = 0;
   out_8407003081910629671[294] = 0;
   out_8407003081910629671[295] = 0;
   out_8407003081910629671[296] = 0;
   out_8407003081910629671[297] = 0;
   out_8407003081910629671[298] = 0;
   out_8407003081910629671[299] = 0;
   out_8407003081910629671[300] = 0;
   out_8407003081910629671[301] = 0;
   out_8407003081910629671[302] = 0;
   out_8407003081910629671[303] = 0;
   out_8407003081910629671[304] = 1;
   out_8407003081910629671[305] = 0;
   out_8407003081910629671[306] = 0;
   out_8407003081910629671[307] = 0;
   out_8407003081910629671[308] = 0;
   out_8407003081910629671[309] = 0;
   out_8407003081910629671[310] = 0;
   out_8407003081910629671[311] = 0;
   out_8407003081910629671[312] = 0;
   out_8407003081910629671[313] = 0;
   out_8407003081910629671[314] = 0;
   out_8407003081910629671[315] = 0;
   out_8407003081910629671[316] = 0;
   out_8407003081910629671[317] = 0;
   out_8407003081910629671[318] = 0;
   out_8407003081910629671[319] = 0;
   out_8407003081910629671[320] = 0;
   out_8407003081910629671[321] = 0;
   out_8407003081910629671[322] = 0;
   out_8407003081910629671[323] = 1;
}
void h_4(double *state, double *unused, double *out_1815771948845189763) {
   out_1815771948845189763[0] = state[6] + state[9];
   out_1815771948845189763[1] = state[7] + state[10];
   out_1815771948845189763[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_8002804411073519401) {
   out_8002804411073519401[0] = 0;
   out_8002804411073519401[1] = 0;
   out_8002804411073519401[2] = 0;
   out_8002804411073519401[3] = 0;
   out_8002804411073519401[4] = 0;
   out_8002804411073519401[5] = 0;
   out_8002804411073519401[6] = 1;
   out_8002804411073519401[7] = 0;
   out_8002804411073519401[8] = 0;
   out_8002804411073519401[9] = 1;
   out_8002804411073519401[10] = 0;
   out_8002804411073519401[11] = 0;
   out_8002804411073519401[12] = 0;
   out_8002804411073519401[13] = 0;
   out_8002804411073519401[14] = 0;
   out_8002804411073519401[15] = 0;
   out_8002804411073519401[16] = 0;
   out_8002804411073519401[17] = 0;
   out_8002804411073519401[18] = 0;
   out_8002804411073519401[19] = 0;
   out_8002804411073519401[20] = 0;
   out_8002804411073519401[21] = 0;
   out_8002804411073519401[22] = 0;
   out_8002804411073519401[23] = 0;
   out_8002804411073519401[24] = 0;
   out_8002804411073519401[25] = 1;
   out_8002804411073519401[26] = 0;
   out_8002804411073519401[27] = 0;
   out_8002804411073519401[28] = 1;
   out_8002804411073519401[29] = 0;
   out_8002804411073519401[30] = 0;
   out_8002804411073519401[31] = 0;
   out_8002804411073519401[32] = 0;
   out_8002804411073519401[33] = 0;
   out_8002804411073519401[34] = 0;
   out_8002804411073519401[35] = 0;
   out_8002804411073519401[36] = 0;
   out_8002804411073519401[37] = 0;
   out_8002804411073519401[38] = 0;
   out_8002804411073519401[39] = 0;
   out_8002804411073519401[40] = 0;
   out_8002804411073519401[41] = 0;
   out_8002804411073519401[42] = 0;
   out_8002804411073519401[43] = 0;
   out_8002804411073519401[44] = 1;
   out_8002804411073519401[45] = 0;
   out_8002804411073519401[46] = 0;
   out_8002804411073519401[47] = 1;
   out_8002804411073519401[48] = 0;
   out_8002804411073519401[49] = 0;
   out_8002804411073519401[50] = 0;
   out_8002804411073519401[51] = 0;
   out_8002804411073519401[52] = 0;
   out_8002804411073519401[53] = 0;
}
void h_10(double *state, double *unused, double *out_3017578568057486318) {
   out_3017578568057486318[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_3017578568057486318[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_3017578568057486318[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_8560344590081885983) {
   out_8560344590081885983[0] = 0;
   out_8560344590081885983[1] = 9.8100000000000005*cos(state[1]);
   out_8560344590081885983[2] = 0;
   out_8560344590081885983[3] = 0;
   out_8560344590081885983[4] = -state[8];
   out_8560344590081885983[5] = state[7];
   out_8560344590081885983[6] = 0;
   out_8560344590081885983[7] = state[5];
   out_8560344590081885983[8] = -state[4];
   out_8560344590081885983[9] = 0;
   out_8560344590081885983[10] = 0;
   out_8560344590081885983[11] = 0;
   out_8560344590081885983[12] = 1;
   out_8560344590081885983[13] = 0;
   out_8560344590081885983[14] = 0;
   out_8560344590081885983[15] = 1;
   out_8560344590081885983[16] = 0;
   out_8560344590081885983[17] = 0;
   out_8560344590081885983[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_8560344590081885983[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_8560344590081885983[20] = 0;
   out_8560344590081885983[21] = state[8];
   out_8560344590081885983[22] = 0;
   out_8560344590081885983[23] = -state[6];
   out_8560344590081885983[24] = -state[5];
   out_8560344590081885983[25] = 0;
   out_8560344590081885983[26] = state[3];
   out_8560344590081885983[27] = 0;
   out_8560344590081885983[28] = 0;
   out_8560344590081885983[29] = 0;
   out_8560344590081885983[30] = 0;
   out_8560344590081885983[31] = 1;
   out_8560344590081885983[32] = 0;
   out_8560344590081885983[33] = 0;
   out_8560344590081885983[34] = 1;
   out_8560344590081885983[35] = 0;
   out_8560344590081885983[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_8560344590081885983[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_8560344590081885983[38] = 0;
   out_8560344590081885983[39] = -state[7];
   out_8560344590081885983[40] = state[6];
   out_8560344590081885983[41] = 0;
   out_8560344590081885983[42] = state[4];
   out_8560344590081885983[43] = -state[3];
   out_8560344590081885983[44] = 0;
   out_8560344590081885983[45] = 0;
   out_8560344590081885983[46] = 0;
   out_8560344590081885983[47] = 0;
   out_8560344590081885983[48] = 0;
   out_8560344590081885983[49] = 0;
   out_8560344590081885983[50] = 1;
   out_8560344590081885983[51] = 0;
   out_8560344590081885983[52] = 0;
   out_8560344590081885983[53] = 1;
}
void h_13(double *state, double *unused, double *out_2428159707165076335) {
   out_2428159707165076335[0] = state[3];
   out_2428159707165076335[1] = state[4];
   out_2428159707165076335[2] = state[5];
}
void H_13(double *state, double *unused, double *out_4790530585741186600) {
   out_4790530585741186600[0] = 0;
   out_4790530585741186600[1] = 0;
   out_4790530585741186600[2] = 0;
   out_4790530585741186600[3] = 1;
   out_4790530585741186600[4] = 0;
   out_4790530585741186600[5] = 0;
   out_4790530585741186600[6] = 0;
   out_4790530585741186600[7] = 0;
   out_4790530585741186600[8] = 0;
   out_4790530585741186600[9] = 0;
   out_4790530585741186600[10] = 0;
   out_4790530585741186600[11] = 0;
   out_4790530585741186600[12] = 0;
   out_4790530585741186600[13] = 0;
   out_4790530585741186600[14] = 0;
   out_4790530585741186600[15] = 0;
   out_4790530585741186600[16] = 0;
   out_4790530585741186600[17] = 0;
   out_4790530585741186600[18] = 0;
   out_4790530585741186600[19] = 0;
   out_4790530585741186600[20] = 0;
   out_4790530585741186600[21] = 0;
   out_4790530585741186600[22] = 1;
   out_4790530585741186600[23] = 0;
   out_4790530585741186600[24] = 0;
   out_4790530585741186600[25] = 0;
   out_4790530585741186600[26] = 0;
   out_4790530585741186600[27] = 0;
   out_4790530585741186600[28] = 0;
   out_4790530585741186600[29] = 0;
   out_4790530585741186600[30] = 0;
   out_4790530585741186600[31] = 0;
   out_4790530585741186600[32] = 0;
   out_4790530585741186600[33] = 0;
   out_4790530585741186600[34] = 0;
   out_4790530585741186600[35] = 0;
   out_4790530585741186600[36] = 0;
   out_4790530585741186600[37] = 0;
   out_4790530585741186600[38] = 0;
   out_4790530585741186600[39] = 0;
   out_4790530585741186600[40] = 0;
   out_4790530585741186600[41] = 1;
   out_4790530585741186600[42] = 0;
   out_4790530585741186600[43] = 0;
   out_4790530585741186600[44] = 0;
   out_4790530585741186600[45] = 0;
   out_4790530585741186600[46] = 0;
   out_4790530585741186600[47] = 0;
   out_4790530585741186600[48] = 0;
   out_4790530585741186600[49] = 0;
   out_4790530585741186600[50] = 0;
   out_4790530585741186600[51] = 0;
   out_4790530585741186600[52] = 0;
   out_4790530585741186600[53] = 0;
}
void h_14(double *state, double *unused, double *out_8197873605832283710) {
   out_8197873605832283710[0] = state[6];
   out_8197873605832283710[1] = state[7];
   out_8197873605832283710[2] = state[8];
}
void H_14(double *state, double *unused, double *out_4039563554734034872) {
   out_4039563554734034872[0] = 0;
   out_4039563554734034872[1] = 0;
   out_4039563554734034872[2] = 0;
   out_4039563554734034872[3] = 0;
   out_4039563554734034872[4] = 0;
   out_4039563554734034872[5] = 0;
   out_4039563554734034872[6] = 1;
   out_4039563554734034872[7] = 0;
   out_4039563554734034872[8] = 0;
   out_4039563554734034872[9] = 0;
   out_4039563554734034872[10] = 0;
   out_4039563554734034872[11] = 0;
   out_4039563554734034872[12] = 0;
   out_4039563554734034872[13] = 0;
   out_4039563554734034872[14] = 0;
   out_4039563554734034872[15] = 0;
   out_4039563554734034872[16] = 0;
   out_4039563554734034872[17] = 0;
   out_4039563554734034872[18] = 0;
   out_4039563554734034872[19] = 0;
   out_4039563554734034872[20] = 0;
   out_4039563554734034872[21] = 0;
   out_4039563554734034872[22] = 0;
   out_4039563554734034872[23] = 0;
   out_4039563554734034872[24] = 0;
   out_4039563554734034872[25] = 1;
   out_4039563554734034872[26] = 0;
   out_4039563554734034872[27] = 0;
   out_4039563554734034872[28] = 0;
   out_4039563554734034872[29] = 0;
   out_4039563554734034872[30] = 0;
   out_4039563554734034872[31] = 0;
   out_4039563554734034872[32] = 0;
   out_4039563554734034872[33] = 0;
   out_4039563554734034872[34] = 0;
   out_4039563554734034872[35] = 0;
   out_4039563554734034872[36] = 0;
   out_4039563554734034872[37] = 0;
   out_4039563554734034872[38] = 0;
   out_4039563554734034872[39] = 0;
   out_4039563554734034872[40] = 0;
   out_4039563554734034872[41] = 0;
   out_4039563554734034872[42] = 0;
   out_4039563554734034872[43] = 0;
   out_4039563554734034872[44] = 1;
   out_4039563554734034872[45] = 0;
   out_4039563554734034872[46] = 0;
   out_4039563554734034872[47] = 0;
   out_4039563554734034872[48] = 0;
   out_4039563554734034872[49] = 0;
   out_4039563554734034872[50] = 0;
   out_4039563554734034872[51] = 0;
   out_4039563554734034872[52] = 0;
   out_4039563554734034872[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_5928268790671582895) {
  err_fun(nom_x, delta_x, out_5928268790671582895);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_5067122510833187171) {
  inv_err_fun(nom_x, true_x, out_5067122510833187171);
}
void pose_H_mod_fun(double *state, double *out_2154722604321560680) {
  H_mod_fun(state, out_2154722604321560680);
}
void pose_f_fun(double *state, double dt, double *out_5197520704410989952) {
  f_fun(state,  dt, out_5197520704410989952);
}
void pose_F_fun(double *state, double dt, double *out_8407003081910629671) {
  F_fun(state,  dt, out_8407003081910629671);
}
void pose_h_4(double *state, double *unused, double *out_1815771948845189763) {
  h_4(state, unused, out_1815771948845189763);
}
void pose_H_4(double *state, double *unused, double *out_8002804411073519401) {
  H_4(state, unused, out_8002804411073519401);
}
void pose_h_10(double *state, double *unused, double *out_3017578568057486318) {
  h_10(state, unused, out_3017578568057486318);
}
void pose_H_10(double *state, double *unused, double *out_8560344590081885983) {
  H_10(state, unused, out_8560344590081885983);
}
void pose_h_13(double *state, double *unused, double *out_2428159707165076335) {
  h_13(state, unused, out_2428159707165076335);
}
void pose_H_13(double *state, double *unused, double *out_4790530585741186600) {
  H_13(state, unused, out_4790530585741186600);
}
void pose_h_14(double *state, double *unused, double *out_8197873605832283710) {
  h_14(state, unused, out_8197873605832283710);
}
void pose_H_14(double *state, double *unused, double *out_4039563554734034872) {
  H_14(state, unused, out_4039563554734034872);
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
