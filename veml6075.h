/**
 * VEML6075 UVA/UVB/UV Index Sensor Library for Raspberry Pi Pico 2
 * Converted from SparkFun Arduino Library
 */

#ifndef VEML6075_H
#define VEML6075_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdbool.h>
#include <stdint.h>

// I2C Address
#define VEML6075_ADDRESS 0x10
#define VEML6075_DEVICE_ID 0x26

// Register addresses
typedef enum {
    REG_UV_CONF = 0x00,
    REG_UVA_DATA = 0x07,
    REG_UVB_DATA = 0x09,
    REG_UVCOMP1_DATA = 0x0A,
    REG_UVCOMP2_DATA = 0x0B,
    REG_ID = 0x0C
} VEML6075_REGISTER_t;

// Error codes
typedef enum {
    VEML6075_ERROR_SUCCESS = 0,
    VEML6075_ERROR_UNDEFINED = -1,
    VEML6075_ERROR_INVALID_ADDRESS = -2,
    VEML6075_ERROR_READ = -3,
    VEML6075_ERROR_WRITE = -4
} VEML6075_error_t;

// Integration time options
typedef enum {
    IT_50MS = 0x00,
    IT_100MS = 0x01,
    IT_200MS = 0x02,
    IT_400MS = 0x03,
    IT_800MS = 0x04,
    IT_RESERVED_0 = 0x05,
    IT_INVALID = 0xFF
} veml6075_uv_it_t;

// High dynamic range options
typedef enum {
    DYNAMIC_NORMAL = 0x00,
    DYNAMIC_HIGH = 0x01,
    HD_INVALID = 0xFF
} veml6075_hd_t;

// Trigger options
typedef enum {
    NO_TRIGGER = 0x00,
    TRIGGER_ONE_OR_UV_TRIG = 0x01,
    TRIGGER_INVALID = 0xFF
} veml6075_uv_trig_t;

// Auto force mode options
typedef enum {
    AF_DISABLE = 0x00,
    AF_ENABLE = 0x01,
    AF_INVALID = 0xFF
} veml6075_af_t;

// Shutdown options
typedef enum {
    POWER_ON = 0x00,
    SHUT_DOWN = 0x01
} VEML6075_shutdown_t;

// VEML6075 device structure
typedef struct {
    i2c_inst_t *i2c;
    uint8_t device_address;
    uint32_t last_read_time;
    uint16_t integration_time;
    float last_index;
    float a_responsivity;
    float b_responsivity;
    bool hd_enabled;
    uint16_t last_uva;
    uint16_t last_uvb;
} VEML6075_t;

// Function prototypes
VEML6075_error_t veml6075_init(VEML6075_t *dev, i2c_inst_t *i2c);
bool veml6075_is_connected(VEML6075_t *dev);

VEML6075_error_t veml6075_set_integration_time(VEML6075_t *dev, veml6075_uv_it_t it);
veml6075_uv_it_t veml6075_get_integration_time(VEML6075_t *dev);

VEML6075_error_t veml6075_set_high_dynamic(VEML6075_t *dev, veml6075_hd_t hd);
veml6075_hd_t veml6075_get_high_dynamic(VEML6075_t *dev);

VEML6075_error_t veml6075_set_trigger(VEML6075_t *dev, veml6075_uv_trig_t trig);
veml6075_uv_trig_t veml6075_get_trigger(VEML6075_t *dev);

VEML6075_error_t veml6075_set_auto_force(VEML6075_t *dev, veml6075_af_t af);
veml6075_af_t veml6075_get_auto_force(VEML6075_t *dev);

VEML6075_error_t veml6075_power_on(VEML6075_t *dev, bool enable);
VEML6075_error_t veml6075_shutdown(VEML6075_t *dev, bool shutdown);
VEML6075_error_t veml6075_trigger(VEML6075_t *dev);

float veml6075_get_uva(VEML6075_t *dev);
float veml6075_get_uvb(VEML6075_t *dev);
float veml6075_get_index(VEML6075_t *dev);

uint16_t veml6075_get_raw_uva(VEML6075_t *dev);
uint16_t veml6075_get_raw_uvb(VEML6075_t *dev);
uint16_t veml6075_get_uv_comp1(VEML6075_t *dev);
uint16_t veml6075_get_uv_comp2(VEML6075_t *dev);
uint16_t veml6075_get_visible_compensation(VEML6075_t *dev);
uint16_t veml6075_get_ir_compensation(VEML6075_t *dev);

VEML6075_error_t veml6075_get_device_id(VEML6075_t *dev, uint8_t *id);

#endif // VEML6075_H