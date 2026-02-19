# ASDT Datalogger
This PCB is a rocketry flight datalogger with a comprehensive sensor suite.

## Features
- GPS
- Radio
- IMU
- Barometer
- Magnetometer
- Status LEDs
- Reverse polarity protection

## Parts
- ESP32-S3 microcontroller
- u-blox MAX-M10S GPS
- RFM95W-915 radio (SX1276)
- ISM6HG256X High/Low-G IMU
- BMI088 Low-G IMU
- ADXL375 High-G IMU
- MS5607 barometer
- BMP390 barometer
- MMC5983MA magnetometer

## Pinout
| GPIO | Function            |
|------|---------------------|
|    0 | BOOT                |
|    1 | MS5607 CS           |
|    2 | BMP390 CS           |
|    4 | MMC598 CS           |
|    7 | Radio CS            |
|   14 | Gyro CS             |
|   21 | HIGH_G CS           |
|   11 | SCK                 |
|   14 | IMU CS              |
|   13 | Barometer CS        |
|    0 | Radio CS            |
|    1 | Radio D0            |
|    2 | Radio reset         |
|   12 | LED data            |
|    7 | Expansion pin       |
|   15 | Expansion pin       |
|   18 | SD CMD              |
|   19 | SD data 0           |
|   20 | SD data 1           |
|   21 | SD data 2           |
|   22 | SD data 3           |
|   23 | SD clock            |
|    8 | Servo power enable  |
|   28 | Servo 1             |
|   29 | Servo 2             |
|   25 | Servo 3             |
|   24 | Servo 4             |
|   17 | Servo 5             |
|   16 | Servo 6             |
|   26 | Battery sense       |

## Addresses
| Name             | Part     | I2C Address |
|------------------|----------|-------------|
| Magnetometer     | LIS2MDL  | 0x1e        |
| GPS              | MAX-M10S | 0x42        |

## Notes
- The boot and reset button on the board are enabled by the jumper JXX on the top right side of the board, this jumper must be removed before flight
- The radio **may be damaged** if it is run without an antenna attached
- The servo data pins and LED data pin should never be floating
- The arming switch should have an internal pullup, and closing the switch connects the pin to ground
- The battery sense pin is connected through a XXXk-XXk voltage divider, so the pin voltage is 0.3311 times the battery voltage
- The total current consumption should be below 500mA continuous
- The GPS will blink an LED near the bottom of the board once per second once it gets a fix
- The addressable LEDs are WS2812-compatible, and they are chained as follows: SD, radio, IMU, barometer, magnetometer, status

