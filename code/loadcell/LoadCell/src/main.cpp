#include <Arduino.h>
#include <QNEthernet.h>

// ADC
const int CS = 10; 
const int DRDY = 2; 
const int PDWN = 9;

ADS1248 adc(/* parameters */);

// Ethernet

const int sensorNumber = 4;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
IPAddress ip(10, 0, 0, 69);     // MCU IP
IPAddress subnet(255,255,255,0); // set subnet mask
// IPAddress remote(10,0,0,51);    // PC IP
IPAddress remote(10,0,0,51);
unsigned int localPort = 1682;     // local port to listen on
unsigned int remotePort = 1682;

// An EthernetUDP instance to let us send and receive packets over UDP
using namespace qindesign::network;
EthernetUDP Udp;

int packetSize = 0;

// buffers for receiving and sending data
//each reading is 4 byte
//individual reading timestamp 4 byte
//id 1 byte
//timestamp 4 byte
const int dataPacketSize = sensorNumber*4+4; 
uint8_t outgoingDataPacketBuffers[dataPacketSize]; // buffer for going out to PC

void setup() {
   Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  Serial.println("Serial available");

  // put your setup code here, to run once:
  memset(outgoingDataPacketBuffers, 0, dataPacketSize);

  // Check for Ethernet hardware present
  if (!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }

  // Listen for link changes
  Ethernet.onLinkState([](bool state) {
    printf("[Ethernet] Link %s\r\n", state ? "ON" : "OFF");
  });

  // start UDP
  Udp.beginWithReuse(localPort);

  //Freeze the display for 1 sec
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}