#pragma once

#include <SDFS.h>
#include <Arduino.h>
#include "pins.h"

struct __attribute__((packed)) LogEntry {
  uint32_t ms;
  float temp;
  float press;
  int16_t accX, accY, accZ;
  int16_t gyroX, gyroY, gyroZ;
  int16_t magX, magY, magZ;
  int16_t test;
};

void test_sd_speed();