#include <Arduino.h>
#include <pins.h>
#include <SoftwareSPI.h>
#include <MS5611_SPI.h>
#include <ISM6HG256XSensor.h>
#include <Adafruit_LIS2MDL.h>
#include <SDFS.h>
#include "led.h"
#include "ina745.h"

SoftwareSPI softSPI(SPI_SCK, SPI_MISO, SPI_MOSI);

MS5611_SPI baro(BAROMETER_CS, &softSPI);

ISM6HG256XSensor imu(&softSPI, IMU_CS);

Adafruit_LIS2MDL mag = Adafruit_LIS2MDL();

INA745 servo_monitors[] = {
  INA745(0x40, &Wire),
  INA745(0x41, &Wire),
  INA745(0x43, &Wire),
  INA745(0x44, &Wire),
  INA745(0x45, &Wire),
  INA745(0x47, &Wire)
};

void setup() {
  // Disable servo power on startup due to inrush
  // Removing this could damage the board
  pinMode(SERVO_POWER_ENABLE, OUTPUT);
  digitalWrite(SERVO_POWER_ENABLE, LOW);

  // put the level shifter into the regular mode
  pinMode(LEVELSHIFT_DIR, OUTPUT);
  digitalWrite(LEVELSHIFT_DIR, LOW);

  // no floating pins for levelshifter
  pinMode(SERVO_1, OUTPUT); digitalWrite(SERVO_1, LOW);
  pinMode(SERVO_2, OUTPUT); digitalWrite(SERVO_2, LOW);
  pinMode(SERVO_3, OUTPUT); digitalWrite(SERVO_3, LOW);
  pinMode(SERVO_4, OUTPUT); digitalWrite(SERVO_4, LOW);
  pinMode(SERVO_5, OUTPUT); digitalWrite(SERVO_5, LOW);
  pinMode(SERVO_6, OUTPUT); digitalWrite(SERVO_6, LOW);
  pinMode(LED_DATA, OUTPUT); digitalWrite(LED_DATA, LOW);

  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);

  pinMode(BAROMETER_CS, OUTPUT);
  pinMode(IMU_CS, OUTPUT);
  pinMode(RADIO_CS, OUTPUT);

  pinMode(BATTERY_SENSE, INPUT);

  digitalWrite(BAROMETER_CS, HIGH);
  digitalWrite(IMU_CS, HIGH);
  digitalWrite(RADIO_CS, HIGH);
  baro.setSPIspeed(1000000);

  Serial.begin(9600);
  sleep_ms(2000);
  Serial.println("test");
  //sleep_ms(5000);
  led_init();

  softSPI.begin();

  Serial.println("baro.begin()");
  if (baro.begin() == true) {
    Serial.print("MS5611 found: ");
    Serial.println(baro.getDeviceID(), HEX);
  } else {
    while (true) {
      Serial.println("MS5611 not found. halt.");
      delay(1);
    }
  }
  baro.reset(1);

  int imu_status = imu.begin();
  if (imu_status != 0) {
    Serial.println("IMU error");
    while (true) {}
  }
  Serial.println("IMU detected");
  imu.Enable_X();
  imu.Enable_G();

  imu.Set_X_FullScale(8);

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.setClock(100000);
  Wire.begin();

  if (!mag.begin()) {
    Serial.println("Magnetometer error");
    while (true) {}
  }
  Serial.println("Magnetometer detected");
  mag.setDataRate(LIS2MDL_RATE_100_HZ);

  // my SD card doesn't enumerate unless it's fully powered off and on again,
  // so I have this disabled for development.  This is probably specific to my
  // card, so this probably won't be an issue for you

  //if (!SDFS.setConfig(SDFSConfig(SD_CLOCK, SD_CMD, SD_DATA_0).setAutoFormat(true))) {
  //  Serial.println("SD config error");
  //}
  //if (!SDFS.begin()) {
  //  Serial.println("SD error");
  //  //while (true) {}
  //} else {
  //  Serial.println("SD set up");
  //}

  for (int i = 0; i < 6; i++) {
    if (servo_monitors[i].begin() != INA745Result::INA_SUCCESS) {
      Serial.printf("monitor %d failed to initialize\n", i);
    }
  }
}

// from https://gist.github.com/postspectacular/2a4a8db092011c6743a7
float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float fract(float x) { return x - int(x); }

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

void loop() {
  if (millis() > 2000) {
    // Let the capacitors slowly charge for a couple seconds before opening the mosfets
    digitalWrite(SERVO_POWER_ENABLE, HIGH);
  }

  for (int i = 0; i < 6; i++) {
    float rgb[3] = { 0 };
    hsv2rgb(((float) ((millis() / 2 + i * 80) % 1000)) / 1000.0, 1.0, 0.125, rgb); // control brightness with .125
    leds[i] = RGB((uint8_t)(rgb[0] * 255.0), (uint8_t)(rgb[1] * 255.0), (uint8_t)(rgb[2] * 255.0));
  }
  //leds[LED_STATUS] = RGB(0, 128, 0);
  led_show();
  sleep_us(400); 

  static ulong last_measurement = 0;
  if (millis() - last_measurement > 5000) {
    last_measurement = millis();
    int e = baro.read();
    if (e != MS5611_READ_OK) {
      Serial.print("barometer error = ");
      Serial.println(e);
    }

    Serial.print("Barometer\n");
    Serial.print("T:\t");
    Serial.print(baro.getTemperature(), 2);
    Serial.print("\tP:\t");
    Serial.print(baro.getPressure(), 2);
    Serial.println();
    Serial.println();

    //float battery_voltage = ((float)analogRead(BATTERY_SENSE)) / (0.3311 * 4096.0);
    //Serial.printf("Battery voltage = %fV\n", battery_voltage);

    ISM6HG256X_Axes_t accel, rot_rate;
    imu.Get_X_Axes(&accel);
    imu.Get_G_Axes(&rot_rate);

    Serial.println("IMU:");
    Serial.printf("Acceleration %d %d %d\n", accel.x, accel.y, accel.z);
    Serial.printf("Rotation Rate %d %d %d\n\n", rot_rate.x, rot_rate.y, rot_rate.z);

    sensors_event_t event;
    mag.getEvent(&event);
    Serial.printf("Magnetometer:\n %f %f %f\n\n", event.magnetic.x, event.magnetic.y, event.magnetic.z);

    /*for (int i = 0; i < 6; i++) {
      servo_monitors[i].read();
      Serial.printf("%d: %d mV, %d mA, %d m°C, %d uW\n", i + 1, servo_monitors[i].bus_millivolts(), servo_monitors[i].current_milliamps(), servo_monitors[i].temperature_millicelsius(), servo_monitors[i].power_microwatts());
    }*/
  }
}
