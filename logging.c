#include "sd_card.h"
#include "ff.h"

static FIL file;
const char *logfile = "0:/sensorlogs.csv";

int setup_fs()
{
    sd_init_driver();
    sd_card_t *sd = sd_get_by_drive_prefix("0:/");

    FRESULT fr = f_mount(&sd->state.fatfs, sd_get_drive_prefix(sd), 1);
    if (fr == FR_OK)
    {
        printf("Successfully mounted SD Card file-system.");
    }
    else
    {
        printf("Mount failed: %d\n", fr);
        return -1;
    }
}
