#include <SPI.h>  // SPI coms
#include <Arduino.h> 
#include <Streaming.h>
#include "ads1256.h"
#include "ads12xx.h"
#include "adc_functions.h"

#define NOP __ams__("nop\n\t")

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  //We will need high datarate, so it should be a high baud rate
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
