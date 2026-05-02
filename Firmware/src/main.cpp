//#include <Arduino.h>
#include <pins.h>
#include <SPI.h>
#include <MS5611_SPI.h>
#include <ISM6HG256XSensor.h>
//#include <SparkFun_u-blox_GNSS_v3.h>
#include "led.h"
#include "ina745.h"
#include "microsd.h"

SPIClass mySPI(HSPI);

MS5611_SPI baro(MS5607_CS, &mySPI);

// ISM6HG256XSensor imu(&mySPI, ISM6HG256X_IMU_CS);

//SFE_UBLOX_GNSS myGNSS;
void Sensorscode(void * parameter);
void Storingcode(void * parameter);

TaskHandle_t Sensors;
TaskHandle_t Storing;

LogEntry buffer1[16384 / sizeof(LogEntry)];
LogEntry buffer2[16384 / sizeof(LogEntry)];
LogEntry* currentBuffer = buffer1;
int entryCount = 0;
volatile bool bufferReady = false;
LogEntry* bufferToSave = nullptr;
//static ulong last_measurement = 2217;

void setup() {

  mySPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  pinMode(MS5607_CS, OUTPUT);
  pinMode(BMP390_CS, OUTPUT);
  pinMode(ISM6HG256X_IMU_CS, OUTPUT);
  pinMode(BMI088_ACCEL_CS, OUTPUT);
  pinMode(BMI088_GYRO_CS, OUTPUT);
  pinMode(ADXL375_IMU_CS, OUTPUT);
  pinMode(RADIO_CS, OUTPUT);
  pinMode(MAG_CS, OUTPUT);

  pinMode(BATTERY_SENSE, INPUT);

  digitalWrite(MS5607_CS, HIGH);
  digitalWrite(BMP390_CS, HIGH);
  digitalWrite(ISM6HG256X_IMU_CS, HIGH);
  digitalWrite(BMI088_ACCEL_CS, HIGH);
  digitalWrite(BMI088_GYRO_CS, HIGH);
  digitalWrite(ADXL375_IMU_CS, HIGH);
  digitalWrite(RADIO_CS, HIGH);
  digitalWrite(MAG_CS, HIGH);
  
  baro.setSPIspeed(10000000);



  Serial.begin(9600);
  delay(2000);         // delay(2000);   // Replace sleep_ms()for ESP32
  Serial.println("test");



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

  // int imu_status = imu.begin();
  // if (imu_status != 0) {
  //   Serial.println("IMU error");
  // }
  // Serial.println("IMU detected");
  // imu.Enable_X();
  // imu.Enable_G();

  // imu.Set_X_FullScale(8);

  Wire.begin(4, 5);      // SDA, SCL
  Wire.setClock(100000); // => 100kHz I2C clock speed


  // Initialize SD here on Core 1
  SD_MMC.setPins(SD_CLOCK, SD_CMD, SD_DATA_0, SD_DATA_1, SD_DATA_2, SD_DATA_3);

  //true for 1-bit, false for 4-bit
  if (!SD_MMC.begin("/sdcard", false)) {
    Serial.println("SD_MMC mount failed");
    return;
  }
  Serial.println("SD_MMC mounted");

  //
  // if (myGNSS.begin() == false) {
  //   Serial.println(F("u-blox GNSS not detected. Check wiring/address."));
  //   while (1);
  // }
  //
  xTaskCreatePinnedToCore(
                    Sensorscode,   /* Task function. */
                    "Sensors",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Sensors,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 
  xTaskCreatePinnedToCore(
                    Storingcode,   /* Task function. */
                    "Storing",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Storing,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */                  

  
}

void loop() {

}

void Sensorscode(void * parameter) {
  while (true) {
    static ulong last_measurement_us = 0;
    if (micros() - last_measurement_us >= 10000) {
      last_measurement_us = micros();
      int e = baro.read();
      if (e != MS5611_READ_OK) {
        Serial.print("barometer error = ");
        Serial.println(e);
      }
      //currentBuffer[entryCount].ms = last_measurement;
      currentBuffer[entryCount].temp = baro.getTemperature();
      currentBuffer[entryCount].press = baro.getPressure();

      //Serial.println(baro.getTemperature(), 2);
      //Serial.println(baro.getPressure(), 2);
      
      //ISM6HG256X_Axes_t accel, rot_rate;
      //imu.Get_X_Axes(&accel);
      //imu.Get_G_Axes(&rot_rate);
      //currentBuffer[entryCount].accX = accel.x;
      //currentBuffer[entryCount].accY = accel.y;
      //currentBuffer[entryCount].accZ = accel.z;
      //currentBuffer[entryCount].gyroX = rot_rate.x;
      //currentBuffer[entryCount].gyroY = rot_rate.y;
      //currentBuffer[entryCount].gyroZ = rot_rate.z;
      currentBuffer[entryCount].test = 0xFF;
      
      //last_measurement ++;
  

      entryCount++;
      if (entryCount >= (16384 / sizeof(LogEntry))) {
        bufferToSave = currentBuffer;
        bufferReady = true;

        // Switch to the other bucket
        currentBuffer = (currentBuffer == buffer1) ? buffer2 : buffer1;
        
        entryCount = 0;
      }
    }
    vTaskDelay(1);
  }
}

void Storingcode(void * parameter) {
  while (true) {
    if (bufferReady) {
      File f = SD_MMC.open("/data500.bin", FILE_APPEND);
      if (f) {
        f.write((uint8_t*)bufferToSave, 16384);
        f.close();
        Serial.println("Written to file");
      }
      bufferReady = false; // Reset the flag
    }
    vTaskDelay(1);
  }
}


//   static ulong last_measurement_us = 0;
  
//   //if (millis() - last_measurement >= 1) {
//   if (micros() - last_measurement_us >= 10000) {
//     last_measurement_us = micros();
//     int e = baro.read();
//     if (e != MS5611_READ_OK) {
//       Serial.print("barometer error = ");
//       Serial.println(e);
//     }
//     //test_sd_speed();
//     currentBuffer[entryCount].ms = last_measurement;
//     currentBuffer[entryCount].temp = baro.getTemperature();
//     currentBuffer[entryCount].press = baro.getPressure();
//     // Serial.print("Barometer\n");
//     // Serial.print("T:\t");
//     // Serial.print(baro.getTemperature(), 2);
//     // Serial.print("\tP:\t");
//     // Serial.print(baro.getPressure(), 2);
//     // Serial.println();
//     // Serial.println();

//     //float battery_voltage = ((float)analogRead(BATTERY_SENSE)) / (0.3311 * 4096.0);
//     //Serial.printf("Battery voltage = %fV\n", battery_voltage);

//     // Add following two lines for ESP32:
//     // analogReadResolution(12);
//     // analogSetAttenuation(ADC_11db);

//     ISM6HG256X_Axes_t accel, rot_rate;
//     imu.Get_X_Axes(&accel);
//     imu.Get_G_Axes(&rot_rate);
//     currentBuffer[entryCount].accX = accel.x;
//     currentBuffer[entryCount].accY = accel.y;
//     currentBuffer[entryCount].accZ = accel.z;
//     currentBuffer[entryCount].gyroX = rot_rate.x;
//     currentBuffer[entryCount].gyroY = rot_rate.y;
//     currentBuffer[entryCount].gyroZ = rot_rate.z;
//     // Serial.println("IMU:");
//     // Serial.printf("Acceleration %d %d %d\n", accel.x, accel.y, accel.z);
//     // Serial.printf("Rotation Rate %d %d %d\n\n", rot_rate.x, rot_rate.y, rot_rate.z);

    
    
//     // currentBuffer[entryCount].magX = event.magnetic.x;
//     // currentBuffer[entryCount].magY = event.magnetic.y;
//     // currentBuffer[entryCount].magZ = event.magnetic.z;
//     currentBuffer[entryCount].test = 0xFF;
//     //Serial.printf("Magnetometer:\n %f %f %f\n\n", event.magnetic.x, event.magnetic.y, event.magnetic.z);

//     // if (myGNSS.getPVT()) {
//     // long latitude = myGNSS.getLatitude(); // Degrees * 10^-7
//     // long longitude = myGNSS.getLongitude();
//     // long altitude = myGNSS.getAltitude(); // mm above ellipsoid
//     // byte SIV = myGNSS.getSIV();           // Satellites in View
//     // }
//     last_measurement ++;
    

//     entryCount++;
//     if (entryCount >= (16384 / sizeof(LogEntry))) {
//       bufferToSave = currentBuffer;
//       bufferReady = true;

//       // Switch to the other bucket
//       currentBuffer = (currentBuffer == buffer1) ? buffer2 : buffer1;
      
//       entryCount = 0;
//     }
//   }
// }