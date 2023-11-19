#include "adc_functions.h"

void initialize(int CS, int START, int DRDY, int _RESET)
{
  
}

// Waiting until data is ready to be convert
volatile int DRDY_state = HIGH;
void waitforDRDY() {
  while (DRDY_state) {
    continue;
  }
  noInterrupts();
  DRDY_state = HIGH;
  interrupts();
}

//Interrupt function
void DRDY_Interuppt() {
  DRDY_state = LOW;
}

long readRegister(uint8_t address)
{
  uint8_t bufr;
  digitalWriteFast(_CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(RREG | regAdress); // send 1st command byte, address of the register
  SPI.transfer(0x00);     // send 2nd command byte, read only one register
  delayMicroseconds(10);
  bufr = SPI.transfer(NOP); // read data of the register
  delayMicroseconds(10);
  digitalWriteFast(_CS, HIGH);
  //digitalWrite(_START, LOW);
  SPI.endTransaction();
  return bufr;
}