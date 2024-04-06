#include "ADS1256.h"
#include "StateMachine.h"
#include <inttypes.h>
#include <time.h>
#include <Wire.h>
#include <QNEthernet.h>
#include <Adafruit_MAX31856.h>

/*======== ADC ========*/

const int CS1 = 10;
const int DRDY = 2;
const int PDWN = 9;

ADS1256 adc(DRDY, 2000000, PDWN, CS1, 2.5); //DRDY, SPI speed, SYNC(PDWN), CS, VREF(float).
StateMachine machina(2,3,4,5,6,7,1,14,15,16);

long rawConversion = 0; //24-bit raw value
float voltageValue = 0; //human-readable floating point value

int singleEndedChannels[8] = {SING_0, SING_1, SING_2, SING_3, SING_4, SING_5, SING_6, SING_7}; //Array to store the single-ended channels
int inputChannel = 0; //Number used to pick the channel from the above two arrays
char inputMode = ' '; //can be 's' and 'd': single-ended and differential

int pgaValues[7] = {PGA_1, PGA_2, PGA_4, PGA_8, PGA_16, PGA_32, PGA_64}; //Array to store the PGA settings
int pgaSelection = 0; //Number used to pick the PGA value from the above array

const unsigned int fireTime = 5000;
const unsigned int purgeTime = 3000;

int drateValues[16] =
{
  DRATE_30000SPS,
  DRATE_15000SPS,
  DRATE_7500SPS,
  DRATE_3750SPS,
  DRATE_2000SPS,
  DRATE_1000SPS,
  DRATE_500SPS,
  DRATE_100SPS,
  DRATE_60SPS,
  DRATE_50SPS,
  DRATE_30SPS,
  DRATE_25SPS,
  DRATE_15SPS,
  DRATE_10SPS,
  DRATE_5SPS,
  DRATE_2SPS
}; //Array to store the sampling rates

int drateSelection = 0; //Number used to pick the sampling rate from the above array

String registers[11] =
{
  "STATUS",
  "MUX",
  "ADCON",
  "DRATE",
  "IO",
  "OFC0",
  "OFC1",
  "OFC2",
  "FSC0",
  "FSC1",
  "FSC2"
};//Array to store the registers

int registerToRead = 0; //Register number to be read
int registerToWrite = 0; //Register number to be written
int registerValueToWrite = 0; //Value to be written in the selected register

// Thermocouples
// int engineTC_CS = 39;
// int engineTC_DRDY = 38;
// Adafruit_MAX31856 engineTC = Adafruit_MAX31856(engineTC_CS);


//==================Ethernet==================//
const int sensor_number = 8;

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
const int dataPacketSize = sensor_number*4+4+4; 
uint8_t outgoingDataPacketBuffers[dataPacketSize]; // buffer for going out to PC

uint8_t loopCounter = 0;
uint32_t id = 0;
bool valveStateChange = 1;

unsigned long fireDuration = 0;
unsigned long purgeDuration = 0;

void setup() 
{
  Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  Serial.println("Serial available");

  machina.initializeMachina();

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

  // // initialize adc and set gain + data rate
  adc.InitializeADC();
  adc.setPGA(PGA_1);
  adc.setDRATE(DRATE_1000SPS);

  // perform self calibration
  adc.sendDirectCommand(SELFCAL);
  
  Serial.print("PGA: ");
  Serial.println(adc.readRegister(IO_REG));
  delay(100);

  Serial.print("DRATE: ");
  Serial.println(adc.readRegister(DRATE_REG));
  delay(100);

  // Set up thermocouples
  // engineTC.setThermocoupleType(MAX31856_TCTYPE_K);
  // engineTC.setConversionMode(MAX31856_CONTINUOUS);

  //Freeze the display for 1 sec
  delay(1000);
}


// the function converts a 32 bits int array into 8 bit int for udp compatibility
// void bufferConversion32(uint32_t* arr, int size, uint8_t* buffer)
// {
//   uint8_t startByte; // the position to begin shifting; multiple of 4 because there are
//                      // 4 bytes in a 32 bits int

//   // for loop to index next value in 32 bit array
//   for (int i = 0; i<size; i++)
//   {
//     startByte = 4*i;
//     // for loop to shifts 4 bytes individually from int32 
//     for(int j = 0; j<4; j++)
//     {
//       buffer[startByte + j] = (arr[i] >> (8*(3-j))) & 0xFF;
//     }
//   }
// }

void loop() 
{
  uint32_t commandBuffer[2] = {0,0};
  // int command = commandBuffer[1];
  uint32_t valveStates[9] = {2,machina.isok_state,machina.isol_state,machina.maink_state,
                            machina.mainl_state,machina.ventk_state,machina.ventl_state,machina.purge_state,
                            machina.breakWireStatus};

  // convert 32 bit int array into 8 bit int array for udp compatibility
  uint8_t valveStatesBuffer[36];

  // get thermocouple temperature
  // long engineTemp = engineTC.readThermocoupleTemperature(); //need to convert to temperature by multiplying 0.0078125


  if(machina.valveStateChange)
  {
    Serial.println(machina.getState());

    for (int i = 0; i<9; i++)
    {   
      uint8_t startByte = 4*i;

      // for loop to shifts 4 bytes individually from int32 
      for(int j = 0; j<4; j++)
      {
        valveStatesBuffer[startByte + j] = (valveStates[i] >> (8*(3-j))) & 0xFF;
      }
    }

    Udp.send(remote,remotePort,valveStatesBuffer,36);
    machina.valveStateChange = 0;
  }

  packetSize = Udp.parsePacket(); // check to see if we receive any command

  if(packetSize > 0)
  {
    // pointer address to received data array from UDP
    const uint8_t* packetBuffer = Udp.data(); 

    commandBuffer[0] = packetBuffer[3]; 
    commandBuffer[1] = packetBuffer[7]; 
  }
  else
  {}

  for (int i = 0; i<4; i++)
  {
    outgoingDataPacketBuffers[(dataPacketSize-4)+i] = (millis() >> (8*(3-i))) & 0xFF;
  }

  // filling individual readings into an array
  for (int i = 0; i < sensor_number; i++)
  {
    long tempData;
    uint8_t startByte = (i*4) + 4;
    tempData = adc.cycleSingle();
    
    for (int j = 0; j<4; j++)
    {
      outgoingDataPacketBuffers[startByte+j] = (tempData >> (8*(3-j))) & 0xFF;
    }
  }

  for (int j = 0; j<4; j++)
  {
    outgoingDataPacketBuffers[j] = (id >> (8*(3-j))) & 0xFF;
  }

  // send sensor data packet
  Udp.send(remote,remotePort,outgoingDataPacketBuffers,dataPacketSize);

  // activate state machine
  fireDuration = millis() - machina.referenceTime;
  purgeDuration = millis() - machina.referenceTime;
  machina.processCommand(commandBuffer[1], fireTime, purgeTime, fireDuration, purgeDuration);
}
