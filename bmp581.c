/*
CHECKING POWERUP
- according to datasheet it is required that we
    - wait BMP_TIME_POWERUP_MS before intiating communication
- according to the datasheet, it is recommended that we:
    - read out chip_id and check that not zero
    - read out the STATUS register and check that status_nvm_rdy = 1, 
        and status_nvm_err == 0
    - read out the INT_STATUS.por register field and check that it is set 
        to 1; that means INT_STATUS==0x10

PIN CONNECTIONS
- GND -> EXTERNAL GND
- 3V3 (VDDIO and VDD) -> EXTERNAL 3V3 supply
- SDA -> HOST I2C SDA PIN
- SCL -> HOST I2C SCL PIN
- SDO -> floating, since ADR jumper sets the I2C address to 0x47 by 
- default, therefore it must be internally driving SDO to high?
- CSB -> floating (since, CSB jumper pulls CSB to VDD 3.3V) 
- since it is connected to VDD, need to disable integrated pull-up: 
- resistor: DRIVE_CONFIG.i2c_csb_pup_en = b0 (done by default, see below)
- INT -> EXTERNAL GND
- so need to disable INT_CONFIG.int_en (done by default, see below)

DESIRABLE DEFAULT WRITABLE REGISTER FIELD VALUES -  by default, the following writable register fields have values:
- INT_CONFIG.int_mode = b1 (latched)
- INT_CONFIG.int_pol = b0 (active-low)
- INT_CONFIG.int_od = b1 (open_drain)
- INT_CONFIG.int_en = b0 (disabled)
- INT_CONFIG.pad_int_drv = b0011 (drive strength 3)
- INT_SOURCE = 0x00 (disable INT except POR and software reset completion
    i.e. drdy_data_reg_en = fifo_full_en = fifo_ths_en = oor_p_en = 0)
- FIFO_CONFIG.fifo_threshold = b00000 (FIFO threshold disabled)
- FIFO_CONFIG.fifo_mode = b0 (Stream-to-FIFO MODE)
- FIFO_SEL.fifo_frame_sel = b00 (FIFO not enabled)
- NVM_ADDR.nvm_row_address = b000000
- NVM_ADDR.nvm_prog_en = b0 (NVM programming disabled)
- NVM_DATA_LSB.nvm_data_lsb = b00000000
- NVM_DATA_MSB.nvm_data_msb = b00000000
- DSP_CONFIG.iir_flushed_forced_en = b0 (IIR filter flush not executed in
    forced mode)
- DSP_CONFIG.shdw_sel_irr_t = b0 (temperature data register value selected 
    before IIR filter)
- DSP_CONFIG.fifo_sel_iir_t = b0 (fifo tempeature data value selected 
-   before IIR filter)
- DSP_CONFIG.shdw_sel_irr_p = b0 (pressure data register value selected 
    before IIR filter)
- DSP_CONFIG.fifo_sel_iir_p = b0 (fifo pressure data value selected before 
    IIR filter)
- DSP_CONFIG.oor_sel_iir_p = b0 (value selected before IIR filter)
- DSP_IIR.set_iir_p = b000 (filter coefficient is 0, BYPASS)
- DSP_IIR.set_iir_t = b000 (dilter coefficient is 0, BYPASS)
- OOR_THR_P_LSB.oor_thr_p_7_0 = b00000000 (press oor threshold[7:0]=0)
- OOR_THR_P_MSB.oor_thr_p_15_8 = b00000000 (press oor threshold[15:8]=0)
- OOR_RANGE.oor_range_p = 0 (press oor range is 0)
- OOR_CONFIG.oor_thr_p_16 = 0 (pressure oor threshold[16] = 0)
- OOR_CONFIG.cnt_lim = b00 (1 pressure measurement oor causes interrupt)
- ODR_CONFIG.odr = b11100 (1 Hz Output Data Rate on Normal Mode)
- ODR_CONFIG.deep_dis = b0 (one condition satisfied for deep standby)
- These values are acceptable
- the device is deep standby (since all conditions satisfied) however
    since we are using continuous mode, we dont care...

UNDESIRABLE DEFAULT REGISTER FIELD VALUES
- By default, the following registers have values:
- OSR_CONFIG.osr_t = b000 (temperature oversampling rate is 1)
- OSR_CONFIG.osr_p = b000 (pressure oversampling rate is 1)
- OSR_CONFIG.press_en = b0 (pressure measurements diasbled, since 
- fifio_frame_sel != [b10, b11])
- ODR_CONFIG.pwr_mode = b00 (standby mode)

REGISTER FIELDS TO SET
- OSR_CONFIG.osr_t = userinput
- OSR_CONFIG.osr_p = userinput
- OSR_CONFIG.press_en = b1
- ODR_CONFIG.pwr_mode = 11 (continuous mode)

REGISTER FIELDS NOT TO READ- we dont need to read
- OSR_EFF because configuration check is only performed in normal mode, 
    not continuous. furthermore, all configurations OSR, ODR should be 
    valid in continuous mode
- CHIP_STATUS.hif because we can safely assume we are in either I2C
    only mode or SPI and I2C Available
    other register fields since they are irrelevant (e.g. FIFO_DATA) 

REGISTER FIELDS TO READ - relevant fields we should read:
- ODR_CONFIG.pwr_mode -> to check in continuous mode
- INT_STATUS.por -> to check if a 'random' power-on-reset (aka power-up-reset) 
    has occured
- PRESS_DATA_XLSB, PRESS_DATA_LSB, PRESS_DATA_MSB -> we are measuring this
READING INT_STATUS and PRESS_DATA
- Note the INT_STATUS register (0x26) is close to the PRESS_DATA (0x20-0x22) 
    registers
- thus we can either do a single burst read (from PRESS_DATA_XLSB to 
    INT_STATUS), or 1 burst read (from PRESS_DATA_XLSB to PRESS_DATA_MSB) 
    and a register read (INT_STATUS).
- For The First Approach:
    - send send slave i2c address (1)
    - send PRESS_DATA_XLSB address (1) 
    - send slave i2c address (1)
    - read bytes (7) (PRESS_DATA_*, reserved registers, INT_STATUS)
- For The Second Approach
    - send slave i2c address (1) 
    - send PRESS_DATA_XLSB address (1)
    - send slave i2c address (1) 
    - read bytes (3) (PRESS_DATA*) 
    - send slave i2c address (1)
    - send INT_STATUS reg address (1)
    - send slave i2c address (1)
    - read bytes (1) (INT_STATUS)
- Both transfer the same number of bytes
- Generally the first approach is better since there are fewer I2C write and 
    read operations in total. Consequently there will less error checking and
    fewer start, stop and restart bits.
*/
#include "bmp581.h"
#include <stdbool.h>

#define BMP581_I2C_SLAVE_ADDR 0x47 //due to ADR jumper
#define BMP581_TIME_POWERUP_MS 2
#define BMP581_TIME_MAX_MS 4
#define BMP581_TIME_SOFT_RESET_MS 2
#define BMP581_BITS_PER_BYTE 8
#define BMP581_PRESS_WIDTH BMP581_NUM_PRESS_DATA_REGS * BMP581_BITS_PER_BYTE
#define BMP581_PRESS_RADIX_BIT_POS 6u 
#define BMP581_MAX_MEASUREMENT_PERIOD_MS 110
#define MS_TO_US 1000
#define REG(X) ((enum bmp581_reg_t)(X))

#define NONSTOP true
#define STOP false

enum bmp581_reg_t {
    bmp581_chip_id = 0x01,
    bmp581_int_source = 0x15,
    bmp581_press_data_xlsb  = 0x20,
    bmp581_press_data_lsb  = 0x21,
    bmp581_press_data_msb  = 0x22,
    bmp581_reserved_reg1 = 0x23,
    bmp581_reserved_reg2 = 0x24,
    bmp581_reserved_reg3 = 0x25,
    bmp581_reserved_reg4 = 0x26,
    bmp581_int_status = 0x27,
    bmp581_status = 0x28,
    bmp581_osr_config = 0x36,
    bmp581_odr_config = 0x37,
    bmp581_cmd = 0x7E,
};

enum bmp581_int_source_field_t {
    bmp581_drdy_data_reg_en = 0b00000001
    //other fields ignored since irrelevant...
};

enum bmp581_int_status_field_t {
    bmp581_por              = 0b00010000,            //power on reset
    bmp581_drdy_data_reg    = 0b00000001   
    //other fields ignored since irrelevant...
};

enum bmp581_status_field_t {
    bmp581_status_nvm_rdy      = 0b00000010,
    bmp581_status_nvm_err      = 0b00000100,
    bmp581_status_nvm_cmd_err  = 0b00001000,
};

enum bmp581_osr_config_field_t {
    bmp581_osr_t    = 0b00000111,
    bmp581_osr_p    = 0b00111000,
    bmp581_press_en = 0b01000000
};

enum bmp581_odr_config_field_t{
    bmp581_pwr_mode = 0b00000011,
    bmp581_odr      = 0b01111100,
    bmp581_deep_dis = 0b10000000
};

enum bmp581_pwr_mode_t {
    bmp581_standby =    0b00,
    bmp581_normal =     0b01,
    bmp581_forced =     0b10,
    bmp581_nonstop =    0b11, //aka continuous mode
};

enum bmp581_odr_t {
    bmp581_1hz  =  0x1C << 2
    //lot other more other output data rate
    //however since we are using continuous mode, the value
    //of odr is ignored
}; 

enum bmp581_cmd_t {
    bmp581_cmd_soft_reset = 0xB6
};

#define TYPE_ASSERT(V, T) _Generic(V, T: (void)NULL)

#define BMP581_BURST_WRITE(I2C, LEN, REG, BYTES_WRITTEN, ...)       \
    bmp581_burst_write(I2C, (LEN), (uint8_t[]){REG, __VA_ARGS__})

#define BMP581_BURST_WRITE_EX(I2C, LEN, REG, WRITE_NACK,                 \
    WRITE_MISMATCH, ...)                                                 \
    bmp581_burst_write_ex(I2C, (LEN), (uint8_t[]){REG, __VA_ARGS__},     \
        WRITE_NACK, WRITE_MISMATCH)

#define EXTRACT(REG_VAL, MASK) (REG_VAL & (MASK))
#define FLAGGED(REG_VAL, FLAG) EXTRACT(REG_VAL, FLAG)

static int bmp581_burst_write(
    i2c_inst_t* i2c, 
    size_t len,
    const uint8_t* buf
){
    return i2c_write_blocking(i2c, BMP581_I2C_SLAVE_ADDR, buf, len + 1, STOP); 
}

static int bmp581_reg_write(
    i2c_inst_t* i2c, 
    enum bmp581_reg_t reg, 
    uint8_t val
){
    return bmp581_burst_write(i2c, 1, (uint8_t[]){reg, val});
}

static int bmp581_set_reg_addr(
    i2c_inst_t* i2c, 
    enum bmp581_reg_t reg, 
    bool nonstop
){
    return i2c_write_blocking(i2c, BMP581_I2C_SLAVE_ADDR, (uint8_t[]){reg}, 1, 
        nonstop);
}

static int bmp581_burst_read(i2c_inst_t* i2c, size_t len, uint8_t o_buf[len]){
    return i2c_read_blocking(i2c, BMP581_I2C_SLAVE_ADDR, o_buf, len, STOP);
}

static int bmp581_reg_read(i2c_inst_t* i2c,  uint8_t* o_reg_val){
    return bmp581_burst_read(i2c, 1, o_reg_val);
}

static enum bmp581_err_t bmp581_burst_read_ex(
    i2c_inst_t* i2c,
    enum bmp581_reg_t reg, 
    size_t len,
    uint8_t o_buf[len], 
    enum bmp581_err_t reg_set_nack, 
    enum bmp581_err_t reg_set_mismatch, 
    enum bmp581_err_t read_nack,
    enum bmp581_err_t read_mismatch
){
    int bytes_moved;
    bytes_moved = bmp581_set_reg_addr(i2c, reg, NONSTOP);
    if (bytes_moved == PICO_ERROR_GENERIC)
        return reg_set_nack;
    if (bytes_moved != 1)
        return reg_set_mismatch;
    bytes_moved = bmp581_burst_read(i2c, len, o_buf);
    if (bytes_moved == PICO_ERROR_GENERIC)
        return read_mismatch;
    if (bytes_moved != len)
        return read_mismatch;
    return bmp581_err_ok;
}

static enum bmp581_err_t bmp581_reg_read_ex(
    i2c_inst_t* i2c,
    enum bmp581_reg_t reg, 
    uint8_t* o_reg_val, 
    enum bmp581_err_t reg_set_nack, 
    enum bmp581_err_t reg_set_mismatch, 
    enum bmp581_err_t read_nack,
    enum bmp581_err_t read_mismatch
){
    return bmp581_burst_read_ex(
        i2c, reg, 1, o_reg_val, 
        reg_set_nack, reg_set_mismatch, read_nack, read_mismatch
    );
}

static int bmp581_burst_write_ex(
    i2c_inst_t* i2c,
    size_t len,
    const uint8_t* buf,
    enum bmp581_err_t write_nack,
    enum bmp581_err_t write_mismatch
){
    int bytes_moved;
    bytes_moved = bmp581_burst_write(i2c, len, buf);
    if (bytes_moved == PICO_ERROR_GENERIC)
        return write_nack;
    if (bytes_moved != len + 1)
        return write_mismatch;
    return bmp581_err_ok;
}

static int bmp581_reg_write_ex(
    i2c_inst_t* i2c,
    enum bmp581_reg_t reg, 
    uint8_t val,
    enum bmp581_err_t write_nack,
    enum bmp581_err_t write_mismatch
){
    int bytes_moved;
    bytes_moved = bmp581_reg_write(i2c, reg, val);
    if (bytes_moved == PICO_ERROR_GENERIC)
        return write_nack;
    if (bytes_moved != 2)
        return write_mismatch;
    return bmp581_err_ok;
}

static void bmp581_wait_for_powerup(void){sleep_ms(BMP581_TIME_POWERUP_MS);}

static void bmp581_wait_max(void){sleep_ms(BMP581_TIME_MAX_MS);}

static void bmp581_wait_for_soft_reset(void){
    sleep_ms(BMP581_TIME_SOFT_RESET_MS);
}


/*
PRE: 
- i2c_init has been called
- the most recent call of i2c_init was successful
PURPOSE
- this function performs some checks upon power-up of the bmp581,
    as recommended by the datasheet
*/
static enum bmp581_err_t bmp581_check_powerup(i2c_inst_t* i2c){
    //compiler should optimize some of these local variables away
    enum {
        start_status = bmp581_int_status,
        end_status   = bmp581_status,
        num_statuses = end_status - start_status + 1
    };
    static_assert(start_status + num_statuses == end_status + 1);
    uint8_t chip_id;
    uint8_t statuses[num_statuses];
    uint8_t status;
    uint8_t int_status;
    enum bmp581_err_t err;
    //sensor is in sleep mode
    //need to wait BMP_TIME_POWERUP_MS before communicating
    bmp581_wait_for_powerup();
    //read out chip_id and check that not zero
    err = bmp581_reg_read_ex(i2c, bmp581_chip_id, &chip_id,
        bmp581_err_chip_id_set_addr_nack, 
        bmp581_err_chip_id_set_mismatch, 
        bmp581_err_chip_id_read_addr_nack,
        bmp581_err_chip_id_read_mismatch);
    if (err != bmp581_err_ok) return err;
    if (chip_id == 0)
        return bmp581_err_zero_chipid;
    //read out the STATUS register and check that status_nvm_rdy = 1, 
    //and status_nvm_err == 0
    err = bmp581_burst_read_ex(i2c, start_status, num_statuses, statuses,
        bmp581_err_statuses_set_addr_nack,
        bmp581_err_statuses_set_mismatch,
        bmp581_err_statuses_read_addr_nack,
        bmp581_err_statuses_read_mismatch
    );
    if (err != bmp581_err_ok) return err;
    status = statuses[bmp581_status - start_status];
    switch(EXTRACT(status, bmp581_status_nvm_err | bmp581_status_nvm_rdy)){
        case 0:
            return bmp581_err_nvm_not_rdy; 
        case bmp581_status_nvm_rdy:
            break;
        case bmp581_status_nvm_err:
            return bmp581_err_nvm_err_and_nvm_not_rdy;
        case bmp581_status_nvm_err | bmp581_status_nvm_rdy:
            return bmp581_err_nvm_err;
    }
    //read out the INT_STATUS.por register field and check that it is set 
    //to 1; that means INT_STATUS==0x10
    //this also clear INT_STATUS.por in the process
    int_status = statuses[bmp581_int_status - start_status];
    if (int_status != bmp581_por)
        return bmp581_err_zero_por;
    return bmp581_err_ok;
}

/* 
PRE: 
    - bmp581_check_powerup was called
    - the most recent call to bmp581_check_powerup was successful
PURPOSE: 
- this function sets:
- OSR_CONFIG.osr_t = userinput
- OSR_CONFIG.osr_p = userinput
- OSR_CONFIG.press_en = b1 (pressure measurements enabled)
- OSR_CONFIG.reserved_7 = b0 (same as before)
- ODR_CONFIG.pwr_mode = b11 (continuous mode)
- ODR_CONFIG.odr = 0x1C (1 Hz output data rate)
    the value of ODR_CONFIG.odr can be anything since it is
    ignored in continuous mode. odr is set to the same
    value it was before
- ODR_CONFIG.deep_dis = b0
    i.e. deep_dis is set to the same value it was before  
*/
static enum bmp581_err_t bmp581_configure(
    i2c_inst_t* i2c, 
    enum bmp581_osr_t_t osr_t, 
    enum bmp581_osr_p_t osr_p
){
    enum {
        start_config = bmp581_osr_config,
        end_config   = bmp581_odr_config,
        num_configs  = end_config - start_config + 1
    };
    static_assert(start_config + num_configs == end_config + 1);
    uint8_t osr_config;
    uint8_t odr_config;
    uint8_t osr_config_read;
    uint8_t odr_config_read;
    uint8_t configs[num_configs];
    enum bmp581_err_t err;
    //write desired osr_config and odr_config to bmp581
    osr_config = osr_t | osr_p | bmp581_press_en;
    odr_config = bmp581_nonstop | bmp581_1hz;
    err = BMP581_BURST_WRITE_EX(i2c, num_configs, REG(bmp581_osr_config), 
        bmp581_err_configs_write_addr_nack, bmp581_err_configs_write_mismatch,  
        osr_config, odr_config);
    if (err != bmp581_err_ok) return err;
    /*Wait Till Changes Are Implemented
    Data sheet doesnt specify how long to wait.
    So we will wait with the maximum time of all the electrical timing 
    characteristics: BMP581_TIME_MAX_MS*/ 
    bmp581_wait_max();
    //Check To See If Registers Were Set As Intended
    err = bmp581_burst_read_ex(i2c, start_config, num_configs, configs,
        bmp581_err_osr_config_set_addr_nack,
        bmp581_err_osr_config_set_mismatch,
        bmp581_err_configs_read_addr_nack,
        bmp581_err_configs_read_mismatch);
    if (err != bmp581_err_ok) return err;
    osr_config_read = configs[bmp581_osr_config - start_config];
    odr_config_read = configs[bmp581_odr_config - start_config];
    if (osr_config_read != osr_config){
        if (osr_config_read != odr_config)
            return bmp581_err_opposing_configs_read;
        return bmp581_err_opposing_osr_config_read;
    } else if (odr_config_read != odr_config)
        return bmp581_err_opposing_odr_config_read;
    return bmp581_err_ok;
}

static enum bmp581_err_t bmp581_write_int_source(
    i2c_inst_t* i2c,
    uint8_t int_source_val
){
    int bytes_moved;
    enum bmp581_err_t err;
    uint8_t int_source_read = 100;
    err = bmp581_reg_write_ex(i2c, bmp581_int_source, int_source_val,
        bmp581_err_int_source_write_addr_nack,
        bmp581_err_int_source_write_mismatch);
    if (err != bmp581_err_ok) return err;
    err = bmp581_reg_read_ex(i2c, bmp581_int_source, &int_source_read,
        bmp581_err_int_source_set_addr_nack,
        bmp581_err_int_source_set_mismatch,
        bmp581_err_int_source_read_addr_nack,
        bmp581_err_int_source_read_mismatch);
    if (err != bmp581_err_ok) return err;
    if (int_source_val != int_source_read)
        return bmp581_err_opposing_int_source_read;
    return bmp581_err_ok;
}

static bmp581_eerr_t bmp581_wait_for_drdy(i2c_inst_t* i2c){
    enum bmp581_err_t err;
    err = bmp581_write_int_source(i2c, bmp581_drdy_data_reg_en);
    if (err != bmp581_err_ok) return err;
    absolute_time_t start = get_absolute_time();
    while (true) {
        uint8_t int_status;
        err = bmp581_reg_read_ex(i2c, bmp581_int_status, &int_status,
            bmp581_err_int_status_set_addr_nack,
            bmp581_err_int_status_set_mismatch,
            bmp581_err_int_status_read_addr_nack,
            bmp581_err_int_status_read_mismatch);
        if (err != bmp581_err_ok) return err;
        if (FLAGGED(int_status, bmp581_drdy_data_reg))
            break;
        if (absolute_time_diff_us(start, get_absolute_time()) / MS_TO_US >     
            BMP581_MAX_MEASUREMENT_PERIOD_MS)
            return bmp581_err_drdy_timeout;
        sleep_us(500); //no need to overwork rpi pico
    }
    err = bmp581_write_int_source(i2c, 0);
    if (err != bmp581_err_ok) return -err;
}

/*
PRE: 
- i2c_init called 
- the most recent call to i2c_init was successful
PURPOSE
- calls bmp581_check_powerup to perform checks upon powerup
- if there is an error, it returns the error
- otherwise, it calls bmp581_configure to configure the
    bmp581, 
- if there is an error, it returns the error code
- otherwise, it calls bmp581_wait_for_drdy to cause the
    rpi pico to wait until bmp581_wait_for_drdy is ready
*/
extern enum bmp581_err_t bmp581_init(
    i2c_inst_t* i2c, 
    enum bmp581_osr_t_t osr_t, 
    enum bmp581_osr_p_t osr_p
){
    enum bmp581_err_t err;
    err = bmp581_check_powerup(i2c);
    if (err != bmp581_err_ok) return err;
    err = bmp581_configure(i2c, osr_t, osr_p);
    if (err != bmp581_err_ok) return err;
    return bmp581_wait_for_drdy(i2c);
}

/*
PRE:
- bmp581_configure or bmp581_handle_por called and the most recent call
    was successful
PURPOSE:
- reads INT_STATUS.por to check if a 'random' power-on-reset (aka power-up 
    reset has occurred)
- if so, it returns with an error
- otherwise it reads and returns the pressure data
*/
static enum bmp581_err_t bmp581_read_press(
    i2c_inst_t* i2c, 
    bmp581_press_t* o_press
){
    static_assert(sizeof *o_press >= BMP581_NUM_PRESS_DATA_REGS);
    enum {
        start_reg = bmp581_press_data_xlsb,
        end_reg = bmp581_int_status,
        bytes_to_read = end_reg - start_reg + 1
    };
    static_assert(start_reg + bytes_to_read == end_reg + 1);
    uint8_t press_data_xlsb;
    uint8_t press_data_lsb;
    uint8_t press_data_msb;
    uint8_t int_status;
    uint8_t reg_vals[bytes_to_read];
    int bytes_moved;
    //read PRESS_DATA_* and INT_STATUS.por 
    //the por interrupt is always enabled
    //A read of the INT_STATUS will clear the status
    bytes_moved = bmp581_set_reg_addr(i2c, start_reg, NONSTOP);
    if (bytes_moved == PICO_ERROR_GENERIC)
        return bmp581_err_press_data_set_addr_nack;
    else if (bytes_moved != 1)
        return bmp581_err_press_data_set_mismath;
    bytes_moved = bmp581_burst_read(i2c, bytes_to_read, reg_vals);
    if (bytes_moved == PICO_ERROR_GENERIC)
        return bmp581_err_press_data_int_status_read_addr_nack;
    if (bytes_moved != bytes_to_read)
        return bmp581_err_press_data_int_status_read_mismatch;
    int_status = reg_vals[bmp581_int_status - start_reg];
    //check por is 1 and return an err if so
    if (FLAGGED(int_status, bmp581_por))
        return bmp581_err_por;
    //read and return the pressure value
    press_data_xlsb = reg_vals[bmp581_press_data_xlsb - start_reg]; 
    press_data_lsb  = reg_vals[bmp581_press_data_lsb  - start_reg]; 
    press_data_msb  = reg_vals[bmp581_press_data_msb  - start_reg]; 
    *o_press = 
        press_data_xlsb | 
        press_data_lsb << BMP581_BITS_PER_BYTE | 
        press_data_msb << BMP581_BITS_PER_BYTE * 2;
    return bmp581_err_ok;
}

/*
PRE: 
- i2c_init has been called
- the most recent call of i2c_init was successfull
PURPOSE
- resets the values of every register to their default and changed the 
    mode of the device to deep standby
*/
extern enum bmp581_err_t bmp581_soft_reset(i2c_inst_t * i2c){
    enum bmp581_err_t err;
    err = bmp581_reg_write_ex(i2c, bmp581_cmd, bmp581_cmd_soft_reset,
        bmp581_err_cmd_write_addr_nack,
        bmp581_err_cmd_write_mismatch);
    if (err != bmp581_err_ok) return err;
    /*
    int bytes_written;
    bytes_written = bmp581_reg_write(i2c, , bmp581_cmd_soft_reset);
    if (bytes_written == PICO_ERROR_GENERIC)
        return bmp581_err_cmd_write_addr_nack;
    if (bytes_written != 1)
        return bmp581_err_cmd_write_mismatch;
    */
    bmp581_wait_for_soft_reset();
    return bmp581_err_ok;
}

/*
PRE:
- bmp581_read_pressure has been called
- the most recent call to bmp581_read_pressure returns bmp581_err_por
PURPOSE:
- attempts to reinitialize the device by calling bmp581_init,
- if this fails, it performs a software reset, and tries again
- if it still fails, it gives up and returns the error
*/
static enum bmp581_err_t bmp581_handle_por(
    i2c_inst_t* i2c, 
    enum bmp581_osr_t_t osr_t, 
    enum bmp581_osr_p_t osr_p
){
    //try init the device again
    enum bmp581_err_t err;
    if (bmp581_init(i2c, osr_t, osr_p) == bmp581_err_ok)
        return bmp581_err_ok;
    /*under normal circumstances, bmp581_init should never fail since 
    power-on-reset should reset registers to their defaults and change the
    current mode to standby.*/
    //Perform a software reset and try init the device again, and give up
    //if this doesnt work
    err = bmp581_soft_reset(i2c);
    if (err != bmp581_err_ok)
        return err;
    return bmp581_init(i2c, osr_t, osr_p);
}

/*
PRE:
- bmp581_configure was called
- the most recent call to bmp581_configure was successful
PURPOSE:
- calls bmp581_read_pressure to try read pressure data
- if successful, the pressure data is returned
- otherwise,
*/
extern bmp581_eerr_t bmp581_read_press_handle_por(
    i2c_inst_t* i2c, 
    bmp581_press_t* o_pressure,
    enum bmp581_osr_t_t osr_t, 
    enum bmp581_osr_p_t osr_p
){
    enum bmp581_err_t err;
    err = bmp581_read_press(i2c, o_pressure);
    if (err != bmp581_err_por)
        return err;
    err = bmp581_handle_por(i2c, osr_t, osr_p);
    if (err == bmp581_err_ok)
        return err;
    return -bmp581_read_press(i2c, o_pressure);
}

extern struct bmp581_pressure_t bmp581_decode_press(bmp581_press_t press){
    struct bmp581_pressure_t pressure;
    pressure.nat = press >> BMP581_PRESS_RADIX_BIT_POS;
    pressure.frac = press & ~(~0u << BMP581_PRESS_RADIX_BIT_POS);
    pressure.frac = pressure.frac * BMP581_PRESSURE_DP_POW >> 
        BMP581_PRESS_RADIX_BIT_POS;
    return pressure;
}

#if BMP581_ENABLE_DECODE_PRESSF
extern float bmp581_decode_pressf(bmp581_press_t press){
    return (float)press / (2 << BMP581_PRESS_RADIX_BIT_POS);
}
#endif

//0111 1111 0111 1111 0101 1111