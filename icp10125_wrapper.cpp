// #include "pico/stdlib.h"
// #include "hardware/i2c.h"

// #include "icp10125.h"                  // Pimoroni C++ driver
// #include "common/pimoroni_i2c.hpp"       // Pimoroni I2C class

// #include "icp10125_wrapper.h"

// using namespace pimoroni;

// static I2C i2c(PimoroniI2C::I2C0, 0x63, 400000);   // same as Breakout Garden default
// static ICP10125 icp10125(&i2c);

// extern "C" {

// // ----------------------
// // Initialize device
// // ----------------------
// int icp10125_init_wrapper() {
//     return icp10125.init() ? 0 : -1;
// }

// // ----------------------
// // Perform measurement
// // ----------------------
// int icp10125_measure_wrapper(float *temperature, float *pressure, int *status) {

//     auto result = icp10125.measure(ICP10125::NORMAL);

//     *temperature = result.temperature;
//     *pressure = result.pressure;
//     *status = result.status;

//     return 0;
// }

// } // extern "C"
