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

int main() {
    // Initialize stdio for USB serial output
    stdio_init_all();
    sleep_ms(2000);  // Give time for USB to enumerate
    
    printf("VEML6075 UV Sensor Test\n");
    printf("========================\n\n");
    
    // Initialize I2C
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Initialize VEML6075
    VEML6075_t uv_sensor;
    VEML6075_error_t err = veml6075_init(&uv_sensor, I2C_PORT);
    
    if (err != VEML6075_ERROR_SUCCESS) {
        printf("Failed to initialize VEML6075! Error: %d\n", err);
        return -1;
    }
    
    printf("VEML6075 initialized successfully!\n");
    
    // Check connection
    if (!veml6075_is_connected(&uv_sensor)) {
        printf("VEML6075 not connected!\n");
        return -1;
    }
    
    // Get device ID
    uint8_t device_id;
    veml6075_get_device_id(&uv_sensor, &device_id);
    printf("Device ID: 0x%02X\n", device_id);
    
    // Configure sensor (optional - these are already set in init)
    veml6075_set_integration_time(&uv_sensor, IT_100MS);
    veml6075_set_high_dynamic(&uv_sensor, DYNAMIC_NORMAL);
    
    printf("\nStarting UV measurements...\n");
    printf("Press Ctrl+C to exit\n\n");
    
    // Main loop - read UV data every second
    while (true) {
        // Get raw values
        uint16_t raw_uva = veml6075_get_raw_uva(&uv_sensor);
        uint16_t raw_uvb = veml6075_get_raw_uvb(&uv_sensor);
        
        // Get compensated values
        float uva = veml6075_get_uva(&uv_sensor);
        float uvb = veml6075_get_uvb(&uv_sensor);
        
        // Get UV index
        float uv_index = veml6075_get_index(&uv_sensor);
        
        // Get compensation values
        uint16_t vis_comp = veml6075_get_visible_compensation(&uv_sensor);
        uint16_t ir_comp = veml6075_get_ir_compensation(&uv_sensor);
        
        // Print results
        printf("Raw UVA: %5u | Raw UVB: %5u | ", raw_uva, raw_uvb);
        printf("UVA: %7.2f | UVB: %7.2f | ", uva, uvb);
        printf("UV Index: %.2f", uv_index);
        
        // Add UV index interpretation
        if (uv_index < 3.0) {
            printf(" (Low)");
        } else if (uv_index < 6.0) {
            printf(" (Moderate)");
        } else if (uv_index < 8.0) {
            printf(" (High)");
        } else if (uv_index < 11.0) {
            printf(" (Very High)");
        } else {
            printf(" (Extreme)");
        }
        
        printf("\n");
        
        // Wait 1 second before next reading
        sleep_ms(1000);
    }
    
    return 0;
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