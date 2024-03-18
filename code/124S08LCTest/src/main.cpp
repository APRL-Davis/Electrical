#include <Arduino.h>
#include "ads124s08.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Connected...");

  adcStartupRoutine();
}

void loop() {
  Serial.print("ID: ");
  Serial.println(readSingleRegister(REG_ADDR_ID));

  Serial.print("STATUS: ");
  Serial.println(readSingleRegister(REG_ADDR_STATUS));

  delay(5000);
}