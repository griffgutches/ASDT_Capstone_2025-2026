#include <Arduino.h>
#include <pins.h>
#include <SoftwareSPI.h>
#include <MS5611_SPI.h>
#include <ISM6HG256XSensor.h>
#include <Adafruit_LIS2MDL.h>
//#include <SDFS.h>
//#include <SparkFun_u-blox_GNSS_v3.h>
#include "led.h"
#include "ina745.h"
#include "microsd.h"

SoftwareSPI softSPI(SPI_SCK, SPI_MISO, SPI_MOSI);

MS5611_SPI baro(MS5607_CS, &softSPI);

ISM6HG256XSensor imu(&softSPI, ISM6HG256X_IMU_CS);

Adafruit_LIS2MDL mag = Adafruit_LIS2MDL();

//SFE_UBLOX_GNSS myGNSS;

LogEntry buffer1[16384 / sizeof(LogEntry)];
LogEntry buffer2[16384 / sizeof(LogEntry)];
LogEntry* currentBuffer = buffer1;
int entryCount = 0;
volatile bool bufferReady = false;
LogEntry* bufferToSave = nullptr;
static ulong last_measurement = 2217;

void setup() {

  pinMode(MS5607_CS, OUTPUT);
  pinMode(ISM6HG256X_IMU_CS, OUTPUT);
  pinMode(RADIO_CS, OUTPUT);

  pinMode(BATTERY_SENSE, INPUT);

  digitalWrite(MS5607_CS, HIGH);
  digitalWrite(ISM6HG256X_IMU_CS, HIGH);
  digitalWrite(RADIO_CS, HIGH);
  baro.setSPIspeed(10000000);

  Serial.begin(9600);
  sleep_ms(2000);
  Serial.println("test");

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

  //
  // if (myGNSS.begin() == false) {
  //   Serial.println(F("u-blox GNSS not detected. Check wiring/address."));
  //   while (1);
  // }
  //
  
}
void setup1() {
  // Initialize SDFS here on Core 1
  if (!SDFS.setConfig(SDFSConfig(SD_CLOCK, SD_CMD, SD_DATA_0).setAutoFormat(true))) {
   Serial.println("SD config error");
  }
  if (!SDFS.begin()) {
   Serial.println("SD error");
  } else {
   Serial.println("SD set up");
  }

}

void loop1() {
  static ulong last_write = 2500;
  if (bufferReady) {
    File f = SDFS.open("data300.bin", "a");
    if (f) {
      f.write((uint8_t*)bufferToSave, 16384);
      f.close();
      //Serial.println("Written to file");
    }
    bufferReady = false; // Reset the flag
  }
}

void loop() {
  //for (int i = 0; i < 6; i++) {
  // int i = 5;
  //   float rgb[3] = { 0 };
  //   hsv2rgb(((float) ((millis() / 2 + i * 80) % 1000)) / 1000.0, 1.0, 0.05, rgb); // control brightness with .125
  //   leds[i] = RGB((uint8_t)(rgb[0] * 255.0), (uint8_t)(rgb[1] * 255.0), (uint8_t)(rgb[2] * 255.0));
  //}
  //leds[LED_STATUS] = RGB(0, 128, 0);
  // led_show();
  // sleep_us(400); 

  static ulong last_measurement_us = 0;
  
  //if (millis() - last_measurement >= 1) {
  if (micros() - last_measurement_us >= 10000) {
    last_measurement_us = micros();
    int e = baro.read();
    if (e != MS5611_READ_OK) {
      Serial.print("barometer error = ");
      Serial.println(e);
    }
    //test_sd_speed();
    currentBuffer[entryCount].ms = last_measurement;
    currentBuffer[entryCount].temp = baro.getTemperature();
    currentBuffer[entryCount].press = baro.getPressure();
    // Serial.print("Barometer\n");
    // Serial.print("T:\t");
    // Serial.print(baro.getTemperature(), 2);
    // Serial.print("\tP:\t");
    // Serial.print(baro.getPressure(), 2);
    // Serial.println();
    // Serial.println();

    //float battery_voltage = ((float)analogRead(BATTERY_SENSE)) / (0.3311 * 4096.0);
    //Serial.printf("Battery voltage = %fV\n", battery_voltage);

    ISM6HG256X_Axes_t accel, rot_rate;
    imu.Get_X_Axes(&accel);
    imu.Get_G_Axes(&rot_rate);
    currentBuffer[entryCount].accX = accel.x;
    currentBuffer[entryCount].accY = accel.y;
    currentBuffer[entryCount].accZ = accel.z;
    currentBuffer[entryCount].gyroX = rot_rate.x;
    currentBuffer[entryCount].gyroY = rot_rate.y;
    currentBuffer[entryCount].gyroZ = rot_rate.z;
    // Serial.println("IMU:");
    // Serial.printf("Acceleration %d %d %d\n", accel.x, accel.y, accel.z);
    // Serial.printf("Rotation Rate %d %d %d\n\n", rot_rate.x, rot_rate.y, rot_rate.z);

    sensors_event_t event;
    mag.getEvent(&event);
    currentBuffer[entryCount].magX = event.magnetic.x;
    currentBuffer[entryCount].magY = event.magnetic.y;
    currentBuffer[entryCount].magZ = event.magnetic.z;
    currentBuffer[entryCount].test = 0xFF;
    //Serial.printf("Magnetometer:\n %f %f %f\n\n", event.magnetic.x, event.magnetic.y, event.magnetic.z);

    // if (myGNSS.getPVT()) {
    // long latitude = myGNSS.getLatitude(); // Degrees * 10^-7
    // long longitude = myGNSS.getLongitude();
    // long altitude = myGNSS.getAltitude(); // mm above ellipsoid
    // byte SIV = myGNSS.getSIV();           // Satellites in View
    // }
    last_measurement ++;
    

    entryCount++;
    if (entryCount >= (16384 / sizeof(LogEntry))) {
      bufferToSave = currentBuffer;
      bufferReady = true;

      // Switch to the other bucket
      currentBuffer = (currentBuffer == buffer1) ? buffer2 : buffer1;
      
      entryCount = 0;
    }
  }
}