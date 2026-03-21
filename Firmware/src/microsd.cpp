#include <microsd.h>


void test_sd_speed(){
const uint32_t totalSize = 5 * 1024 * 1024; // 5MB Test
  const uint32_t bufSize = 16384;            // 16KB Buffer for speed
  uint8_t* buf = (uint8_t*)malloc(bufSize);
  if (buf) memset(buf, 0xAA, bufSize);

  File testFile = SD_MMC.open("/bench.bin", FILE_WRITE);
  if (!testFile) {
    Serial.println("Failed to create file.");
    return;
  }

  Serial.println("Writing 5MB...");
  uint32_t startTime = millis();
  uint32_t bytesWritten = 0;

  while (bytesWritten < totalSize) {
    uint32_t w = testFile.write(buf, bufSize);
    if (w == 0) break;
    bytesWritten += w;
  }

  testFile.flush();
  uint32_t endTime = millis();
  testFile.close();
  free(buf);

  // 4. Calculate Results
  float duration = (endTime - startTime) / 1000.0;
  float mbps = (totalSize / 1024.0 / 1024.0) / duration;

  Serial.print("Time: "); Serial.print(duration); Serial.println(" s");
  Serial.print("Sustained Write Speed: "); Serial.print(mbps); Serial.println(" MB/s");
}