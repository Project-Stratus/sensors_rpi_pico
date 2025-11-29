// #include "sd_card.h"
// #include "ff.h"

// const char *logfile = "sensorlogs.csv";

// int setup_fs()
// {
//     sd_init_driver();
//     sd_card_t *sd = sd_get_by_drive_prefix("0:/");

//     FRESULT fr = f_mount(&sd->state.fatfs, sd_get_drive_prefix(sd), 1);
//     if (fr == FR_OK)
//     {
//         printf("Successfully mounted SD Card file-system.");
//     }
//     else
//     {
//         printf("Mount failed: %d\n", fr);
//         return -1;
//     }
// }

// void write_log(char *msg)
// {
//     uint32_t timestamp = to_ms_since_boot(get_absolute_time());
//     char buffer[256];

//     sprintf(buffer, "%u::%s", timestamp, msg);

//     FIL file;
//     FRESULT fr = f_open(&file, logfile, FA_WRITE | FA_OPEN_APPEND);

//     if (fr == FR_OK)
//     {
//         UINT bw;
//         fr = f_write(&file, buffer, strlen(buffer), &bw);
//         if (fr != FR_OK)
//         {
//             printf("Log file was opened, but failed to write log.");
//         }

//         f_close(&file);
//     }
// }

#include "f_util.h"
#include "ff.h"
#include <stdint.h>
#include <pico/time.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hw_config.h"

const char *filename = "data_log.csv";
static FATFS fs;
struct Log
{
    float uv;
    float direction;
    float temperature;
};

void setup_fs()
{
    FRESULT fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        while (1)
        {
            printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        return;
    }
}

void write_result(float uv, float direction, float temperature)
{
    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK)
    {
        printf("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
        f_unmount("");
        return;
    }

    if (f_printf(&fil, "%d, %f, %f, %f\n", to_ms_since_boot(get_absolute_time()), uv, direction, temperature) < 0)
    {
        printf("f_printf failed\n");
    }

    fr = f_close(&fil);
    if (fr != FR_OK)
    {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
}