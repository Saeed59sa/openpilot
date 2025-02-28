#pragma once

#include "safety_declarations.h"
#include "safety_hyundai_common.h"

#define HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(bus) \
  {0x1CF, bus, 8},  /* CRUISE_BUTTON */          \

#define HYUNDAI_CANFD_CRUISE_BUTTON_ALT_TX_MSGS(bus) \
  {0x1AA, bus, 16},  /* CRUISE_BUTTON ALT */         \

#define HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(a_can, e_can) \
  HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(e_can)                    \
  {0x50,  a_can, 16},  /* LKAS */                               \
  {0x2A4, a_can, 24},  /* CAM_0x2A4 */                          \

#define HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(a_can, e_can) \
  HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(e_can)                        \
  HYUNDAI_CANFD_CRUISE_BUTTON_ALT_TX_MSGS(e_can)                    \
  {0x110, a_can, 32},  /* LKAS_ALT */                               \
  {0x362, a_can, 32},  /* CAM_0x362 */                              \

#define HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(e_can) \
  {0x12A, e_can, 16},  /* LFA */                         \
  {0x1E0, e_can, 16},  /* LFAHDA_CLUSTER */              \

#define HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(e_can) \
  {0x1A0, e_can, 32},  /* SCC_CONTROL */                \

// *** Addresses checked in rx hook ***
// EV, ICE, HYBRID: ACCELERATOR (0x35), ACCELERATOR_BRAKE_ALT (0x100), ACCELERATOR_ALT (0x105)
#define HYUNDAI_CANFD_COMMON_RX_CHECKS(pt_bus)                                                                       \
  {.msg = {{0x35, (pt_bus), 32, .check_checksum = true, .max_counter = 0xffU, .frequency = 100U},                    \
           {0x100, (pt_bus), 32, .check_checksum = true, .max_counter = 0xffU, .frequency = 100U},                   \
           {0x105, (pt_bus), 32, .check_checksum = true, .max_counter = 0xffU, .frequency = 100U}}},                 \
  {.msg = {{0x175, (pt_bus), 24, .check_checksum = true, .max_counter = 0xffU, .frequency = 50U}, { 0 }, { 0 }}},    \
  {.msg = {{0xa0, (pt_bus), 24, .check_checksum = true, .max_counter = 0xffU, .frequency = 100U}, { 0 }, { 0 }}},    \
  {.msg = {{0xea, (pt_bus), 24, .check_checksum = true, .max_counter = 0xffU, .frequency = 100U}, { 0 }, { 0 }}},    \
  {.msg = {{0x1cf, (pt_bus), 8, .check_checksum = false, .max_counter = 0xfU, .frequency = 50U},                     \
           {0x1aa, (pt_bus), 16, .check_checksum = false, .max_counter = 0xffU, .frequency = 50U}, { 0 }}},          \
  {.msg = {{0x125, (pt_bus), 16, .check_checksum = false, .max_counter = 0xffU, .frequency = 100U}, { 0 }, { 0 }}},  \

// SCC_CONTROL (from ADAS unit or camera)
#define HYUNDAI_CANFD_SCC_ADDR_CHECK(scc_bus)                                                                       \
  {.msg = {{0x1a0, (scc_bus), 32, .check_checksum = true, .max_counter = 0xffU, .frequency = 50U}, { 0 }, { 0 }}},  \

bool hyundai_canfd_alt_buttons = false;
bool hyundai_canfd_lka_steering_alt = false;
bool hyundai_canfd_angle_steering = false;

// {   80,   81,   272,   282,   298,   352,   353,   354,   442,   485,   416,   437,   506,   474,   480,   490,   512,   676,   866,   837,  1402,   908,  1848,  1187,  1204,  203,   0 }
// { 0x50, 0x51, 0x110, 0x11A, 0x12A, 0x160, 0x161, 0x162, 0x1BA, 0x1E5, 0x1A0, 0x1B5, 0x1FA, 0x1DA, 0x1E0, 0x1EA, 0x200, 0x2A4, 0x362, 0x345, 0x57A, 0x38C, 0x738, 0x4A3, 0x4B4, 0xCB, 0x0 }
int canfd_tx_addr[32] = { 80, 81, 272, 282, 298, 352, 353, 354, 442, 485, 416, 437, 506, 474, 480, 490, 512, 676, 866, 837, 1402, 908, 1848, 1187, 1204, 203, 0, };
uint32_t canfd_tx_time[32] = { 0, };

int hyundai_canfd_get_lka_addr(void) {
  return hyundai_canfd_lka_steering_alt ? 0x110 : 0x50;
}

static uint8_t hyundai_canfd_get_counter(const CANPacket_t *to_push) {
  uint8_t ret = 0;
  if (GET_LEN(to_push) == 8U) {
    ret = GET_BYTE(to_push, 1) >> 4;
  } else {
    ret = GET_BYTE(to_push, 2);
  }
  return ret;
}

static uint32_t hyundai_canfd_get_checksum(const CANPacket_t *to_push) {
  uint32_t chksum = GET_BYTE(to_push, 0) | (GET_BYTE(to_push, 1) << 8);
  return chksum;
}

static void hyundai_canfd_rx_hook(const CANPacket_t *to_push) {
  int bus = GET_BUS(to_push);
  int addr = GET_ADDR(to_push);

  int pt_bus = (hyundai_canfd_lka_steering && !hyundai_camera_scc) ? 1 : 0;
  const int scc_bus = hyundai_camera_scc ? 2 : pt_bus;

  if (bus == pt_bus) {
    // driver torque
    if (addr == 0xea) {
      int torque_driver_new = ((GET_BYTE(to_push, 11) & 0x1fU) << 8U) | GET_BYTE(to_push, 10);
      torque_driver_new -= 4095;
      update_sample(&torque_driver, torque_driver_new);
    }

    // steering angle
    if (addr == 0x125) {
      int angle_meas_new = (GET_BYTE(to_push, 4) << 8) | GET_BYTE(to_push, 3);
      // Multiply by 10 to apply the DBC scaling factor of -0.1 for STEERING_ANGLE
      angle_meas_new = to_signed(angle_meas_new, 16);
      update_sample(&angle_meas, angle_meas_new);
    }

    // cruise buttons
    const int button_addr = hyundai_canfd_alt_buttons ? 0x1aa : 0x1cf;
    if (addr == button_addr) {
      bool main_button = false;
      int cruise_button = 0;
      if (addr == 0x1cf) {
        cruise_button = GET_BYTE(to_push, 2) & 0x7U;
        main_button = GET_BIT(to_push, 19U);
      } else {
        cruise_button = (GET_BYTE(to_push, 4) >> 4) & 0x7U;
        main_button = GET_BIT(to_push, 34U);
      }
      hyundai_common_cruise_buttons_check(cruise_button, main_button);
    }

    // gas press, different for EV, hybrid, and ICE models
    if ((addr == 0x35) && hyundai_ev_gas_signal) {
      gas_pressed = GET_BYTE(to_push, 5) != 0U;
    } else if ((addr == 0x105) && hyundai_hybrid_gas_signal) {
      gas_pressed = GET_BIT(to_push, 103U) || (GET_BYTE(to_push, 13) != 0U) || GET_BIT(to_push, 112U);
    } else if ((addr == 0x100) && !hyundai_ev_gas_signal && !hyundai_hybrid_gas_signal) {
      gas_pressed = GET_BIT(to_push, 176U);
    } else {
    }

    // brake press
    if (addr == 0x175) {
      brake_pressed = GET_BIT(to_push, 81U);
    }

    // vehicle moving
    if (addr == 0xa0) {
      const uint32_t fl = ((GET_BYTE(to_push, 9) & 0x3FU) << 8) | GET_BYTE(to_push, 8);
      const uint32_t fr = ((GET_BYTE(to_push, 11) & 0x3FU) << 8) | GET_BYTE(to_push, 10);
      const uint32_t rl = ((GET_BYTE(to_push, 13) & 0x3FU) << 8) | GET_BYTE(to_push, 12);
      const uint32_t rr = ((GET_BYTE(to_push, 15) & 0x3FU) << 8) | GET_BYTE(to_push, 14);

      vehicle_moving = (fl > HYUNDAI_STANDSTILL_THRSLD) || (fr > HYUNDAI_STANDSTILL_THRSLD) ||
                       (rl > HYUNDAI_STANDSTILL_THRSLD) || (rr > HYUNDAI_STANDSTILL_THRSLD);

      // average of all 4 wheel speeds. Conversion: raw * 0.03125 / 3.6 = m/s
      UPDATE_VEHICLE_SPEED((fr + rr + rl + fl) / 4. * 0.03125 / 3.6);
    }
  }

  gas_pressed = brake_pressed = false;

  if (bus == scc_bus) {
    // cruise state
    if ((addr == 0x1a0) && !hyundai_longitudinal) {
      // 1=enabled, 2=driver override
      int cruise_status = ((GET_BYTE(to_push, 8) >> 4) & 0x7U);
      bool cruise_engaged = (cruise_status == 1) || (cruise_status == 2);
      hyundai_common_cruise_state_check(cruise_engaged);
    }
  }

  const int steer_addr = hyundai_canfd_lka_steering ? hyundai_canfd_get_lka_addr() : 0x12a;
  bool stock_ecu_detected = (addr == steer_addr) && (bus == 0);
  if (hyundai_longitudinal) {
    // on LKA steering cars, ensure ADRV ECU is still knocked out
    // on others, ensure accel msg is blocked from camera
    stock_ecu_detected = stock_ecu_detected || ((addr == 0x1a0) && (bus == pt_bus));
  }
  generic_rx_checks(stock_ecu_detected);
}

static bool hyundai_canfd_tx_hook(const CANPacket_t *to_send) {
  const TorqueSteeringLimits HYUNDAI_CANFD_TORQUE_STEERING_LIMITS = {
    .max_steer = 512,
    .max_rt_delta = 112,
    .max_rt_interval = 250000,
    .max_rate_up = 10,
    .max_rate_down = 10,
    .driver_torque_allowance = 250,
    .driver_torque_multiplier = 2,
    .type = TorqueDriverLimited,

    // the EPS faults when the steering angle is above a certain threshold for too long. to prevent this,
    // we allow setting torque actuation bit to 0 while maintaining the requested torque value for two consecutive frames
    .min_valid_request_frames = 89,
    .max_invalid_request_frames = 2,
    .min_valid_request_rt_interval = 810000,  // 810ms; a ~10% buffer on cutting every 90 frames
    .has_steer_req_tolerance = true,
  };

  const AngleSteeringLimits HYUNDAI_CANFD_ANGLE_STEERING_LIMITS = {
    .max_angle = 1800,
    .angle_deg_to_can = 10,
    .angle_rate_up_lookup = {
      {5., 25., 25.},
      {0.3, 0.15, 0.15}
    },
    .angle_rate_down_lookup = {
      {5., 25., 25.},
      {0.36, 0.26, 0.26}
    },
  };

  bool tx = true;
  int addr = GET_ADDR(to_send);

  // steering
  const int steer_addr = (hyundai_canfd_lka_steering && !hyundai_longitudinal) ? hyundai_canfd_get_lka_addr() : 0x12a;
  if (addr == steer_addr) {
    if (hyundai_canfd_angle_steering) {
      const int lkas_angle_active = (GET_BYTE(to_send, 9) >> 4) & 0x3U;
      const bool steer_angle_req = lkas_angle_active != 1;

      int desired_angle = (GET_BYTE(to_send, 11) << 6) | (GET_BYTE(to_send, 10) >> 2);

      // Multiply by 10 to apply the DBC scaling factor of 0.1 for LKAS_ANGLE_CMD
      desired_angle = to_signed(desired_angle, 14);

      if (steer_angle_cmd_checks(desired_angle, steer_angle_req, HYUNDAI_CANFD_ANGLE_STEERING_LIMITS)) {
        tx = false;
      }
    } else {
      int desired_torque = (((GET_BYTE(to_send, 6) & 0xFU) << 7U) | (GET_BYTE(to_send, 5) >> 1U)) - 1024U;
      bool steer_req = GET_BIT(to_send, 52U);

      if (steer_torque_cmd_checks(desired_torque, steer_req, HYUNDAI_CANFD_TORQUE_STEERING_LIMITS)) {
        tx = false;
      }
    }
  }

  // cruise buttons check
  if (addr == 0x1cf) {
    int button = GET_BYTE(to_send, 2) & 0x7U;
    bool is_cancel = (button == HYUNDAI_BTN_CANCEL);
    bool is_resume = (button == HYUNDAI_BTN_RESUME);
    bool is_set = (button == HYUNDAI_BTN_SET);

    bool allowed = (is_cancel && cruise_engaged_prev) || (is_resume && controls_allowed) || (is_set && controls_allowed);
    if (!allowed) {
      tx = false;
    }
  }

  // UDS: only tester present ("\x02\x3E\x80\x00\x00\x00\x00\x00") allowed on diagnostics address
  if (((addr == 0x730) && hyundai_canfd_lka_steering) || ((addr == 0x7D0) && !hyundai_camera_scc)) {
    if ((GET_BYTES(to_send, 0, 4) != 0x00803E02U) || (GET_BYTES(to_send, 4, 4) != 0x0U)) {
      tx = false;
    }
  }

  // ACCEL: safety check
  if (addr == 0x1a0) {
    int desired_accel_raw = (((GET_BYTE(to_send, 17) & 0x7U) << 8) | GET_BYTE(to_send, 16)) - 1023U;
    int desired_accel_val = ((GET_BYTE(to_send, 18) << 4) | (GET_BYTE(to_send, 17) >> 4)) - 1023U;

    bool violation = false;

    if (hyundai_longitudinal) {
      violation |= longitudinal_accel_checks(desired_accel_raw, HYUNDAI_LONG_LIMITS);
      violation |= longitudinal_accel_checks(desired_accel_val, HYUNDAI_LONG_LIMITS);
    } else {
      // only used to cancel on here
      if ((desired_accel_raw != 0) || (desired_accel_val != 0)) {
        violation = true;
      }
    }

    if (violation) {
      tx = false;
    }
  }

  for (int i = 0; canfd_tx_addr[i] > 0; i++) {
    if (addr == canfd_tx_addr[i]) canfd_tx_time[i] = (tx) ? microsecond_timer_get() : 0;
  }

  return tx;
}

#define MAX_ADDR_LIST_SIZE 128
#define OP_CAN_SEND_TIMEOUT 100000

typedef struct {
  int addrs[MAX_ADDR_LIST_SIZE];
  int count;
} AddrList;

static bool add_addr_to_list(AddrList *list, int addr) {
  if (list->count >= MAX_ADDR_LIST_SIZE) {
    return false; // List is full
  }

  for (int i = 0; i < list->count; i++) {
    if (list->addrs[i] == addr) {
      return false; // Already exists
    }
  }

  list->addrs[list->count++] = addr;
  return true;
}

static void print_addr_list(const char *prefix, const AddrList *list) {
  print(prefix);
  for (int j = 0; j < list->count; j++) {
    putui((uint32_t)list->addrs[j]);
    print(",");
  }
  print("\n");
}

static int hyundai_canfd_fwd_hook(int bus_num, int addr) {
  int bus_fwd = -1;
  uint32_t now = microsecond_timer_get();

  static AddrList addr_list1 = {{0}, 0};
  static AddrList addr_list2 = {{0}, 0};

  switch (bus_num) {
    case 0:
      bus_fwd = 2;
      break;
    case 1:
      if (add_addr_to_list(&addr_list1, addr)) {
        print_addr_list("!!!!! bus1_list=", &addr_list1);
      }
      break;
    case 2:
      if (add_addr_to_list(&addr_list2, addr)) {
        print_addr_list("@@@@ bus2_list=", &addr_list2);
      }
      bus_fwd = 0;
      for (int i = 0; canfd_tx_addr[i] > 0; i++) {
        if (addr == canfd_tx_addr[i] && (now - canfd_tx_time[i]) < OP_CAN_SEND_TIMEOUT) {
          bus_fwd = -1;
          break;
        }
      }
      break;
  }

  return bus_fwd;
}

/*
    // LKAS for cars with LKAS and LFA messages, LFA for cars with no LKAS messages
    int lfa_block_addr = hyundai_canfd_lka_steering_alt ? 0x362 : 0x2a4;
    bool is_lka_msg = ((addr == hyundai_canfd_get_lka_addr()) || (addr == lfa_block_addr)) && hyundai_canfd_lka_steering;
    bool is_lfa_msg = ((addr == 0x12a) && !hyundai_canfd_lka_steering);

    // HUD icons
    bool is_lfahda_msg = ((addr == 0x1e0) && !hyundai_canfd_lka_steering);

    // SCC_CONTROL and ADRV_0x160 for camera SCC cars, we send our own longitudinal commands and to show FCA light
    bool is_scc_msg = (((addr == 0x1a0) || (addr == 0x160)) && hyundai_longitudinal && !hyundai_canfd_lka_steering);

    bool block_msg = is_lka_msg || is_lfa_msg || is_lfahda_msg || is_scc_msg;
    if (!block_msg) {
      bus_fwd = 0;
    }
*/

static safety_config hyundai_canfd_init(uint16_t param) {
  const int HYUNDAI_PARAM_CANFD_LKA_STEERING_ALT = 128;
  const int HYUNDAI_PARAM_CANFD_ALT_BUTTONS = 32;
  const int HYUNDAI_PARAM_CANFD_ANGLE_STEERING = 256;

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(0, 1)
  };

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_ALT_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(0, 1)
  };

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_LONG_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(0, 1)
    HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(1, 1)
    HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(1, 1)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(1)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(1)
    {0x51,  0, 32},  // ADRV_0x51
    {0x51,  1, 32},  // ADRV_0x51
    {0x730, 1,  8},  // tester present for ADAS ECU disable
    {0x160, 1, 16},  // ADRV_0x160
    {0x160, 1, 16},  // ADRV_0x160
    {0x161, 0, 32},  // CCNC_0x161
    {0x162, 0, 32},  // CCNC_0x162
    {0x1EA, 1, 32},  // ADRV_0x1ea
    {0x1EA, 1, 32},  // ADRV_0x1ea
    {0x200, 1,  8},  // ADRV_0x200
    {0x200, 1,  8},  // ADRV_0x200
    {0x345, 1,  8},  // ADRV_0x345
    {0x345, 1,  8},  // ADRV_0x345
    {0x1DA, 1, 32},  // ADRV_0x1da
    {0xCB,  0, 24},  // CB
    {0x4A3, 2,  8},  // 4A3
    {0x4B4, 2,  8},  // 4B4
  };

  static const CanMsg HYUNDAI_CANFD_LFA_STEERING_TX_MSGS[] = {
    HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(2)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0)
  };

  static const CanMsg HYUNDAI_CANFD_LFA_STEERING_LONG_TX_MSGS[] = {
    HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(2)
    HYUNDAI_CANFD_CRUISE_BUTTON_ALT_TX_MSGS(2)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0)
    {0x160, 0, 16}, // ADRV_0x160
    {0x7D0, 0, 8},  // tester present for radar ECU disable
    {203, 0, 24},   // CB
  };

  static const CanMsg HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_TX_MSGS[] = {
    HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(2)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0)
    {0x160, 0, 16}, // ADRV_0x160
  };

  hyundai_common_init(param);

  gen_crc_lookup_table_16(0x1021, hyundai_canfd_crc_lut);
  hyundai_canfd_alt_buttons = GET_FLAG(param, HYUNDAI_PARAM_CANFD_ALT_BUTTONS);
  hyundai_canfd_lka_steering_alt = GET_FLAG(param, HYUNDAI_PARAM_CANFD_LKA_STEERING_ALT);
  hyundai_canfd_angle_steering = GET_FLAG(param, HYUNDAI_PARAM_CANFD_ANGLE_STEERING);

  safety_config ret;
  if (hyundai_longitudinal) {
    if (hyundai_canfd_lka_steering) {
      static RxCheck hyundai_canfd_lka_steering_long_rx_checks_camera_scc[] = {
        HYUNDAI_CANFD_COMMON_RX_CHECKS(0)
      };
      static RxCheck hyundai_canfd_lka_steering_long_rx_checks[] = {
        HYUNDAI_CANFD_COMMON_RX_CHECKS(1)
      };

      ret = hyundai_camera_scc ?
        BUILD_SAFETY_CFG(hyundai_canfd_lka_steering_long_rx_checks_camera_scc, HYUNDAI_CANFD_LKA_STEERING_LONG_TX_MSGS) : \
        BUILD_SAFETY_CFG(hyundai_canfd_lka_steering_long_rx_checks, HYUNDAI_CANFD_LKA_STEERING_LONG_TX_MSGS);
    } else {
      // Longitudinal checks for LFA steering
      static RxCheck hyundai_canfd_long_rx_checks[] = {
        HYUNDAI_CANFD_COMMON_RX_CHECKS(0)
      };

      ret = hyundai_camera_scc ?
        BUILD_SAFETY_CFG(hyundai_canfd_long_rx_checks, HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_TX_MSGS) : \
        BUILD_SAFETY_CFG(hyundai_canfd_long_rx_checks, HYUNDAI_CANFD_LFA_STEERING_LONG_TX_MSGS);
    }
  } else {
    if (hyundai_canfd_lka_steering) {
      // *** LKA steering checks ***
      // E-CAN is on bus 1, SCC messages are sent on cars with ADRV ECU.
      // Does not use the alt buttons message
      static RxCheck hyundai_canfd_lka_steering_rx_checks[] = {
        HYUNDAI_CANFD_COMMON_RX_CHECKS(1)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(1)
      };

      ret = hyundai_canfd_lka_steering_alt ?
        BUILD_SAFETY_CFG(hyundai_canfd_lka_steering_rx_checks, HYUNDAI_CANFD_LKA_STEERING_ALT_TX_MSGS) : \
        BUILD_SAFETY_CFG(hyundai_canfd_lka_steering_rx_checks, HYUNDAI_CANFD_LKA_STEERING_TX_MSGS);
    } else if (!hyundai_camera_scc) {
      // Radar sends SCC messages on these cars instead of camera
      static RxCheck hyundai_canfd_radar_scc_rx_checks[] = {
        HYUNDAI_CANFD_COMMON_RX_CHECKS(0)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(0)
      };

      ret = BUILD_SAFETY_CFG(hyundai_canfd_radar_scc_rx_checks, HYUNDAI_CANFD_LFA_STEERING_TX_MSGS);
    } else {
      // *** LFA steering checks ***
      // Camera sends SCC messages on LFA steering cars.
      // Both button messages exist on some platforms, so we ensure we track the correct one using flag
      static RxCheck hyundai_canfd_rx_checks[] = {
        HYUNDAI_CANFD_COMMON_RX_CHECKS(0)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(2)
      };

      ret = BUILD_SAFETY_CFG(hyundai_canfd_rx_checks, HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_TX_MSGS);
    }
  }

  return ret;
}

const safety_hooks hyundai_canfd_hooks = {
  .init = hyundai_canfd_init,
  .rx = hyundai_canfd_rx_hook,
  .tx = hyundai_canfd_tx_hook,
  .fwd = hyundai_canfd_fwd_hook,
  .get_counter = hyundai_canfd_get_counter,
  .get_checksum = hyundai_canfd_get_checksum,
  .compute_checksum = hyundai_common_canfd_compute_checksum,
};
