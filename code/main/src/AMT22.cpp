#include <Arduino.h>
#include <SPI.h>

#include "AMT22.h"

AMT22::AMT22(uint8_t CS, RES_Type_t resType) {
  this->CS = CS;
  this->resType = resType;

  lastGoodRead = 0;
  badReadCounter = 0;
  encoderHealthy = true;

  SPI.begin();
  pinMode(CS, OUTPUT);

  // Get the CS line high (default inactive state)
  digitalWrite(CS, HIGH);

  // Set SPI clock rate to 2 MHz (max for AMT)
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
}

void AMT22::setCSLine(uint8_t pinState) {
  digitalWrite(CS, pinState);
}

/**
* This function does the SPI transfer.
* 
* @param byte the byte to send
* @param releaseLine whether or not to release the chip
                     select line after sending
* 
* @returns the received data from the transaction
*/
uint8_t AMT22::spiWriteRead(uint8_t byte, uint8_t releaseLine) {
  uint8_t data;

  setCSLine(LOW);

  // Minimum time delay after CS goes low before data is shifted from encoder
  delayMicroseconds(3);

  // Send the command
  data = SPI.transfer(byte);

  delayMicroseconds(3); // hold CS line low for minimum time requirement
  setCSLine(releaseLine);

  return data;
}

bool AMT22::isHealthy() {
  return encoderHealthy;
}

/**
* This function gets the absolute position
* from the AMT22 encoder using the SPI bus.
* Returns the last valid position if we read
* bad data.
*/
uint16_t AMT22::getPosition() {
  uint16_t currentPosition;
  uint8_t evenCheckbit;
  uint8_t oddCheckbit;
  uint8_t evenChecksum;
  uint8_t oddChecksum;

  // Get high byte, shift up by 8, don't release the CS line
  currentPosition = spiWriteRead(AMT22_NOP, false) << 8;

  // Time required between bytes
  delayMicroseconds(3);

  // Get low byte, release the CS line
  currentPosition |= spiWriteRead(AMT22_NOP, true);

  evenCheckbit = (currentPosition & (0x01 << 14)) >> 14;
  oddCheckbit = (currentPosition & (0x01 << 15)) >> 15;

  evenChecksum = currentPosition & 0x01;
  oddChecksum = (currentPosition & (0x01 << 1)) >> 1;

  for (uint8_t i = 2; i < 14; i++) {
    if ((i % 2) == 0) evenChecksum ^= (currentPosition & (0x01 << i)) >> i;
    else oddChecksum ^= (currentPosition & (0x01 << i)) >> i;
  }

  evenChecksum = ~evenChecksum & 0x01;
  oddChecksum = ~oddChecksum & 0x01;

  if ((evenCheckbit == evenChecksum) && (oddCheckbit == oddChecksum)) {
    // We got back a good position, mask away checkbits
    currentPosition &= 0x3FFF;

    // Reset the counter, previous bad read was a random occurrence
    badReadCounter = 0;
  }
  else {
    if (encoderHealthy) badReadCounter++;
    if (badReadCounter >= 200) encoderHealthy = false;

    return lastGoodRead; // bad read, return the last valid position
  }

  // If resolution is 12 bits, shift bits
  if (resType == RES_Type_t::RES12) currentPosition = currentPosition >> 2;

  lastGoodRead = currentPosition;
  return currentPosition;
}

/**
* This function zeros the encoder.
*/
void AMT22::zeroAMT22() {
  spiWriteRead(AMT22_NOP, false);

  // Time required between bytes
  delayMicroseconds(3);

  spiWriteRead(AMT22_ZERO, true);
  delay(250); // 250 second delay to allow the encoder to reset
}

/**
* This function resets the encoder.
*/
void AMT22::resetAMT22() {
  spiWriteRead(AMT22_NOP, false);

  // Time required between bytes
  delayMicroseconds(3);

  spiWriteRead(AMT22_RESET, true);
  delay(250); // 250 second delay to allow the encoder to start back up
}