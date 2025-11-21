/**
 * Example usage of VEML6075 library for Raspberry Pi Pico 2
 * 
 * Wiring:
 * - VEML6075 SDA -> Pico GPIO 4 (I2C0 SDA)
 * - VEML6075 SCL -> Pico GPIO 5 (I2C0 SCL)
 * - VEML6075 VCC -> Pico 3.3V
 * - VEML6075 GND -> Pico GND
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "veml6075.h"

// I2C configuration
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define I2C_FREQ 100000  // 100kHz

VEML6075_t uv_sensor;
VEML6075_error_t err;

void init_uv_sensor(void) {
        // Initialize VEML6075
    err = veml6075_init(&uv_sensor, I2C_PORT);
    
    if (err != VEML6075_ERROR_SUCCESS) {
        printf("Failed to initialize VEML6075! Error: %d\n", err);
        return;
    }
    
    printf("VEML6075 initialized successfully!\n");
    
    // Check connection
    if (!veml6075_is_connected(&uv_sensor)) {
        printf("VEML6075 not connected!\n");
        return;
    }
    
    // Configure sensor (optional - these are already set in init)
    veml6075_set_integration_time(&uv_sensor, IT_100MS);
    veml6075_set_high_dynamic(&uv_sensor, DYNAMIC_NORMAL);
}

float get_uv() {
        // // Get raw values
        // uint16_t raw_uva = veml6075_get_raw_uva(&uv_sensor);
        // uint16_t raw_uvb = veml6075_get_raw_uvb(&uv_sensor);
        
        // // Get compensated values
        float uva = veml6075_get_uva(&uv_sensor);
        float uvb = veml6075_get_uvb(&uv_sensor);
        printf("UVA: %.2f, UVB: %.2f\n", uva, uvb);
        
        // Get UV index
        float uv_index = veml6075_get_index(&uv_sensor);
        
    return uv_index;
}

// Example CMakeLists.txt content:
/*
cmake_minimum_required(VERSION 3.13)

# Pull in Pico SDK
include(pico_sdk_import.cmake)

project(veml6075_example C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Add executable
add_executable(veml6075_example
    main.c
    veml6075.c
)

# Link libraries
target_link_libraries(veml6075_example
    pico_stdlib
    hardware_i2c
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(veml6075_example 1)
pico_enable_stdio_uart(veml6075_example 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(veml6075_example)
*/