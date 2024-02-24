#include <Arduino.h>
#include <iostream>
#include <cmath>
#include <inttypes.h>
#include <QNEthernet.h>

using namespace qindesign::network;
EthernetUDP udp;

unsigned long previous = 0;
unsigned long duration = 0;
long tempValue;
long previousTime = 0;

//==================Etherne==================//
// how many reading sets is in 1 udp packet at 25 Hz transmission rate.
// packet size based on sampling rate, number of sensors, and udp transmission frequency
const int SENSOR_READINGS_PER_PACKET = 50; // fixed size. Send as soon as packet is filled.
const int sensorNumber = 8;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x04, 0xE9, 0xE5, 0x13, 0x0C, 0x74
};
IPAddress ip(10, 0, 0, 69);     // MCU IP
IPAddress subnet(255,255,255,0); // set subnet mask
IPAddress remote(10,0,0,175);    // PC IP
unsigned int localPort = 1682;     // local port to listen on

//outgoing packet
long packetOut [9];

// buffers for receiving and sending data
char packetBuffer[32];  // buffer to hold incoming packet from PC

uint8_t loopCounter = 0;
int packetSize;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  while (!Serial)
  {
    ;//Wait until the serial becomes available
  }

  Serial.println("Serial available");

  // start the Ethernet
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }

  // start udp
  udp.beginWithReuse(localPort);
  previous = millis();
}

void loop() {

  packetSize = udp.parsePacket();
  if(packetSize>0)
  {
    for (int i = 0; i < 8; i++)
    {
      tempValue = random(4,5000000);
      packetOut[i] = tempValue;        
    }

    duration = millis() - previousTime;
    packetOut[8] = duration;
  }
  
  uint8_t* ptr = &packetOut;

  udp.send(remote,localPort,ptr,9); 

}
