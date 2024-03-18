#include <Arduino.h>
#include "ads124s08.h"

#include <SPI.h>

void setup() {
  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }
  
  Serial.println("Connected...");

  InitGPIO();

  SPI.begin();

  adcStartupRoutine();
  stopConversions();

  writeSingleRegister(REG_ADDR_INPMUX, 0b00000001);
  //writeSingleRegister(REG_ADDR_PGA, 0b00001111);
  writeSingleRegister(REG_ADDR_DATARATE, 0b00001011);

  Serial.print("pga: ");
  Serial.println(readSingleRegister(REG_ADDR_PGA));
  Serial.print("datarate: ");
  Serial.println(readSingleRegister(REG_ADDR_DATARATE));

  delay(5000);
  startConversions();
  enableDRDYinterrupt(true);
}

void loop() {
  // Serial.print("ID: ");
  // Serial.println(readSingleRegister(REG_ADDR_ID));

  // Serial.print("STATUS: ");
  // Serial.println(readSingleRegister(REG_ADDR_STATUS));

  // delay(5000);

  if(waitForDRDYHtoL(10000)){
    Serial.print("Time: ");
    Serial.println(millis());
    Serial.print("Reading: ");
    Serial.println(readConvertedData(NULL, COMMAND));
  }
}