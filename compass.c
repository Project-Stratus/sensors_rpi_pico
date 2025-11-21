#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

#define CMPS12_ADDRESS 0x60
#define ANGLE_8 1  // Register for 8-bit angle

uint8_t high_byte, low_byte, angle8;
int8_t pitch, roll;
uint16_t angle16;

// ---------------------------------------------
// Convert angle (0–359) into cardinal direction
// ---------------------------------------------
const char* getCardinalDirection(int angle) {
    static const char* directions[] = {
        "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
    };

    int index = (int)((angle / 22.5f) + 0.5f);
    return directions[index % 16];
}

int main() {
    stdio_init_all();

    // Init I2C at 400 kHz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(1000);
    printf("CMPS12 Test Start\n");

    while (1) {
        uint8_t reg = ANGLE_8;

        // Tell CMPS12 which register to start reading
        i2c_write_blocking(I2C_PORT, CMPS12_ADDRESS, &reg, 1, true);

        // Read 5 bytes: angle8, high, low, pitch, roll
        uint8_t buf[5];
        i2c_read_blocking(I2C_PORT, CMPS12_ADDRESS, buf, 5, false);

        angle8 = buf[0];
        high_byte = buf[1];
        low_byte  = buf[2];
        pitch = (int8_t)buf[3];
        roll  = (int8_t)buf[4];

        angle16 = ((uint16_t)high_byte << 8) | low_byte;

        int angle_deg = angle16 / 10;  // Convert to integer degrees

        printf("roll: %d    pitch: %d    angle8: %d    angle16: %d.%d    ",
               roll, pitch, angle8, angle16 / 10, angle16 % 10);

        printf("cardinal: %s\n", getCardinalDirection(angle_deg));

        sleep_ms(100);
    }

    return 0;
}
