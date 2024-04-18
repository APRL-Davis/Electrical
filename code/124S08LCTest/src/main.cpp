#include <Arduino.h>
#include "ads124s08.h"

#include <SPI.h>

#include <QNEthernet.h>

//==================Ethernet==================//
// The IP address will be dependent on your local network:
IPAddress ip(192,168,88,247);     // MCU IP
IPAddress subnet(255,255,255,0); // set subnet mask
// IPAddress remote(10,0,0,51);    // PC IP
IPAddress remote(192,168,88,251);
unsigned int localPort = 1683;     // local port to listen on
unsigned int remotePort = 1683;

// An EthernetUDP instance to let us send and receive packets over UDP
using namespace qindesign::network;
EthernetUDP Udp;

int packetSize = 0;

//each reading is 4 byte
//individual reading timestamp 4 byte
//id 1 byte
//timestamp 4 byte
const int sensor_number = 4;

const int dataPacketSize = sensor_number*4+4+4; 
uint8_t outgoingDataPacketBuffers[dataPacketSize]; // buffer for going out to PC

const int packetID = 7;

void setup() {
  Serial.begin(115200);
  while (!Serial)
  {
    ;
  }
  
  Serial.println("Connected...");

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

  InitGPIO();

  SPI.begin();

  adcStartupRoutine();
  stopConversions();

  writeSingleRegister(REG_ADDR_INPMUX, 0b00000001);

  //set pga enabled, gain 128 TODO: Check if conversion needs to be active to set pga register
  writeSingleRegister(REG_ADDR_PGA, (ADS_PGA_ENABLED|ADS_GAIN_128));
  //set drate to 800
  writeSingleRegister(REG_ADDR_DATARATE, ADS_DR_800);

  // disable reference buffers
  writeSingleRegister(REG_ADDR_REF, (ADS_REFP_BYP_DISABLE | ADS_REFN_BYP_DISABLE));

  #ifdef DEBUG
  Serial.print("pga: ");
  Serial.println(readSingleRegister(REG_ADDR_PGA));
  Serial.print("datarate: ");
  Serial.println(readSingleRegister(REG_ADDR_DATARATE));
  #endif

  delay(10); //allow adc time to settle/config
  startConversions();
  delay(10); //allow adc time to settle/config
  sendCommand(OPCODE_SFOCAL);
  delay(10); //allow adc time to settle/config
  stopConversions();
  enableDRDYinterrupt(true);
}

int lastLoop = 0;

void loop() {

  // if(waitForDRDYHtoL(10000)){
  //   Serial.print("Time: ");
  //   Serial.println(millis());
  //   Serial.print("Reading: ");
  //   Serial.println(readConvertedData(NULL, COMMAND));
  // }

  for (int i = 0; i<4; i++)
  {
    outgoingDataPacketBuffers[(dataPacketSize-4)+i] = (millis() >> (8*(3-i))) & 0xFF;
  }

  // filling individual readings into an array
  for (int i = 0; i < sensor_number; i++)
  {
    long tempData;
    uint8_t startByte = (i*4) + 4;

    if(i == 0){
      writeSingleRegister(REG_ADDR_INPMUX, ADS_P_AIN0 | ADS_N_AIN1);
      startConversions();
      if(waitForDRDYHtoL(1000)){
        tempData = readConvertedData(NULL, COMMAND);
        stopConversions();
        Serial.print(">lc1 read:");
        Serial.println(tempData);
      }
    }
    else if(i == 1){
      writeSingleRegister(REG_ADDR_INPMUX, ADS_P_AIN2 | ADS_N_AIN3);
      startConversions();
      if(waitForDRDYHtoL(1000)){
        tempData = readConvertedData(NULL, COMMAND);
        stopConversions();
        Serial.print(">lc2 read:");
        Serial.println(tempData);
      }
    }
    else if(i == 2){
      writeSingleRegister(REG_ADDR_INPMUX, ADS_P_AIN4 | ADS_N_AIN5);
      startConversions();
      if(waitForDRDYHtoL(1000)){
        tempData = readConvertedData(NULL, COMMAND);
        stopConversions();
        Serial.print(">lc3 read:");
        Serial.println(tempData);
      }
    }
    else if(i == 3){
      writeSingleRegister(REG_ADDR_INPMUX, ADS_P_AIN6 | ADS_N_AIN7);
      startConversions();
      if(waitForDRDYHtoL(1000)){
        tempData = readConvertedData(NULL, COMMAND);
        stopConversions();
        Serial.print(">lc4 read:");
        Serial.println(tempData);
      }
    }
    // tempData = random(100);
    
    for (int j = 0; j<4; j++)
    {
      outgoingDataPacketBuffers[startByte+j] = (tempData >> (8*(3-j))) & 0xFF;
    }
  }

  for (int j = 0; j<4; j++)
  {
    outgoingDataPacketBuffers[j] = (packetID >> (8*(3-j))) & 0xFF;
  }

  // send sensor data packet
  Udp.send(remote,remotePort,outgoingDataPacketBuffers,dataPacketSize);

  //Serial.println(millis()-lastLoop);
  //lastLoop = millis();
}