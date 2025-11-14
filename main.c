#include "tmp117.h"
#include "tmp117_registers.h"
#include "temperature.h"
#include "uv.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "hardware/i2c.h"
#include <stdbool.h>
#include <stdint.h>

// #include "hw_config.h"
// #include "f_util.h"
// #include "ff.h"

#define SERIAL_INIT_DELAY_MS 1000 // adjust as needed to mitigate garbage characters after serial interface is started
#define I2C_SDA_PIN 4// set to a different SDA pin as needed
#define I2C_SCL_PIN 5 // set to a different SCL pin as needed
#define TMP117_OFFSET_VALUE -25.0f  // temperature offset in degrees C set by user (try negative values for testing)
#define TMP117_CONVERSION_DELAY_MS 1000 // Adjust the delay based on conversion cycle time and preference

char*filename = "data_log.csv";

// void print_to_file(void) {
//     FATFS fs;
//     FRESULT fr = f_mount(&fs, "", 1);
//     if (fr != FR_OK) {
//         printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
//         return;
//     }

//     FIL fil;
//     fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
//     if (fr != FR_OK) {
//         printf("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
//         f_unmount("");
//         return;
//     }

//     if (f_printf(&fil, "%d, %s\n", to_ms_since_boot(get_absolute_time()), "hi") < 0) {
//         printf("f_printf failed\n");
//     }

//     fr = f_close(&fil);
//     if (fr != FR_OK) {
//         printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
//     }

//     f_unmount("");
// }

int main(void) {
    // initialize chosen interface
    stdio_init_all();
    // a little delay to ensure serial line stability
    sleep_ms(SERIAL_INIT_DELAY_MS);

    while (1) {
        // print_to_file();
        printf("Logged data to %s\n", filename);
        sleep_ms(1000);
    }

    // uncomment below to set I2C address other than 0x48 (e.g., 0x49)
    //tmp117_set_address(0x49);

    // Selects I2C instance (i2c_default is set as default in the tmp117.c)
    //tmp117_set_instance(i2c1); // change to i2c1 as needed

    // initialize I2C (default i2c0) and initialize variable with I2C frequency
    i2c_init(i2c_instance, 200 * 1000); // TMP117 400 kHz max.

    // configure the GPIO pins for I2C
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);


 
    // // check if TMP117 is on the I2C bus at the address specified
    // check_status();

    // // TMP117 software reset; loads EEPROM Power On Reset values
    // soft_reset();

    // while (1) {

    //     do {
    //         sleep_ms(TMP117_CONVERSION_DELAY_MS);
    //     } while (!data_ready()); // check if the data ready flag is high

    //     /* 1) typecast temp_result register to integer, converting from two's complement
    //        2) Multiply by 100 to scale the temperature (i.e. 2 decimal places)
    //        3) Shift right by 7 to account for the TMP117's 1/128 resolution (Q7 format) */
    //     int temp = read_temp_raw() * 100 >> 7;
    //     // Display the temperature in degrees Celsius, formatted to show two decimal places.
    //     printf("Temperature: %d.%02d °C\n", temp / 100, (temp < 0 ? -temp : temp) % 100);


    //     // floating point functions are also available for converting temp_result to Cesius or Fahrenheit
    //     //printf("\nTemperature: %.2f °C\t%.2f °F", read_temp_celsius(), read_temp_fahrenheit());
    // }

    return 0;
}