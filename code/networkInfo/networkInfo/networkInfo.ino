#include "Arduino.h"
#include <TeensyID.h>

uint8_t serial[4];
uint8_t mac[6];
uint8_t uid64[8];

void setup() {
  Serial.begin(9600);
  delay(2000);
  teensySN(serial);
  teensyMAC(mac);
  teensyUID64(uid64);
  Serial.printf("USB Serialnumber: %u \n", teensyUsbSN());
  Serial.printf("Array Serialnumber: %02X-%02X-%02X-%02X \n", serial[0], serial[1], serial[2], serial[3]);
  Serial.printf("String Serialnumber: %s\n", teensySN());
  Serial.printf("Array MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("String MAC Address: %s\n", teensyMAC());
  Serial.printf("UID 64-bit: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n", uid64[0], uid64[1], uid64[2], uid64[3], uid64[4], uid64[5], uid64[6], uid64[7]);
  Serial.printf("UID 64-bit: %s\n", teensyUID64());
  }

void loop() {}
