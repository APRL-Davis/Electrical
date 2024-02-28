#include <Arduino.h>
#include "ads124s08.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Connected...");

  adcStartupRoutine();
}

void loop() {
  // put your main code here, to run repeatedly:
}