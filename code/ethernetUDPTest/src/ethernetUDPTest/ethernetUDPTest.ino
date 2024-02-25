#include <Arduino.h>
#include <iostream>
#include <cmath>
#include <inttypes.h>
#include <QNEthernet.h>

using namespace qindesign::network;
EthernetUDP udp;

uint8_t tempValue;

//==================Ethernet==================//
// how many reading sets is in 1 udp packet at 25 Hz transmission rate.
// packet size based on sampling rate, number of sensors, and udp transmission frequency
const int SENSOR_READINGS_PER_PACKET = 50; // fixed size. Send as soon as packet is filled.
const int sensorNumber = 8;

// The IP address will be dependent on your local network:
IPAddress ip(10, 0, 0, 69);     // MCU IP
IPAddress subnet(255,255,255,0); // set subnet mask
IPAddress remote(10,0,0,51);    // PC IP
unsigned int localPort = 1682;     // local port to listen on

//outgoing packet
uint8_t packetOut [32];

// buffers for receiving and sending data
char packetBuffer[32];  // buffer to hold incoming packet from PC

int packetSize;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  while (!Serial)
  {
    ;//Wait until the serial becomes available
  }

  Serial.println("Serial available");
  
  memset(packetOut, 0, 32);

  // Check for Ethernet hardware present
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    printf("[Ethernet] Link %s\r\n", state ? "ON" : "OFF");
  });

  // start udp
  udp.beginWithReuse(localPort);
}

void loop() {

  for (int i = 0; i < 32; i++)
  {
    tempValue = random(0,169);
    packetOut[i] = tempValue;        
  }

  udp.send(remote, localPort, packetOut,32); 

  delay(20);

}
