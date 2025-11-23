Connections:

SD Card -> Pico

CS -> 13

SCK -> 10

MOSI -> 11

MISO -> 12

VCC -> 3V3

GND -> GND


All sensors (on same I2C line) -> Pico:

SDA -> 4

SCL -> 5

3.3V -> 3V3

GND -> GND



This repository contains source code to be compiled and run on a raspberry pico 2

**Aim**
- read temperature from the TMP117
- read magnetic strength in the x, y and z axis from the CMPS12
- read UVA, UVB, UVcomp1, and UVcomp intensity from the VEML6075
- read pressure (and possibly temperature) from the ICP10125
- read humitidy from the DollaTek humidity sensor
- save the measurements to the SD card
- Repeat several times
  
**Upload Any Code Related To These Aims Here**

**Pictorial Representation of This Repositories Aim**

![sensor-schematic](https://github.com/user-attachments/assets/ad4575c2-7ec9-4815-8330-a7ca4e80918e)


**Hardware:**
- TMP117 sensor
- CMPS12 sensor
- VEML6075
- ICP10125
- DollaTek humidity sensor
- SD card module
- Raspberry Pi Pico 2
