// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "icp10125_wrapper.h"

// int main() {
//     stdio_init_all();

//     if (icp10125_init_wrapper() != 0) {
//         printf("ICP10125 init failed!\n");
//         return 1;
//     }

//     printf("ICP10125 init OK\n");

//     while (1) {
//         float temp, press;
//         int stat;

//         icp10125_measure_wrapper(&temp, &press, &stat);

//         printf("%fc  %fPa  %d\n", temp, press, stat);

//         sleep_ms(1000);
//     }

//     return 0;
// }
