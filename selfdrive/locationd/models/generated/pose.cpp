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
void err_fun(double *nom_x, double *delta_x, double *out_227942031587100209) {
   out_227942031587100209[0] = delta_x[0] + nom_x[0];
   out_227942031587100209[1] = delta_x[1] + nom_x[1];
   out_227942031587100209[2] = delta_x[2] + nom_x[2];
   out_227942031587100209[3] = delta_x[3] + nom_x[3];
   out_227942031587100209[4] = delta_x[4] + nom_x[4];
   out_227942031587100209[5] = delta_x[5] + nom_x[5];
   out_227942031587100209[6] = delta_x[6] + nom_x[6];
   out_227942031587100209[7] = delta_x[7] + nom_x[7];
   out_227942031587100209[8] = delta_x[8] + nom_x[8];
   out_227942031587100209[9] = delta_x[9] + nom_x[9];
   out_227942031587100209[10] = delta_x[10] + nom_x[10];
   out_227942031587100209[11] = delta_x[11] + nom_x[11];
   out_227942031587100209[12] = delta_x[12] + nom_x[12];
   out_227942031587100209[13] = delta_x[13] + nom_x[13];
   out_227942031587100209[14] = delta_x[14] + nom_x[14];
   out_227942031587100209[15] = delta_x[15] + nom_x[15];
   out_227942031587100209[16] = delta_x[16] + nom_x[16];
   out_227942031587100209[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_6904622864079995273) {
   out_6904622864079995273[0] = -nom_x[0] + true_x[0];
   out_6904622864079995273[1] = -nom_x[1] + true_x[1];
   out_6904622864079995273[2] = -nom_x[2] + true_x[2];
   out_6904622864079995273[3] = -nom_x[3] + true_x[3];
   out_6904622864079995273[4] = -nom_x[4] + true_x[4];
   out_6904622864079995273[5] = -nom_x[5] + true_x[5];
   out_6904622864079995273[6] = -nom_x[6] + true_x[6];
   out_6904622864079995273[7] = -nom_x[7] + true_x[7];
   out_6904622864079995273[8] = -nom_x[8] + true_x[8];
   out_6904622864079995273[9] = -nom_x[9] + true_x[9];
   out_6904622864079995273[10] = -nom_x[10] + true_x[10];
   out_6904622864079995273[11] = -nom_x[11] + true_x[11];
   out_6904622864079995273[12] = -nom_x[12] + true_x[12];
   out_6904622864079995273[13] = -nom_x[13] + true_x[13];
   out_6904622864079995273[14] = -nom_x[14] + true_x[14];
   out_6904622864079995273[15] = -nom_x[15] + true_x[15];
   out_6904622864079995273[16] = -nom_x[16] + true_x[16];
   out_6904622864079995273[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_3022304305314020797) {
   out_3022304305314020797[0] = 1.0;
   out_3022304305314020797[1] = 0.0;
   out_3022304305314020797[2] = 0.0;
   out_3022304305314020797[3] = 0.0;
   out_3022304305314020797[4] = 0.0;
   out_3022304305314020797[5] = 0.0;
   out_3022304305314020797[6] = 0.0;
   out_3022304305314020797[7] = 0.0;
   out_3022304305314020797[8] = 0.0;
   out_3022304305314020797[9] = 0.0;
   out_3022304305314020797[10] = 0.0;
   out_3022304305314020797[11] = 0.0;
   out_3022304305314020797[12] = 0.0;
   out_3022304305314020797[13] = 0.0;
   out_3022304305314020797[14] = 0.0;
   out_3022304305314020797[15] = 0.0;
   out_3022304305314020797[16] = 0.0;
   out_3022304305314020797[17] = 0.0;
   out_3022304305314020797[18] = 0.0;
   out_3022304305314020797[19] = 1.0;
   out_3022304305314020797[20] = 0.0;
   out_3022304305314020797[21] = 0.0;
   out_3022304305314020797[22] = 0.0;
   out_3022304305314020797[23] = 0.0;
   out_3022304305314020797[24] = 0.0;
   out_3022304305314020797[25] = 0.0;
   out_3022304305314020797[26] = 0.0;
   out_3022304305314020797[27] = 0.0;
   out_3022304305314020797[28] = 0.0;
   out_3022304305314020797[29] = 0.0;
   out_3022304305314020797[30] = 0.0;
   out_3022304305314020797[31] = 0.0;
   out_3022304305314020797[32] = 0.0;
   out_3022304305314020797[33] = 0.0;
   out_3022304305314020797[34] = 0.0;
   out_3022304305314020797[35] = 0.0;
   out_3022304305314020797[36] = 0.0;
   out_3022304305314020797[37] = 0.0;
   out_3022304305314020797[38] = 1.0;
   out_3022304305314020797[39] = 0.0;
   out_3022304305314020797[40] = 0.0;
   out_3022304305314020797[41] = 0.0;
   out_3022304305314020797[42] = 0.0;
   out_3022304305314020797[43] = 0.0;
   out_3022304305314020797[44] = 0.0;
   out_3022304305314020797[45] = 0.0;
   out_3022304305314020797[46] = 0.0;
   out_3022304305314020797[47] = 0.0;
   out_3022304305314020797[48] = 0.0;
   out_3022304305314020797[49] = 0.0;
   out_3022304305314020797[50] = 0.0;
   out_3022304305314020797[51] = 0.0;
   out_3022304305314020797[52] = 0.0;
   out_3022304305314020797[53] = 0.0;
   out_3022304305314020797[54] = 0.0;
   out_3022304305314020797[55] = 0.0;
   out_3022304305314020797[56] = 0.0;
   out_3022304305314020797[57] = 1.0;
   out_3022304305314020797[58] = 0.0;
   out_3022304305314020797[59] = 0.0;
   out_3022304305314020797[60] = 0.0;
   out_3022304305314020797[61] = 0.0;
   out_3022304305314020797[62] = 0.0;
   out_3022304305314020797[63] = 0.0;
   out_3022304305314020797[64] = 0.0;
   out_3022304305314020797[65] = 0.0;
   out_3022304305314020797[66] = 0.0;
   out_3022304305314020797[67] = 0.0;
   out_3022304305314020797[68] = 0.0;
   out_3022304305314020797[69] = 0.0;
   out_3022304305314020797[70] = 0.0;
   out_3022304305314020797[71] = 0.0;
   out_3022304305314020797[72] = 0.0;
   out_3022304305314020797[73] = 0.0;
   out_3022304305314020797[74] = 0.0;
   out_3022304305314020797[75] = 0.0;
   out_3022304305314020797[76] = 1.0;
   out_3022304305314020797[77] = 0.0;
   out_3022304305314020797[78] = 0.0;
   out_3022304305314020797[79] = 0.0;
   out_3022304305314020797[80] = 0.0;
   out_3022304305314020797[81] = 0.0;
   out_3022304305314020797[82] = 0.0;
   out_3022304305314020797[83] = 0.0;
   out_3022304305314020797[84] = 0.0;
   out_3022304305314020797[85] = 0.0;
   out_3022304305314020797[86] = 0.0;
   out_3022304305314020797[87] = 0.0;
   out_3022304305314020797[88] = 0.0;
   out_3022304305314020797[89] = 0.0;
   out_3022304305314020797[90] = 0.0;
   out_3022304305314020797[91] = 0.0;
   out_3022304305314020797[92] = 0.0;
   out_3022304305314020797[93] = 0.0;
   out_3022304305314020797[94] = 0.0;
   out_3022304305314020797[95] = 1.0;
   out_3022304305314020797[96] = 0.0;
   out_3022304305314020797[97] = 0.0;
   out_3022304305314020797[98] = 0.0;
   out_3022304305314020797[99] = 0.0;
   out_3022304305314020797[100] = 0.0;
   out_3022304305314020797[101] = 0.0;
   out_3022304305314020797[102] = 0.0;
   out_3022304305314020797[103] = 0.0;
   out_3022304305314020797[104] = 0.0;
   out_3022304305314020797[105] = 0.0;
   out_3022304305314020797[106] = 0.0;
   out_3022304305314020797[107] = 0.0;
   out_3022304305314020797[108] = 0.0;
   out_3022304305314020797[109] = 0.0;
   out_3022304305314020797[110] = 0.0;
   out_3022304305314020797[111] = 0.0;
   out_3022304305314020797[112] = 0.0;
   out_3022304305314020797[113] = 0.0;
   out_3022304305314020797[114] = 1.0;
   out_3022304305314020797[115] = 0.0;
   out_3022304305314020797[116] = 0.0;
   out_3022304305314020797[117] = 0.0;
   out_3022304305314020797[118] = 0.0;
   out_3022304305314020797[119] = 0.0;
   out_3022304305314020797[120] = 0.0;
   out_3022304305314020797[121] = 0.0;
   out_3022304305314020797[122] = 0.0;
   out_3022304305314020797[123] = 0.0;
   out_3022304305314020797[124] = 0.0;
   out_3022304305314020797[125] = 0.0;
   out_3022304305314020797[126] = 0.0;
   out_3022304305314020797[127] = 0.0;
   out_3022304305314020797[128] = 0.0;
   out_3022304305314020797[129] = 0.0;
   out_3022304305314020797[130] = 0.0;
   out_3022304305314020797[131] = 0.0;
   out_3022304305314020797[132] = 0.0;
   out_3022304305314020797[133] = 1.0;
   out_3022304305314020797[134] = 0.0;
   out_3022304305314020797[135] = 0.0;
   out_3022304305314020797[136] = 0.0;
   out_3022304305314020797[137] = 0.0;
   out_3022304305314020797[138] = 0.0;
   out_3022304305314020797[139] = 0.0;
   out_3022304305314020797[140] = 0.0;
   out_3022304305314020797[141] = 0.0;
   out_3022304305314020797[142] = 0.0;
   out_3022304305314020797[143] = 0.0;
   out_3022304305314020797[144] = 0.0;
   out_3022304305314020797[145] = 0.0;
   out_3022304305314020797[146] = 0.0;
   out_3022304305314020797[147] = 0.0;
   out_3022304305314020797[148] = 0.0;
   out_3022304305314020797[149] = 0.0;
   out_3022304305314020797[150] = 0.0;
   out_3022304305314020797[151] = 0.0;
   out_3022304305314020797[152] = 1.0;
   out_3022304305314020797[153] = 0.0;
   out_3022304305314020797[154] = 0.0;
   out_3022304305314020797[155] = 0.0;
   out_3022304305314020797[156] = 0.0;
   out_3022304305314020797[157] = 0.0;
   out_3022304305314020797[158] = 0.0;
   out_3022304305314020797[159] = 0.0;
   out_3022304305314020797[160] = 0.0;
   out_3022304305314020797[161] = 0.0;
   out_3022304305314020797[162] = 0.0;
   out_3022304305314020797[163] = 0.0;
   out_3022304305314020797[164] = 0.0;
   out_3022304305314020797[165] = 0.0;
   out_3022304305314020797[166] = 0.0;
   out_3022304305314020797[167] = 0.0;
   out_3022304305314020797[168] = 0.0;
   out_3022304305314020797[169] = 0.0;
   out_3022304305314020797[170] = 0.0;
   out_3022304305314020797[171] = 1.0;
   out_3022304305314020797[172] = 0.0;
   out_3022304305314020797[173] = 0.0;
   out_3022304305314020797[174] = 0.0;
   out_3022304305314020797[175] = 0.0;
   out_3022304305314020797[176] = 0.0;
   out_3022304305314020797[177] = 0.0;
   out_3022304305314020797[178] = 0.0;
   out_3022304305314020797[179] = 0.0;
   out_3022304305314020797[180] = 0.0;
   out_3022304305314020797[181] = 0.0;
   out_3022304305314020797[182] = 0.0;
   out_3022304305314020797[183] = 0.0;
   out_3022304305314020797[184] = 0.0;
   out_3022304305314020797[185] = 0.0;
   out_3022304305314020797[186] = 0.0;
   out_3022304305314020797[187] = 0.0;
   out_3022304305314020797[188] = 0.0;
   out_3022304305314020797[189] = 0.0;
   out_3022304305314020797[190] = 1.0;
   out_3022304305314020797[191] = 0.0;
   out_3022304305314020797[192] = 0.0;
   out_3022304305314020797[193] = 0.0;
   out_3022304305314020797[194] = 0.0;
   out_3022304305314020797[195] = 0.0;
   out_3022304305314020797[196] = 0.0;
   out_3022304305314020797[197] = 0.0;
   out_3022304305314020797[198] = 0.0;
   out_3022304305314020797[199] = 0.0;
   out_3022304305314020797[200] = 0.0;
   out_3022304305314020797[201] = 0.0;
   out_3022304305314020797[202] = 0.0;
   out_3022304305314020797[203] = 0.0;
   out_3022304305314020797[204] = 0.0;
   out_3022304305314020797[205] = 0.0;
   out_3022304305314020797[206] = 0.0;
   out_3022304305314020797[207] = 0.0;
   out_3022304305314020797[208] = 0.0;
   out_3022304305314020797[209] = 1.0;
   out_3022304305314020797[210] = 0.0;
   out_3022304305314020797[211] = 0.0;
   out_3022304305314020797[212] = 0.0;
   out_3022304305314020797[213] = 0.0;
   out_3022304305314020797[214] = 0.0;
   out_3022304305314020797[215] = 0.0;
   out_3022304305314020797[216] = 0.0;
   out_3022304305314020797[217] = 0.0;
   out_3022304305314020797[218] = 0.0;
   out_3022304305314020797[219] = 0.0;
   out_3022304305314020797[220] = 0.0;
   out_3022304305314020797[221] = 0.0;
   out_3022304305314020797[222] = 0.0;
   out_3022304305314020797[223] = 0.0;
   out_3022304305314020797[224] = 0.0;
   out_3022304305314020797[225] = 0.0;
   out_3022304305314020797[226] = 0.0;
   out_3022304305314020797[227] = 0.0;
   out_3022304305314020797[228] = 1.0;
   out_3022304305314020797[229] = 0.0;
   out_3022304305314020797[230] = 0.0;
   out_3022304305314020797[231] = 0.0;
   out_3022304305314020797[232] = 0.0;
   out_3022304305314020797[233] = 0.0;
   out_3022304305314020797[234] = 0.0;
   out_3022304305314020797[235] = 0.0;
   out_3022304305314020797[236] = 0.0;
   out_3022304305314020797[237] = 0.0;
   out_3022304305314020797[238] = 0.0;
   out_3022304305314020797[239] = 0.0;
   out_3022304305314020797[240] = 0.0;
   out_3022304305314020797[241] = 0.0;
   out_3022304305314020797[242] = 0.0;
   out_3022304305314020797[243] = 0.0;
   out_3022304305314020797[244] = 0.0;
   out_3022304305314020797[245] = 0.0;
   out_3022304305314020797[246] = 0.0;
   out_3022304305314020797[247] = 1.0;
   out_3022304305314020797[248] = 0.0;
   out_3022304305314020797[249] = 0.0;
   out_3022304305314020797[250] = 0.0;
   out_3022304305314020797[251] = 0.0;
   out_3022304305314020797[252] = 0.0;
   out_3022304305314020797[253] = 0.0;
   out_3022304305314020797[254] = 0.0;
   out_3022304305314020797[255] = 0.0;
   out_3022304305314020797[256] = 0.0;
   out_3022304305314020797[257] = 0.0;
   out_3022304305314020797[258] = 0.0;
   out_3022304305314020797[259] = 0.0;
   out_3022304305314020797[260] = 0.0;
   out_3022304305314020797[261] = 0.0;
   out_3022304305314020797[262] = 0.0;
   out_3022304305314020797[263] = 0.0;
   out_3022304305314020797[264] = 0.0;
   out_3022304305314020797[265] = 0.0;
   out_3022304305314020797[266] = 1.0;
   out_3022304305314020797[267] = 0.0;
   out_3022304305314020797[268] = 0.0;
   out_3022304305314020797[269] = 0.0;
   out_3022304305314020797[270] = 0.0;
   out_3022304305314020797[271] = 0.0;
   out_3022304305314020797[272] = 0.0;
   out_3022304305314020797[273] = 0.0;
   out_3022304305314020797[274] = 0.0;
   out_3022304305314020797[275] = 0.0;
   out_3022304305314020797[276] = 0.0;
   out_3022304305314020797[277] = 0.0;
   out_3022304305314020797[278] = 0.0;
   out_3022304305314020797[279] = 0.0;
   out_3022304305314020797[280] = 0.0;
   out_3022304305314020797[281] = 0.0;
   out_3022304305314020797[282] = 0.0;
   out_3022304305314020797[283] = 0.0;
   out_3022304305314020797[284] = 0.0;
   out_3022304305314020797[285] = 1.0;
   out_3022304305314020797[286] = 0.0;
   out_3022304305314020797[287] = 0.0;
   out_3022304305314020797[288] = 0.0;
   out_3022304305314020797[289] = 0.0;
   out_3022304305314020797[290] = 0.0;
   out_3022304305314020797[291] = 0.0;
   out_3022304305314020797[292] = 0.0;
   out_3022304305314020797[293] = 0.0;
   out_3022304305314020797[294] = 0.0;
   out_3022304305314020797[295] = 0.0;
   out_3022304305314020797[296] = 0.0;
   out_3022304305314020797[297] = 0.0;
   out_3022304305314020797[298] = 0.0;
   out_3022304305314020797[299] = 0.0;
   out_3022304305314020797[300] = 0.0;
   out_3022304305314020797[301] = 0.0;
   out_3022304305314020797[302] = 0.0;
   out_3022304305314020797[303] = 0.0;
   out_3022304305314020797[304] = 1.0;
   out_3022304305314020797[305] = 0.0;
   out_3022304305314020797[306] = 0.0;
   out_3022304305314020797[307] = 0.0;
   out_3022304305314020797[308] = 0.0;
   out_3022304305314020797[309] = 0.0;
   out_3022304305314020797[310] = 0.0;
   out_3022304305314020797[311] = 0.0;
   out_3022304305314020797[312] = 0.0;
   out_3022304305314020797[313] = 0.0;
   out_3022304305314020797[314] = 0.0;
   out_3022304305314020797[315] = 0.0;
   out_3022304305314020797[316] = 0.0;
   out_3022304305314020797[317] = 0.0;
   out_3022304305314020797[318] = 0.0;
   out_3022304305314020797[319] = 0.0;
   out_3022304305314020797[320] = 0.0;
   out_3022304305314020797[321] = 0.0;
   out_3022304305314020797[322] = 0.0;
   out_3022304305314020797[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_5347359958206634853) {
   out_5347359958206634853[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_5347359958206634853[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_5347359958206634853[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_5347359958206634853[3] = dt*state[12] + state[3];
   out_5347359958206634853[4] = dt*state[13] + state[4];
   out_5347359958206634853[5] = dt*state[14] + state[5];
   out_5347359958206634853[6] = state[6];
   out_5347359958206634853[7] = state[7];
   out_5347359958206634853[8] = state[8];
   out_5347359958206634853[9] = state[9];
   out_5347359958206634853[10] = state[10];
   out_5347359958206634853[11] = state[11];
   out_5347359958206634853[12] = state[12];
   out_5347359958206634853[13] = state[13];
   out_5347359958206634853[14] = state[14];
   out_5347359958206634853[15] = state[15];
   out_5347359958206634853[16] = state[16];
   out_5347359958206634853[17] = state[17];
}
void F_fun(double *state, double dt, double *out_2486298903049178646) {
   out_2486298903049178646[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2486298903049178646[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2486298903049178646[2] = 0;
   out_2486298903049178646[3] = 0;
   out_2486298903049178646[4] = 0;
   out_2486298903049178646[5] = 0;
   out_2486298903049178646[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2486298903049178646[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2486298903049178646[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_2486298903049178646[9] = 0;
   out_2486298903049178646[10] = 0;
   out_2486298903049178646[11] = 0;
   out_2486298903049178646[12] = 0;
   out_2486298903049178646[13] = 0;
   out_2486298903049178646[14] = 0;
   out_2486298903049178646[15] = 0;
   out_2486298903049178646[16] = 0;
   out_2486298903049178646[17] = 0;
   out_2486298903049178646[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2486298903049178646[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2486298903049178646[20] = 0;
   out_2486298903049178646[21] = 0;
   out_2486298903049178646[22] = 0;
   out_2486298903049178646[23] = 0;
   out_2486298903049178646[24] = 0;
   out_2486298903049178646[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2486298903049178646[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_2486298903049178646[27] = 0;
   out_2486298903049178646[28] = 0;
   out_2486298903049178646[29] = 0;
   out_2486298903049178646[30] = 0;
   out_2486298903049178646[31] = 0;
   out_2486298903049178646[32] = 0;
   out_2486298903049178646[33] = 0;
   out_2486298903049178646[34] = 0;
   out_2486298903049178646[35] = 0;
   out_2486298903049178646[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2486298903049178646[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2486298903049178646[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2486298903049178646[39] = 0;
   out_2486298903049178646[40] = 0;
   out_2486298903049178646[41] = 0;
   out_2486298903049178646[42] = 0;
   out_2486298903049178646[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2486298903049178646[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_2486298903049178646[45] = 0;
   out_2486298903049178646[46] = 0;
   out_2486298903049178646[47] = 0;
   out_2486298903049178646[48] = 0;
   out_2486298903049178646[49] = 0;
   out_2486298903049178646[50] = 0;
   out_2486298903049178646[51] = 0;
   out_2486298903049178646[52] = 0;
   out_2486298903049178646[53] = 0;
   out_2486298903049178646[54] = 0;
   out_2486298903049178646[55] = 0;
   out_2486298903049178646[56] = 0;
   out_2486298903049178646[57] = 1;
   out_2486298903049178646[58] = 0;
   out_2486298903049178646[59] = 0;
   out_2486298903049178646[60] = 0;
   out_2486298903049178646[61] = 0;
   out_2486298903049178646[62] = 0;
   out_2486298903049178646[63] = 0;
   out_2486298903049178646[64] = 0;
   out_2486298903049178646[65] = 0;
   out_2486298903049178646[66] = dt;
   out_2486298903049178646[67] = 0;
   out_2486298903049178646[68] = 0;
   out_2486298903049178646[69] = 0;
   out_2486298903049178646[70] = 0;
   out_2486298903049178646[71] = 0;
   out_2486298903049178646[72] = 0;
   out_2486298903049178646[73] = 0;
   out_2486298903049178646[74] = 0;
   out_2486298903049178646[75] = 0;
   out_2486298903049178646[76] = 1;
   out_2486298903049178646[77] = 0;
   out_2486298903049178646[78] = 0;
   out_2486298903049178646[79] = 0;
   out_2486298903049178646[80] = 0;
   out_2486298903049178646[81] = 0;
   out_2486298903049178646[82] = 0;
   out_2486298903049178646[83] = 0;
   out_2486298903049178646[84] = 0;
   out_2486298903049178646[85] = dt;
   out_2486298903049178646[86] = 0;
   out_2486298903049178646[87] = 0;
   out_2486298903049178646[88] = 0;
   out_2486298903049178646[89] = 0;
   out_2486298903049178646[90] = 0;
   out_2486298903049178646[91] = 0;
   out_2486298903049178646[92] = 0;
   out_2486298903049178646[93] = 0;
   out_2486298903049178646[94] = 0;
   out_2486298903049178646[95] = 1;
   out_2486298903049178646[96] = 0;
   out_2486298903049178646[97] = 0;
   out_2486298903049178646[98] = 0;
   out_2486298903049178646[99] = 0;
   out_2486298903049178646[100] = 0;
   out_2486298903049178646[101] = 0;
   out_2486298903049178646[102] = 0;
   out_2486298903049178646[103] = 0;
   out_2486298903049178646[104] = dt;
   out_2486298903049178646[105] = 0;
   out_2486298903049178646[106] = 0;
   out_2486298903049178646[107] = 0;
   out_2486298903049178646[108] = 0;
   out_2486298903049178646[109] = 0;
   out_2486298903049178646[110] = 0;
   out_2486298903049178646[111] = 0;
   out_2486298903049178646[112] = 0;
   out_2486298903049178646[113] = 0;
   out_2486298903049178646[114] = 1;
   out_2486298903049178646[115] = 0;
   out_2486298903049178646[116] = 0;
   out_2486298903049178646[117] = 0;
   out_2486298903049178646[118] = 0;
   out_2486298903049178646[119] = 0;
   out_2486298903049178646[120] = 0;
   out_2486298903049178646[121] = 0;
   out_2486298903049178646[122] = 0;
   out_2486298903049178646[123] = 0;
   out_2486298903049178646[124] = 0;
   out_2486298903049178646[125] = 0;
   out_2486298903049178646[126] = 0;
   out_2486298903049178646[127] = 0;
   out_2486298903049178646[128] = 0;
   out_2486298903049178646[129] = 0;
   out_2486298903049178646[130] = 0;
   out_2486298903049178646[131] = 0;
   out_2486298903049178646[132] = 0;
   out_2486298903049178646[133] = 1;
   out_2486298903049178646[134] = 0;
   out_2486298903049178646[135] = 0;
   out_2486298903049178646[136] = 0;
   out_2486298903049178646[137] = 0;
   out_2486298903049178646[138] = 0;
   out_2486298903049178646[139] = 0;
   out_2486298903049178646[140] = 0;
   out_2486298903049178646[141] = 0;
   out_2486298903049178646[142] = 0;
   out_2486298903049178646[143] = 0;
   out_2486298903049178646[144] = 0;
   out_2486298903049178646[145] = 0;
   out_2486298903049178646[146] = 0;
   out_2486298903049178646[147] = 0;
   out_2486298903049178646[148] = 0;
   out_2486298903049178646[149] = 0;
   out_2486298903049178646[150] = 0;
   out_2486298903049178646[151] = 0;
   out_2486298903049178646[152] = 1;
   out_2486298903049178646[153] = 0;
   out_2486298903049178646[154] = 0;
   out_2486298903049178646[155] = 0;
   out_2486298903049178646[156] = 0;
   out_2486298903049178646[157] = 0;
   out_2486298903049178646[158] = 0;
   out_2486298903049178646[159] = 0;
   out_2486298903049178646[160] = 0;
   out_2486298903049178646[161] = 0;
   out_2486298903049178646[162] = 0;
   out_2486298903049178646[163] = 0;
   out_2486298903049178646[164] = 0;
   out_2486298903049178646[165] = 0;
   out_2486298903049178646[166] = 0;
   out_2486298903049178646[167] = 0;
   out_2486298903049178646[168] = 0;
   out_2486298903049178646[169] = 0;
   out_2486298903049178646[170] = 0;
   out_2486298903049178646[171] = 1;
   out_2486298903049178646[172] = 0;
   out_2486298903049178646[173] = 0;
   out_2486298903049178646[174] = 0;
   out_2486298903049178646[175] = 0;
   out_2486298903049178646[176] = 0;
   out_2486298903049178646[177] = 0;
   out_2486298903049178646[178] = 0;
   out_2486298903049178646[179] = 0;
   out_2486298903049178646[180] = 0;
   out_2486298903049178646[181] = 0;
   out_2486298903049178646[182] = 0;
   out_2486298903049178646[183] = 0;
   out_2486298903049178646[184] = 0;
   out_2486298903049178646[185] = 0;
   out_2486298903049178646[186] = 0;
   out_2486298903049178646[187] = 0;
   out_2486298903049178646[188] = 0;
   out_2486298903049178646[189] = 0;
   out_2486298903049178646[190] = 1;
   out_2486298903049178646[191] = 0;
   out_2486298903049178646[192] = 0;
   out_2486298903049178646[193] = 0;
   out_2486298903049178646[194] = 0;
   out_2486298903049178646[195] = 0;
   out_2486298903049178646[196] = 0;
   out_2486298903049178646[197] = 0;
   out_2486298903049178646[198] = 0;
   out_2486298903049178646[199] = 0;
   out_2486298903049178646[200] = 0;
   out_2486298903049178646[201] = 0;
   out_2486298903049178646[202] = 0;
   out_2486298903049178646[203] = 0;
   out_2486298903049178646[204] = 0;
   out_2486298903049178646[205] = 0;
   out_2486298903049178646[206] = 0;
   out_2486298903049178646[207] = 0;
   out_2486298903049178646[208] = 0;
   out_2486298903049178646[209] = 1;
   out_2486298903049178646[210] = 0;
   out_2486298903049178646[211] = 0;
   out_2486298903049178646[212] = 0;
   out_2486298903049178646[213] = 0;
   out_2486298903049178646[214] = 0;
   out_2486298903049178646[215] = 0;
   out_2486298903049178646[216] = 0;
   out_2486298903049178646[217] = 0;
   out_2486298903049178646[218] = 0;
   out_2486298903049178646[219] = 0;
   out_2486298903049178646[220] = 0;
   out_2486298903049178646[221] = 0;
   out_2486298903049178646[222] = 0;
   out_2486298903049178646[223] = 0;
   out_2486298903049178646[224] = 0;
   out_2486298903049178646[225] = 0;
   out_2486298903049178646[226] = 0;
   out_2486298903049178646[227] = 0;
   out_2486298903049178646[228] = 1;
   out_2486298903049178646[229] = 0;
   out_2486298903049178646[230] = 0;
   out_2486298903049178646[231] = 0;
   out_2486298903049178646[232] = 0;
   out_2486298903049178646[233] = 0;
   out_2486298903049178646[234] = 0;
   out_2486298903049178646[235] = 0;
   out_2486298903049178646[236] = 0;
   out_2486298903049178646[237] = 0;
   out_2486298903049178646[238] = 0;
   out_2486298903049178646[239] = 0;
   out_2486298903049178646[240] = 0;
   out_2486298903049178646[241] = 0;
   out_2486298903049178646[242] = 0;
   out_2486298903049178646[243] = 0;
   out_2486298903049178646[244] = 0;
   out_2486298903049178646[245] = 0;
   out_2486298903049178646[246] = 0;
   out_2486298903049178646[247] = 1;
   out_2486298903049178646[248] = 0;
   out_2486298903049178646[249] = 0;
   out_2486298903049178646[250] = 0;
   out_2486298903049178646[251] = 0;
   out_2486298903049178646[252] = 0;
   out_2486298903049178646[253] = 0;
   out_2486298903049178646[254] = 0;
   out_2486298903049178646[255] = 0;
   out_2486298903049178646[256] = 0;
   out_2486298903049178646[257] = 0;
   out_2486298903049178646[258] = 0;
   out_2486298903049178646[259] = 0;
   out_2486298903049178646[260] = 0;
   out_2486298903049178646[261] = 0;
   out_2486298903049178646[262] = 0;
   out_2486298903049178646[263] = 0;
   out_2486298903049178646[264] = 0;
   out_2486298903049178646[265] = 0;
   out_2486298903049178646[266] = 1;
   out_2486298903049178646[267] = 0;
   out_2486298903049178646[268] = 0;
   out_2486298903049178646[269] = 0;
   out_2486298903049178646[270] = 0;
   out_2486298903049178646[271] = 0;
   out_2486298903049178646[272] = 0;
   out_2486298903049178646[273] = 0;
   out_2486298903049178646[274] = 0;
   out_2486298903049178646[275] = 0;
   out_2486298903049178646[276] = 0;
   out_2486298903049178646[277] = 0;
   out_2486298903049178646[278] = 0;
   out_2486298903049178646[279] = 0;
   out_2486298903049178646[280] = 0;
   out_2486298903049178646[281] = 0;
   out_2486298903049178646[282] = 0;
   out_2486298903049178646[283] = 0;
   out_2486298903049178646[284] = 0;
   out_2486298903049178646[285] = 1;
   out_2486298903049178646[286] = 0;
   out_2486298903049178646[287] = 0;
   out_2486298903049178646[288] = 0;
   out_2486298903049178646[289] = 0;
   out_2486298903049178646[290] = 0;
   out_2486298903049178646[291] = 0;
   out_2486298903049178646[292] = 0;
   out_2486298903049178646[293] = 0;
   out_2486298903049178646[294] = 0;
   out_2486298903049178646[295] = 0;
   out_2486298903049178646[296] = 0;
   out_2486298903049178646[297] = 0;
   out_2486298903049178646[298] = 0;
   out_2486298903049178646[299] = 0;
   out_2486298903049178646[300] = 0;
   out_2486298903049178646[301] = 0;
   out_2486298903049178646[302] = 0;
   out_2486298903049178646[303] = 0;
   out_2486298903049178646[304] = 1;
   out_2486298903049178646[305] = 0;
   out_2486298903049178646[306] = 0;
   out_2486298903049178646[307] = 0;
   out_2486298903049178646[308] = 0;
   out_2486298903049178646[309] = 0;
   out_2486298903049178646[310] = 0;
   out_2486298903049178646[311] = 0;
   out_2486298903049178646[312] = 0;
   out_2486298903049178646[313] = 0;
   out_2486298903049178646[314] = 0;
   out_2486298903049178646[315] = 0;
   out_2486298903049178646[316] = 0;
   out_2486298903049178646[317] = 0;
   out_2486298903049178646[318] = 0;
   out_2486298903049178646[319] = 0;
   out_2486298903049178646[320] = 0;
   out_2486298903049178646[321] = 0;
   out_2486298903049178646[322] = 0;
   out_2486298903049178646[323] = 1;
}
void h_4(double *state, double *unused, double *out_4503598515101124135) {
   out_4503598515101124135[0] = state[6] + state[9];
   out_4503598515101124135[1] = state[7] + state[10];
   out_4503598515101124135[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_6296526384104970050) {
   out_6296526384104970050[0] = 0;
   out_6296526384104970050[1] = 0;
   out_6296526384104970050[2] = 0;
   out_6296526384104970050[3] = 0;
   out_6296526384104970050[4] = 0;
   out_6296526384104970050[5] = 0;
   out_6296526384104970050[6] = 1;
   out_6296526384104970050[7] = 0;
   out_6296526384104970050[8] = 0;
   out_6296526384104970050[9] = 1;
   out_6296526384104970050[10] = 0;
   out_6296526384104970050[11] = 0;
   out_6296526384104970050[12] = 0;
   out_6296526384104970050[13] = 0;
   out_6296526384104970050[14] = 0;
   out_6296526384104970050[15] = 0;
   out_6296526384104970050[16] = 0;
   out_6296526384104970050[17] = 0;
   out_6296526384104970050[18] = 0;
   out_6296526384104970050[19] = 0;
   out_6296526384104970050[20] = 0;
   out_6296526384104970050[21] = 0;
   out_6296526384104970050[22] = 0;
   out_6296526384104970050[23] = 0;
   out_6296526384104970050[24] = 0;
   out_6296526384104970050[25] = 1;
   out_6296526384104970050[26] = 0;
   out_6296526384104970050[27] = 0;
   out_6296526384104970050[28] = 1;
   out_6296526384104970050[29] = 0;
   out_6296526384104970050[30] = 0;
   out_6296526384104970050[31] = 0;
   out_6296526384104970050[32] = 0;
   out_6296526384104970050[33] = 0;
   out_6296526384104970050[34] = 0;
   out_6296526384104970050[35] = 0;
   out_6296526384104970050[36] = 0;
   out_6296526384104970050[37] = 0;
   out_6296526384104970050[38] = 0;
   out_6296526384104970050[39] = 0;
   out_6296526384104970050[40] = 0;
   out_6296526384104970050[41] = 0;
   out_6296526384104970050[42] = 0;
   out_6296526384104970050[43] = 0;
   out_6296526384104970050[44] = 1;
   out_6296526384104970050[45] = 0;
   out_6296526384104970050[46] = 0;
   out_6296526384104970050[47] = 1;
   out_6296526384104970050[48] = 0;
   out_6296526384104970050[49] = 0;
   out_6296526384104970050[50] = 0;
   out_6296526384104970050[51] = 0;
   out_6296526384104970050[52] = 0;
   out_6296526384104970050[53] = 0;
}
void h_10(double *state, double *unused, double *out_660870634115408416) {
   out_660870634115408416[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_660870634115408416[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_660870634115408416[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_2423980848173730331) {
   out_2423980848173730331[0] = 0;
   out_2423980848173730331[1] = 9.8100000000000005*cos(state[1]);
   out_2423980848173730331[2] = 0;
   out_2423980848173730331[3] = 0;
   out_2423980848173730331[4] = -state[8];
   out_2423980848173730331[5] = state[7];
   out_2423980848173730331[6] = 0;
   out_2423980848173730331[7] = state[5];
   out_2423980848173730331[8] = -state[4];
   out_2423980848173730331[9] = 0;
   out_2423980848173730331[10] = 0;
   out_2423980848173730331[11] = 0;
   out_2423980848173730331[12] = 1;
   out_2423980848173730331[13] = 0;
   out_2423980848173730331[14] = 0;
   out_2423980848173730331[15] = 1;
   out_2423980848173730331[16] = 0;
   out_2423980848173730331[17] = 0;
   out_2423980848173730331[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_2423980848173730331[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_2423980848173730331[20] = 0;
   out_2423980848173730331[21] = state[8];
   out_2423980848173730331[22] = 0;
   out_2423980848173730331[23] = -state[6];
   out_2423980848173730331[24] = -state[5];
   out_2423980848173730331[25] = 0;
   out_2423980848173730331[26] = state[3];
   out_2423980848173730331[27] = 0;
   out_2423980848173730331[28] = 0;
   out_2423980848173730331[29] = 0;
   out_2423980848173730331[30] = 0;
   out_2423980848173730331[31] = 1;
   out_2423980848173730331[32] = 0;
   out_2423980848173730331[33] = 0;
   out_2423980848173730331[34] = 1;
   out_2423980848173730331[35] = 0;
   out_2423980848173730331[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_2423980848173730331[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_2423980848173730331[38] = 0;
   out_2423980848173730331[39] = -state[7];
   out_2423980848173730331[40] = state[6];
   out_2423980848173730331[41] = 0;
   out_2423980848173730331[42] = state[4];
   out_2423980848173730331[43] = -state[3];
   out_2423980848173730331[44] = 0;
   out_2423980848173730331[45] = 0;
   out_2423980848173730331[46] = 0;
   out_2423980848173730331[47] = 0;
   out_2423980848173730331[48] = 0;
   out_2423980848173730331[49] = 0;
   out_2423980848173730331[50] = 1;
   out_2423980848173730331[51] = 0;
   out_2423980848173730331[52] = 0;
   out_2423980848173730331[53] = 1;
}
void h_13(double *state, double *unused, double *out_4084654337287946880) {
   out_4084654337287946880[0] = state[3];
   out_4084654337287946880[1] = state[4];
   out_4084654337287946880[2] = state[5];
}
void H_13(double *state, double *unused, double *out_3084252558772637249) {
   out_3084252558772637249[0] = 0;
   out_3084252558772637249[1] = 0;
   out_3084252558772637249[2] = 0;
   out_3084252558772637249[3] = 1;
   out_3084252558772637249[4] = 0;
   out_3084252558772637249[5] = 0;
   out_3084252558772637249[6] = 0;
   out_3084252558772637249[7] = 0;
   out_3084252558772637249[8] = 0;
   out_3084252558772637249[9] = 0;
   out_3084252558772637249[10] = 0;
   out_3084252558772637249[11] = 0;
   out_3084252558772637249[12] = 0;
   out_3084252558772637249[13] = 0;
   out_3084252558772637249[14] = 0;
   out_3084252558772637249[15] = 0;
   out_3084252558772637249[16] = 0;
   out_3084252558772637249[17] = 0;
   out_3084252558772637249[18] = 0;
   out_3084252558772637249[19] = 0;
   out_3084252558772637249[20] = 0;
   out_3084252558772637249[21] = 0;
   out_3084252558772637249[22] = 1;
   out_3084252558772637249[23] = 0;
   out_3084252558772637249[24] = 0;
   out_3084252558772637249[25] = 0;
   out_3084252558772637249[26] = 0;
   out_3084252558772637249[27] = 0;
   out_3084252558772637249[28] = 0;
   out_3084252558772637249[29] = 0;
   out_3084252558772637249[30] = 0;
   out_3084252558772637249[31] = 0;
   out_3084252558772637249[32] = 0;
   out_3084252558772637249[33] = 0;
   out_3084252558772637249[34] = 0;
   out_3084252558772637249[35] = 0;
   out_3084252558772637249[36] = 0;
   out_3084252558772637249[37] = 0;
   out_3084252558772637249[38] = 0;
   out_3084252558772637249[39] = 0;
   out_3084252558772637249[40] = 0;
   out_3084252558772637249[41] = 1;
   out_3084252558772637249[42] = 0;
   out_3084252558772637249[43] = 0;
   out_3084252558772637249[44] = 0;
   out_3084252558772637249[45] = 0;
   out_3084252558772637249[46] = 0;
   out_3084252558772637249[47] = 0;
   out_3084252558772637249[48] = 0;
   out_3084252558772637249[49] = 0;
   out_3084252558772637249[50] = 0;
   out_3084252558772637249[51] = 0;
   out_3084252558772637249[52] = 0;
   out_3084252558772637249[53] = 0;
}
void h_14(double *state, double *unused, double *out_4981368864544738461) {
   out_4981368864544738461[0] = state[6];
   out_4981368864544738461[1] = state[7];
   out_4981368864544738461[2] = state[8];
}
void H_14(double *state, double *unused, double *out_2333285527765485521) {
   out_2333285527765485521[0] = 0;
   out_2333285527765485521[1] = 0;
   out_2333285527765485521[2] = 0;
   out_2333285527765485521[3] = 0;
   out_2333285527765485521[4] = 0;
   out_2333285527765485521[5] = 0;
   out_2333285527765485521[6] = 1;
   out_2333285527765485521[7] = 0;
   out_2333285527765485521[8] = 0;
   out_2333285527765485521[9] = 0;
   out_2333285527765485521[10] = 0;
   out_2333285527765485521[11] = 0;
   out_2333285527765485521[12] = 0;
   out_2333285527765485521[13] = 0;
   out_2333285527765485521[14] = 0;
   out_2333285527765485521[15] = 0;
   out_2333285527765485521[16] = 0;
   out_2333285527765485521[17] = 0;
   out_2333285527765485521[18] = 0;
   out_2333285527765485521[19] = 0;
   out_2333285527765485521[20] = 0;
   out_2333285527765485521[21] = 0;
   out_2333285527765485521[22] = 0;
   out_2333285527765485521[23] = 0;
   out_2333285527765485521[24] = 0;
   out_2333285527765485521[25] = 1;
   out_2333285527765485521[26] = 0;
   out_2333285527765485521[27] = 0;
   out_2333285527765485521[28] = 0;
   out_2333285527765485521[29] = 0;
   out_2333285527765485521[30] = 0;
   out_2333285527765485521[31] = 0;
   out_2333285527765485521[32] = 0;
   out_2333285527765485521[33] = 0;
   out_2333285527765485521[34] = 0;
   out_2333285527765485521[35] = 0;
   out_2333285527765485521[36] = 0;
   out_2333285527765485521[37] = 0;
   out_2333285527765485521[38] = 0;
   out_2333285527765485521[39] = 0;
   out_2333285527765485521[40] = 0;
   out_2333285527765485521[41] = 0;
   out_2333285527765485521[42] = 0;
   out_2333285527765485521[43] = 0;
   out_2333285527765485521[44] = 1;
   out_2333285527765485521[45] = 0;
   out_2333285527765485521[46] = 0;
   out_2333285527765485521[47] = 0;
   out_2333285527765485521[48] = 0;
   out_2333285527765485521[49] = 0;
   out_2333285527765485521[50] = 0;
   out_2333285527765485521[51] = 0;
   out_2333285527765485521[52] = 0;
   out_2333285527765485521[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_227942031587100209) {
  err_fun(nom_x, delta_x, out_227942031587100209);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_6904622864079995273) {
  inv_err_fun(nom_x, true_x, out_6904622864079995273);
}
void pose_H_mod_fun(double *state, double *out_3022304305314020797) {
  H_mod_fun(state, out_3022304305314020797);
}
void pose_f_fun(double *state, double dt, double *out_5347359958206634853) {
  f_fun(state,  dt, out_5347359958206634853);
}
void pose_F_fun(double *state, double dt, double *out_2486298903049178646) {
  F_fun(state,  dt, out_2486298903049178646);
}
void pose_h_4(double *state, double *unused, double *out_4503598515101124135) {
  h_4(state, unused, out_4503598515101124135);
}
void pose_H_4(double *state, double *unused, double *out_6296526384104970050) {
  H_4(state, unused, out_6296526384104970050);
}
void pose_h_10(double *state, double *unused, double *out_660870634115408416) {
  h_10(state, unused, out_660870634115408416);
}
void pose_H_10(double *state, double *unused, double *out_2423980848173730331) {
  H_10(state, unused, out_2423980848173730331);
}
void pose_h_13(double *state, double *unused, double *out_4084654337287946880) {
  h_13(state, unused, out_4084654337287946880);
}
void pose_H_13(double *state, double *unused, double *out_3084252558772637249) {
  H_13(state, unused, out_3084252558772637249);
}
void pose_h_14(double *state, double *unused, double *out_4981368864544738461) {
  h_14(state, unused, out_4981368864544738461);
}
void pose_H_14(double *state, double *unused, double *out_2333285527765485521) {
  H_14(state, unused, out_2333285527765485521);
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
