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
  //set pga enabled, gain 128 TODO: Check if conversion needs to be active to set pga register
  writeSingleRegister(REG_ADDR_PGA, ADS_PGA_ENABLED|ADS_GAIN_128);
  //set drate to 800
  writeSingleRegister(REG_ADDR_DATARATE, ADS_DR_800);

  #ifdef DEBUG
  Serial.print("pga: ");
  Serial.println(readSingleRegister(REG_ADDR_PGA));
  Serial.print("datarate: ");
  Serial.println(readSingleRegister(REG_ADDR_DATARATE));
  #endif

  delay(10); //allow adc time to settle/config
  startConversions();
  enableDRDYinterrupt(true);
}

void loop() {

  if(waitForDRDYHtoL(10000)){
    Serial.print("Time: ");
    Serial.println(millis());
    Serial.print("Reading: ");
    Serial.println(readConvertedData(NULL, COMMAND));
  }
}