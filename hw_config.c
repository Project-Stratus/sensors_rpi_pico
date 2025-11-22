
// /* hw_config.c
// Copyright 2021 Carl John Kugler III

// Licensed under the Apache License, Version 2.0 (the License); you may not use
// this file except in compliance with the License. You may obtain a copy of the
// License at

//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
// */

// /*
// This file should be tailored to match the hardware design.

// See
// https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/tree/main#customizing-for-the-hardware-configuration
// */

// #include "hw_config.h"

// /* Configuration of hardware SPI object */
// static spi_t spi = {
//     .hw_inst = spi0,  // SPI component
//     .sck_gpio = 6,    // GPIO number (not Pico pin number)
//     .mosi_gpio = 7,
//     .miso_gpio = 0,
//     .baud_rate = 125 * 1000 * 1000 / 8  // 15625000 Hz
//     //.baud_rate = 125 * 1000 * 1000 / 6  // 20833333 Hz
//     //.baud_rate = 125 * 1000 * 1000 / 4  // 31250000 Hz
//     //.baud_rate = 125 * 1000 * 1000 / 2  // 62500000 Hz
// };

// /* SPI Interface */
// static sd_spi_if_t spi_if = {
//     .spi = &spi,  // Pointer to the SPI driving this card
//     .ss_gpio = 1  // The SPI slave select GPIO for this SD card
// };

// /* Configuration of the SD Card socket object */
// static sd_card_t sd_card = {
//     .type = SD_IF_SPI,
//     .spi_if_p = &spi_if  // Pointer to the SPI interface driving this card
// };

// /* ********************************************************************** */

// size_t sd_get_num() { return 1; }

// /**
//  * @brief Get a pointer to an SD card object by its number.
//  *
//  * @param[in] num The number of the SD card to get.
//  *
//  * @return A pointer to the SD card object, or @c NULL if the number is invalid.
//  */
// sd_card_t *sd_get_by_num(size_t num) {
//     if (0 == num) {
//         // The number 0 is a valid SD card number.
//         // Return a pointer to the sd_card object.
//         return &sd_card;
//     } else {
//         // The number is invalid. Return @c NULL.
//         return NULL;
//     }
// }

// /* [] END OF FILE */

#include "hw_config.h"
#include "sd_card.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "SPI/my_spi.h"

// ----------------------------
// SPI Pin Configuration
// ----------------------------
#define SD_SCK_PIN 10
#define SD_MOSI_PIN 11
#define SD_MISO_PIN 12
#define SD_CS_PIN 13

// ----------------------------
// spi_t instance (my_spi.h)
// ----------------------------
static spi_t sd_spi = {
    .hw_inst = spi1, // Hardware SPI1

    .miso_gpio = SD_MISO_PIN,
    .mosi_gpio = SD_MOSI_PIN,
    .sck_gpio = SD_SCK_PIN,

    .baud_rate = 10 * 1000 * 1000, // 10 MHz (safe for SD cards)

    .spi_mode = 0, // SPI Mode 0 (SD standard)
    .no_miso_gpio_pull_up = false,

    .set_drive_strength = false,
    .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
    .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,

    .use_static_dma_channels = false,
    .tx_dma = 0,
    .rx_dma = 0,

    .initialized = false,
};

// ----------------------------
// SD SPI interface wrapper
// ----------------------------
static sd_spi_if_t sd_spi_if = {
    .spi = &sd_spi,
    .ss_gpio = SD_CS_PIN,

    .set_drive_strength = false,
    .ss_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
};

// ----------------------------
// SD card descriptor
// ----------------------------
static sd_card_t sd_card = {
    .type = SD_IF_SPI,
    .spi_if_p = &sd_spi_if,

    .use_card_detect = false,
    .card_detect_gpio = 0,
    .card_detected_true = 1,
    .card_detect_use_pull = false,
    .card_detect_pull_hi = false,
};

// ----------------------------
// API required by SD driver
// ----------------------------
size_t sd_get_num()
{
    return 1;
}

sd_card_t *sd_get_by_num(size_t num)
{
    return (num == 0) ? &sd_card : NULL;
}
