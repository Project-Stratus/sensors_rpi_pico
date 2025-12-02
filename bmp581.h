#ifndef BMP581_H
#define BMP581_H
#include "hardware/i2c.h"
#include <stdint.h>
#include <limits.h>
#include <stdio.h>


#define BMP581_ENABLE_DECODE_PRESSF 0
#define BMP581_NUM_PRESS_DATA_REGS 3

#define BMP581_STR_IMPL(X) #X
#define BMP581_STR(X) BMP581_STR_IMPL(X)
#define BMP581_PRESSURE_DP_POW 1000000ul
#define BMP581_PRESSURE_DP 6
#define BMP581_PRESSURE_DP_STR BMP581_STR(BMP581_PRESSURE_DP)
struct bmp581_pressure_t {
    long nat;
    long frac;
};
static_assert(sizeof(long) > BMP581_NUM_PRESS_DATA_REGS);
static_assert(ULONG_MAX > 10ull * BMP581_PRESSURE_DP_POW);

enum bmp581_err_t {
    bmp581_err_ok,
    bmp581_err_chip_id_set_addr_nack,
    bmp581_err_chip_id_set_mismatch,
    bmp581_err_chip_id_read_addr_nack,
    bmp581_err_chip_id_read_mismatch,
    bmp581_err_zero_chipid,
    bmp581_err_statuses_set_addr_nack,
    bmp581_err_statuses_set_mismatch,
    bmp581_err_statuses_read_addr_nack,
    bmp581_err_statuses_read_mismatch,
    bmp581_err_nvm_not_rdy,
    bmp581_err_nvm_err_and_nvm_not_rdy,
    bmp581_err_nvm_err,
    bmp581_err_zero_por,
    bmp581_err_configs_write_addr_nack,
    bmp581_err_configs_write_mismatch,
    bmp581_err_osr_config_set_addr_nack,
    bmp581_err_osr_config_set_mismatch,
    bmp581_err_configs_read_addr_nack,
    bmp581_err_configs_read_mismatch,
    bmp581_err_opposing_configs_read,
    bmp581_err_opposing_osr_config_read,
    bmp581_err_opposing_odr_config_read,
    bmp581_err_press_data_set_addr_nack,
    bmp581_err_press_data_set_mismath,
    bmp581_err_press_data_int_status_read_addr_nack,
    bmp581_err_press_data_int_status_read_mismatch,
    bmp581_err_por,
    bmp581_err_cmd_write_addr_nack,
    bmp581_err_cmd_write_mismatch,
    bmp581_err_int_source_set_addr_nack,
    bmp581_err_int_source_set_mismatch,
    bmp581_err_int_source_read_addr_nack,
    bmp581_err_int_source_read_mismatch,
    bmp581_err_int_source_write_addr_nack,
    bmp581_err_int_source_write_mismatch,
    bmp581_err_opposing_int_source_read,
    bmp581_err_int_status_set_addr_nack,
    bmp581_err_int_status_set_mismatch,
    bmp581_err_int_status_read_addr_nack,
    bmp581_err_int_status_read_mismatch,
    bmp581_max_err_val,
    bmp581_err_drdy_timeout
};

enum bmp581_osr_t_t {
    bmp581_osr_t_1x   = 0b000,
    bmp581_osr_t_2x   = 0b001,
    bmp581_osr_t_4x   = 0b010,
    bmp581_osr_t_8x   = 0b011,
    bmp581_osr_t_16x  = 0b100,
    bmp581_osr_t_32x  = 0b101,
    bmp581_osr_t_64x  = 0b110,
    bmp581_osr_t_128x = 0b111
};

enum bmp581_osr_p_t {
    bmp581_osr_p_1x   = bmp581_osr_t_1x   << 3,
    bmp581_osr_p_2x   = bmp581_osr_t_2x   << 3,
    bmp581_osr_p_4x   = bmp581_osr_t_4x   << 3,
    bmp581_osr_p_8x   = bmp581_osr_t_8x   << 3,
    bmp581_osr_p_16x  = bmp581_osr_t_16x  << 3,
    bmp581_osr_p_32x  = bmp581_osr_t_32x  << 3,
    bmp581_osr_p_64x  = bmp581_osr_t_64x  << 3,
    bmp581_osr_p_128x = bmp581_osr_t_128x << 3
};

typedef long bmp581_press_t;

typedef int8_t bmp581_eerr_t;
static_assert(bmp581_max_err_val <= 2 << sizeof(bmp581_eerr_t) * 8);

extern enum bmp581_err_t bmp581_init(
    i2c_inst_t* i2c, 
    enum bmp581_osr_t_t osr_t, 
    enum bmp581_osr_p_t osr_p
);

extern bmp581_eerr_t bmp581_read_press_handle_por(
    i2c_inst_t* i2c, 
    long* o_press,
    enum bmp581_osr_t_t osr_t, 
    enum bmp581_osr_p_t osr_p
);

extern struct bmp581_pressure_t bmp581_decode_press(bmp581_press_t press);

#if BMP581_ENABLE_DECODE_PRESSF
extern float bmp581_decode_pressf(bmp581_press_t press);
#endif

extern enum bmp581_err_t bmp581_soft_reset(i2c_inst_t * i2c);

#endif