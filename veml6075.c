/**
 * VEML6075 UVA/UVB/UV Index Sensor Library for Raspberry Pi Pico 2
 * Converted from SparkFun Arduino Library
 */

#include "veml6075.h"
#include <string.h>

// Constants
#define VEML6075_REGISTER_LENGTH 2
#define NUM_INTEGRATION_TIMES 5

#define VEML6075_UV_IT_MASK 0x70
#define VEML6075_UV_IT_SHIFT 4
#define VEML6075_SHUTDOWN_MASK 0x01
#define VEML6075_SHUTDOWN_SHIFT 0
#define VEML6075_HD_MASK 0x08
#define VEML6075_HD_SHIFT 3
#define VEML6075_TRIG_MASK 0x04
#define VEML6075_TRIG_SHIFT 2
#define VEML6075_AF_MASK 0x02
#define VEML6075_AF_SHIFT 1

// Calculation constants
static const float HD_SCALAR = 2.0f;
static const float UV_ALPHA = 1.0f;
static const float UV_BETA = 1.0f;
static const float UV_GAMMA = 1.0f;
static const float UV_DELTA = 1.0f;

static const float UVA_A_COEF = 2.22f;
static const float UVA_B_COEF = 1.33f;
static const float UVA_C_COEF = 2.95f;
static const float UVA_D_COEF = 1.75f;

static const float UVA_RESPONSIVITY_100MS_UNCOVERED = 0.001111f;
static const float UVB_RESPONSIVITY_100MS_UNCOVERED = 0.00125f;

static const float UVA_RESPONSIVITY[NUM_INTEGRATION_TIMES] = {
    UVA_RESPONSIVITY_100MS_UNCOVERED / 0.5016286645f, // 50ms
    UVA_RESPONSIVITY_100MS_UNCOVERED,                 // 100ms
    UVA_RESPONSIVITY_100MS_UNCOVERED / 2.039087948f,  // 200ms
    UVA_RESPONSIVITY_100MS_UNCOVERED / 3.781758958f,  // 400ms
    UVA_RESPONSIVITY_100MS_UNCOVERED / 7.371335505f   // 800ms
};

static const float UVB_RESPONSIVITY[NUM_INTEGRATION_TIMES] = {
    UVB_RESPONSIVITY_100MS_UNCOVERED / 0.5016286645f, // 50ms
    UVB_RESPONSIVITY_100MS_UNCOVERED,                 // 100ms
    UVB_RESPONSIVITY_100MS_UNCOVERED / 2.039087948f,  // 200ms
    UVB_RESPONSIVITY_100MS_UNCOVERED / 3.781758958f,  // 400ms
    UVB_RESPONSIVITY_100MS_UNCOVERED / 7.371335505f   // 800ms
};

// Private helper functions
static VEML6075_error_t read_i2c_buffer(VEML6075_t *dev, uint8_t *dest, 
                                        VEML6075_REGISTER_t start_reg, uint16_t len) {
    if (dev->device_address != VEML6075_ADDRESS) {
        return VEML6075_ERROR_INVALID_ADDRESS;
    }
    
    int ret = i2c_write_blocking(dev->i2c, dev->device_address, 
                                 (uint8_t*)&start_reg, 1, true);
    if (ret < 0) {
        return VEML6075_ERROR_READ;
    }
    
    ret = i2c_read_blocking(dev->i2c, dev->device_address, dest, len, false);
    if (ret < 0) {
        return VEML6075_ERROR_READ;
    }
    
    return VEML6075_ERROR_SUCCESS;
}

static VEML6075_error_t write_i2c_buffer(VEML6075_t *dev, uint8_t *src, 
                                         VEML6075_REGISTER_t start_reg, uint16_t len) {
    if (dev->device_address != VEML6075_ADDRESS) {
        return VEML6075_ERROR_INVALID_ADDRESS;
    }
    
    uint8_t buffer[len + 1];
    buffer[0] = start_reg;
    memcpy(buffer + 1, src, len);
    
    int ret = i2c_write_blocking(dev->i2c, dev->device_address, buffer, len + 1, false);
    if (ret < 0) {
        return VEML6075_ERROR_WRITE;
    }
    
    return VEML6075_ERROR_SUCCESS;
}

static VEML6075_error_t read_i2c_register(VEML6075_t *dev, uint16_t *dest, 
                                          VEML6075_REGISTER_t reg_addr) {
    uint8_t temp_dest[2];
    VEML6075_error_t err = read_i2c_buffer(dev, temp_dest, reg_addr, VEML6075_REGISTER_LENGTH);
    if (err == VEML6075_ERROR_SUCCESS) {
        *dest = temp_dest[0] | ((uint16_t)temp_dest[1] << 8);
    }
    return err;
}

static VEML6075_error_t write_i2c_register(VEML6075_t *dev, uint16_t data, 
                                           VEML6075_REGISTER_t reg_addr) {
    uint8_t d[2];
    d[0] = (uint8_t)(data & 0x00FF);
    d[1] = (uint8_t)((data & 0xFF00) >> 8);
    return write_i2c_buffer(dev, d, reg_addr, VEML6075_REGISTER_LENGTH);
}

static VEML6075_error_t check_connected(VEML6075_t *dev) {
    uint8_t id;
    VEML6075_error_t err = veml6075_get_device_id(dev, &id);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    if (id != VEML6075_DEVICE_ID) {
        return VEML6075_ERROR_INVALID_ADDRESS;
    }
    return VEML6075_ERROR_SUCCESS;
}

// Public API implementation
VEML6075_error_t veml6075_init(VEML6075_t *dev, i2c_inst_t *i2c) {
    dev->i2c = i2c;
    dev->device_address = VEML6075_ADDRESS;
    dev->last_read_time = 0;
    dev->integration_time = 0;
    dev->last_index = 0.0f;
    dev->a_responsivity = UVA_RESPONSIVITY_100MS_UNCOVERED;
    dev->b_responsivity = UVB_RESPONSIVITY_100MS_UNCOVERED;
    dev->hd_enabled = false;
    dev->last_uva = 0;
    dev->last_uvb = 0;
    
    VEML6075_error_t err = check_connected(dev);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    
    if (veml6075_power_on(dev, true) == VEML6075_ERROR_SUCCESS) {
        veml6075_set_integration_time(dev, IT_100MS);
        veml6075_set_high_dynamic(dev, DYNAMIC_NORMAL);
        veml6075_set_auto_force(dev, AF_DISABLE);
    }
    
    return VEML6075_ERROR_SUCCESS;
}

bool veml6075_is_connected(VEML6075_t *dev) {
    return (check_connected(dev) == VEML6075_ERROR_SUCCESS);
}

VEML6075_error_t veml6075_set_integration_time(VEML6075_t *dev, veml6075_uv_it_t it) {
    if (it >= IT_RESERVED_0) {
        return VEML6075_ERROR_UNDEFINED;
    }
    
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    
    conf &= ~(VEML6075_UV_IT_MASK);
    conf |= (it << VEML6075_UV_IT_SHIFT);
    err = write_i2c_register(dev, conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    
    dev->a_responsivity = UVA_RESPONSIVITY[(uint8_t)it];
    dev->b_responsivity = UVB_RESPONSIVITY[(uint8_t)it];
    
    switch (it) {
        case IT_50MS:  dev->integration_time = 50; break;
        case IT_100MS: dev->integration_time = 100; break;
        case IT_200MS: dev->integration_time = 200; break;
        case IT_400MS: dev->integration_time = 400; break;
        case IT_800MS: dev->integration_time = 800; break;
        default: dev->integration_time = 0; break;
    }
    
    return err;
}

veml6075_uv_it_t veml6075_get_integration_time(VEML6075_t *dev) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return IT_INVALID;
    }
    return (veml6075_uv_it_t)((conf & VEML6075_UV_IT_MASK) >> VEML6075_UV_IT_SHIFT);
}

VEML6075_error_t veml6075_set_high_dynamic(VEML6075_t *dev, veml6075_hd_t hd) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    
    dev->hd_enabled = (hd == DYNAMIC_HIGH);
    conf &= ~(VEML6075_HD_MASK);
    conf |= (hd << VEML6075_HD_SHIFT);
    return write_i2c_register(dev, conf, REG_UV_CONF);
}

veml6075_hd_t veml6075_get_high_dynamic(VEML6075_t *dev) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return HD_INVALID;
    }
    return (veml6075_hd_t)((conf & VEML6075_HD_MASK) >> VEML6075_HD_SHIFT);
}

VEML6075_error_t veml6075_set_trigger(VEML6075_t *dev, veml6075_uv_trig_t trig) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    
    conf &= ~(VEML6075_TRIG_MASK);
    conf |= (trig << VEML6075_TRIG_SHIFT);
    return write_i2c_register(dev, conf, REG_UV_CONF);
}

veml6075_uv_trig_t veml6075_get_trigger(VEML6075_t *dev) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return TRIGGER_INVALID;
    }
    return (veml6075_uv_trig_t)((conf & VEML6075_TRIG_MASK) >> VEML6075_TRIG_SHIFT);
}

VEML6075_error_t veml6075_set_auto_force(VEML6075_t *dev, veml6075_af_t af) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    
    conf &= ~(VEML6075_AF_MASK);
    conf |= (af << VEML6075_AF_SHIFT);
    return write_i2c_register(dev, conf, REG_UV_CONF);
}

veml6075_af_t veml6075_get_auto_force(VEML6075_t *dev) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return AF_INVALID;
    }
    return (veml6075_af_t)((conf & VEML6075_AF_MASK) >> VEML6075_AF_SHIFT);
}

VEML6075_error_t veml6075_power_on(VEML6075_t *dev, bool enable) {
    return veml6075_shutdown(dev, !enable);
}

VEML6075_error_t veml6075_shutdown(VEML6075_t *dev, bool shutdown) {
    uint16_t conf;
    VEML6075_error_t err = read_i2c_register(dev, &conf, REG_UV_CONF);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    
    VEML6075_shutdown_t sd = shutdown ? SHUT_DOWN : POWER_ON;
    conf &= ~(VEML6075_SHUTDOWN_MASK);
    conf |= sd << VEML6075_SHUTDOWN_SHIFT;
    return write_i2c_register(dev, conf, REG_UV_CONF);
}

VEML6075_error_t veml6075_trigger(VEML6075_t *dev) {
    return veml6075_set_trigger(dev, TRIGGER_ONE_OR_UV_TRIG);
}

uint16_t veml6075_get_raw_uva(VEML6075_t *dev) {
    uint8_t uva[2] = {0, 0};
    VEML6075_error_t err = read_i2c_buffer(dev, uva, REG_UVA_DATA, 2);
    if (err != VEML6075_ERROR_SUCCESS) {
        return 0;
    }
    dev->last_read_time = to_ms_since_boot(get_absolute_time());
    dev->last_uva = (uva[0] & 0x00FF) | ((uva[1] & 0x00FF) << 8);
    return dev->last_uva;
}

uint16_t veml6075_get_raw_uvb(VEML6075_t *dev) {
    uint8_t uvb[2] = {0, 0};
    VEML6075_error_t err = read_i2c_buffer(dev, uvb, REG_UVB_DATA, 2);
    if (err != VEML6075_ERROR_SUCCESS) {
        return 0;
    }
    dev->last_read_time = to_ms_since_boot(get_absolute_time());
    dev->last_uvb = (uvb[0] & 0x00FF) | ((uvb[1] & 0x00FF) << 8);
    return dev->last_uvb;
}

uint16_t veml6075_get_uv_comp1(VEML6075_t *dev) {
    uint8_t uvcomp1[2] = {0, 0};
    VEML6075_error_t err = read_i2c_buffer(dev, uvcomp1, REG_UVCOMP1_DATA, 2);
    if (err != VEML6075_ERROR_SUCCESS) {
        return 0;
    }
    return (uvcomp1[0] & 0x00FF) | ((uvcomp1[1] & 0x00FF) << 8);
}

uint16_t veml6075_get_uv_comp2(VEML6075_t *dev) {
    uint8_t uvcomp2[2] = {0, 0};
    VEML6075_error_t err = read_i2c_buffer(dev, uvcomp2, REG_UVCOMP2_DATA, 2);
    if (err != VEML6075_ERROR_SUCCESS) {
        return 0;
    }
    return (uvcomp2[0] & 0x00FF) | ((uvcomp2[1] & 0x00FF) << 8);
}

float veml6075_get_uva(VEML6075_t *dev) {
    float raw_uva = (float)veml6075_get_raw_uva(dev);
    float comp1 = (float)veml6075_get_uv_comp1(dev);
    float comp2 = (float)veml6075_get_uv_comp2(dev);
    
    return raw_uva - ((UVA_A_COEF * UV_ALPHA * comp1) / UV_GAMMA) - 
           ((UVA_B_COEF * UV_ALPHA * comp2) / UV_DELTA);
}

float veml6075_get_uvb(VEML6075_t *dev) {
    float raw_uvb = (float)veml6075_get_raw_uvb(dev);
    float comp1 = (float)veml6075_get_uv_comp1(dev);
    float comp2 = (float)veml6075_get_uv_comp2(dev);
    
    return raw_uvb - ((UVA_C_COEF * UV_BETA * comp1) / UV_GAMMA) - 
           ((UVA_D_COEF * UV_BETA * comp2) / UV_DELTA);
}

float veml6075_get_index(VEML6075_t *dev) {
    float uva_calc = veml6075_get_uva(dev);
    float uvb_calc = veml6075_get_uvb(dev);
    
    // float uvia = uva_calc * (1.0f / UV_ALPHA) * dev->a_responsivity;
    // float uvib = uvb_calc * (1.0f / UV_BETA) * dev->b_responsivity;
    float uvia = uva_calc * 0.001111f;
    float uvib = uvb_calc  * 0.00125f;
    dev->last_index = (uvia + uvib) / 2.0f;
    
    if (dev->hd_enabled) {
        dev->last_index *= HD_SCALAR;
    }
    
    dev->last_read_time = to_ms_since_boot(get_absolute_time());
    return dev->last_index;
}

uint16_t veml6075_get_visible_compensation(VEML6075_t *dev) {
    return veml6075_get_uv_comp1(dev);
}

uint16_t veml6075_get_ir_compensation(VEML6075_t *dev) {
    return veml6075_get_uv_comp2(dev);
}

VEML6075_error_t veml6075_get_device_id(VEML6075_t *dev, uint8_t *id) {
    uint16_t dev_id = 0;
    VEML6075_error_t err = read_i2c_register(dev, &dev_id, REG_ID);
    if (err != VEML6075_ERROR_SUCCESS) {
        return err;
    }
    *id = (uint8_t)(dev_id & 0x00FF);
    return err;
}