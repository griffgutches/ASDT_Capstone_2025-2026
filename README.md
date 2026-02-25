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
|    1 | MS5607 Baro CS      |
|    2 | BMP390 Baro CS      |
|    4 | MMC598 Mag CS       |
|    7 | Radio CS            |
|   14 | BMI088 Gyro CS      |
|   21 | ADXL375 IMU CS      |
|   40 | BMI088 Accel CS     |
|   41 | ISM6HG256X IMU CS   |
|   11 | ADXL375 INT         |
|   13 | Radio INT           |
|   18 | BMI088 Accel INT    |
|   31 | GPS INT             |
|   42 | MMC598 Mag INT      |
|   10 | MISO                |
|   17 | SCK                 |
|   44 | MOSI                |
|   45 | SCL*                |
|   46 | SDA*                |
|   25 | Flash I03           |
|   26 | Flash I02           |
|   27 | Flash CS            |
|   28 | Flash CLK           |
|   29 | Flash IO1           |
|   30 | Flash IO0           |
|   35 | SD CMD              |
|   37 | SD data 0           |
|   38 | SD data 1           |
|   33 | SD data 2           |
|   34 | SD data 3           |
|   36 | SD clock            |
|    8 | Servo power enable  |
|   15 | XTAL_32K_P          |
|   16 | XTAL_32K_N          |
|   19 | USB_D-              |
|   20 | USB_D+              |
|    8 | Timepulse           |
|    8 | Battery Sense       |
|   39 | Buzzer              |
|   43 | ANT_SW              |
|    6 | Radio Busy          |
*SDA/SCL should be moved off GPIO45/46 on future designs because they are strapping pins

## Addresses
| Name             | Part     | I2C Address |
|------------------|----------|-------------|
| GPS              | MAX-M10S | 0x42 ****   |

## Notes
- The boot and reset button on the board are enabled by the jumper J5 on the left side of the board, this jumper must be removed before flight
- The radio **may be damaged** if it is run without an antenna attached
- The LED data pin should never be floating
- The battery sense pin is connected through a 300k-100k voltage divider, so the pin voltage is 0.25 times the battery voltage
- The total current consumption should be below 500mA continuous
- The GPS will blink an LED near the bottom of the board once per second once it gets a fix

