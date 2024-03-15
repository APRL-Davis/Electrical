#include "ADS1256.h"
#include <iostream>
#include <cmath>
#include <inttypes.h>
#include <time.h>
#include <QNEthernet.h>

/*======== ADC ========*/

const int CS1 = 10;
const int CS2 = 40;
const int DRDY = 2;
const int PDWN = 9;

ADS1256 adc(DRDY, 2000000, PDWN, CS1, 2.5); //DRDY, SPI speed, SYNC(PDWN), CS, VREF(float).

long rawConversion = 0; //24-bit raw value
float voltageValue = 0; //human-readable floating point value

int singleEndedChannels[8] = {SING_0, SING_1, SING_2, SING_3, SING_4, SING_5, SING_6, SING_7}; //Array to store the single-ended channels
int inputChannel = 0; //Number used to pick the channel from the above two arrays
char inputMode = ' '; //can be 's' and 'd': single-ended and differential

int pgaValues[7] = {PGA_1, PGA_2, PGA_4, PGA_8, PGA_16, PGA_32, PGA_64}; //Array to store the PGA settings
int pgaSelection = 0; //Number used to pick the PGA value from the above array

/*===== TVC Encoders =====*/

const int X_ENCODER_CS = 0;
const int Y_ENCODER_CS = 1;

AMT22 xEncoder(X_ENCODER_CS, RES_Type_t::RES14); // CS, Resolution (14-bit or 12-bit)
AMT22 yEncoder(Y_ENCODER_CS, RES_Type_t::RES14);

uint16_t xEncoderPosition = 0;
uint16_t yEncoderPosition = 0;

bool xEncoderFaultFlag = 0;
bool yEncoderFaultFlag = 0;

/*======== Relays ========*/

const int RELAY_1 = 3; // isok
const int RELAY_2 = 4; // isol
const int RELAY_3 = 5; // main k
const int RELAY_4 = 6; // main l
const int RELAY_5 = 7; // vent k
const int RELAY_6 = 8; // vent l
const int RELAY_7 = 14; //purge

int relayPins[] = {RELAY_1,RELAY_2,RELAY_3,RELAY_4,RELAY_5,RELAY_6,RELAY_7};
  
unsigned long previousTime;
unsigned long currentMillis;
unsigned long elapsedTime;

// interrupt pins
volatile bool fireState;
volatile bool purgeState;
volatile bool pressureState;
volatile bool checkState;

// relay states/
static bool state1 = 0;
static bool state2 = 0;
static bool state3 = 0;
static bool state4 = 0;
static bool state5 = 0;
static bool state6 = 0;
static bool state7 = 0;

const unsigned int fireTime = 5000;
const unsigned int purgeTime = 3000;
const unsigned int checkTime = 2000;

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

//==================Etherne==================//
// how many reading sets is in 1 udp packet at 25 Hz transmission rate.
// packet size based on sampling rate, number of sensors, and udp transmission frequency
const int SENSOR_READINGS_PER_PACKET = 50; // fixed size. Send as soon as packet is filled.
const int sensor_number = 8;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
IPAddress ip(10, 0, 0, 69);     // MCU IP
IPAddress subnet(255,255,255,0); // set subnet mask
IPAddress remote(10,0,0,51);    // PC IP
unsigned int localPort = 1682;     // local port to listen on

// An EthernetUDP instance to let us send and receive packets over UDP
using namespace qindesign::network;
EthernetUDP Udp;

int packetSize = 0;

// buffers for receiving and sending data
//each reading is 4 byte
//individual reading timestamp 4 byte
//id 1 byte
//timestamp 4 byte
const int dataPacketSize = sensor_number*4+4+4+4; 
uint8_t outgoingDataPacketBuffers[dataPacketSize]; // buffer for going out to PC

uint8_t loopCounter = 0;
uint32_t id = 0;
uint32_t previousValveStates[8] = {2,0,0,0,0,0,0,1};

void setup() 
{
  Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  Serial.println("Serial available");

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

  // set relay pinmodes
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);
  pinMode(RELAY_7, OUTPUT);

  Serial.println("Relays initialized...");

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

  //Freeze the display for 1 sec
  delay(1000);
}

void startCheck()
{
  checkState = 1;
  for (int i=0; i<7; i++)
  {
    digitalWriteFast(relayPins[i],HIGH);
  }
  state1 = 1;
  state2 = 1;
  state3 = 1;
  state4 = 1;
  state5 = 1;
  state6 = 1;
  state7 = 1;
}

void endCheck()
{
  checkState = 0;
  for (int i=0; i<7; i++)
  {
    digitalWriteFast(relayPins[i],LOW);
  }
  state1 = 0;
  state2 = 0;
  state3 = 0;
  state4 = 0;
  state5 = 0;
  state6 = 0;
  state7 = 0;
}

void pressurize()
{ 
  pressureState = 1; 
  state1 = 1;
  state2 = 1;
  state5 = 1;
  state6 = 1;
  digitalWriteFast(RELAY_1, HIGH);
  digitalWriteFast(RELAY_2, HIGH); 
  digitalWriteFast(RELAY_5, HIGH);
  digitalWriteFast(RELAY_6, HIGH);
}

void depressurize()
{
  pressureState = 0;
  state1 = 0;
  state2 = 0;
  state5 = 0;
  state6 = 0;
  digitalWriteFast(RELAY_1, LOW);
  digitalWriteFast(RELAY_2, LOW);
  digitalWriteFast(RELAY_5, LOW);
  digitalWriteFast(RELAY_6, LOW); 
}

// firing sequence
void fire()
{
  fireState = 1;
  state3 = 1;
  state4 = 1;
  digitalWriteFast(RELAY_3, HIGH);
  digitalWriteFast(RELAY_4, HIGH);
}

void endFire()
{
  fireState = 0;
  state3 = 0;
  state4 = 0;
  digitalWriteFast(RELAY_3, LOW);
  digitalWriteFast(RELAY_4, LOW);
}

// purge sequence
void purge()
{
  purgeState = 1;
  state7 = 1;
  digitalWriteFast(RELAY_7, HIGH);
}

void endPurge()
{
  purgeState = 0;
  state7 = 0;
  digitalWriteFast(RELAY_7, LOW);
}

// the function converts a 32 bits int array into 8 bit int for udp compatibility
uint8_t* bufferConversion32(uint32_t arr[], int size, uint8_t* buffer)
{
  uint8_t startByte; // the position to begin shifting; multiple of 4 because there are
                     // 4 bytes in a 32 bits int

  // for loop to index next value in 32 bit array
  for (int i = 0; i<size; i++)
  {
    uint32_t tempValue = arr[i];
    startByte = 4*i;
    // for loop to shifts 4 bytes individually from int32 
    for(int j = 0; j<4; j++)
    {
      buffer[startByte + j] = (tempValue >> (8*(3-j))) & 0xFF;
    }
  }

  return buffer;
}

void loop() 
{
  uint32_t command[2] = {0,0};
  uint32_t valveStates[8] = {2,state1,state2,state3,state4,state5,state6,state7};
  bool valveStateChange = 0;
  int sumCurrentState = 0;
  int sumPreviousState = 0;

  // check to see if there is any valve state changes
  for (int i = 0; i<8; i++)
  {
    sumCurrentState = sumCurrentState + valveStates[i];
    sumPreviousState = sumPreviousState + previousValveStates[i]; 
  }

  if(sumCurrentState == sumPreviousState)
  {
    valveStateChange = 0;
  }
  else
  {
    valveStateChange = 1;
  }

  // convert 32 bit int array into 8 bit int array for udp compatibility
  uint8_t *valveStatesBuffer = new uint8_t[32];
  valveStatesBuffer = bufferConversion32(valveStates,8,valveStatesBuffer);

  // if detects changes, send the new set of states
  if(valveStateChange)
  {
    for(int i = 0; i<8; i++)
    {
      Serial.print(valveStates[i]);
      Serial.print(" ");
    }
    Serial.println("");
    Udp.send(remote,localPort,valveStatesBuffer,32);
  }
  // update the changes for future comparisons
  for (int i = 0; i<8; i++)
  {
    previousValveStates[i] = valveStates[i];
  }

  packetSize = Udp.parsePacket(); // check to see if we receive any command

  if(packetSize > 0)
  {
    // pointer address to received data array from UDP
    const uint8_t* packetBuffer = Udp.data(); 

    command[0] = packetBuffer[3]; 
    command[1] = packetBuffer[7]; 
    Serial.print(command[1]);
    Serial.print(" ");
    Serial.println(packetSize);
  }
  else
  {}

  for (int i = 0; i<4; i++)
  {
    outgoingDataPacketBuffers[36+i] = (millis() >> (8*(3-i))) & 0xFF;
  }

  // filling individual readings into an array
  for (int i = 0; i < sensor_number; i++)
  {
    uint8_t startByte = (i*4) + 4;
    long tempData = adc.cycleSingle();
    for (int j = 0; j<4; j++)
    {
      outgoingDataPacketBuffers[startByte+j] = (tempData >> (8*(3-j))) & 0xFF;
    }
  }

  xEncoderPosition = xEncoder.getPosition();
  for (int j = 0; j < 2; j++) {
    outgoingDataPacketBuffers[36 + j] = (xEncoderPosition >> (8 * (3 - j))) & 0xFF;
  }

  yEncoderPosition = yEncoder.getPosition();
  for (int j = 0; j < 2; j++) {
    outgoingDataPacketBuffers[38 + j] = (yEncoderPosition >> (8 * (3 - j))) & 0xFF;
  }

  for (int j = 0; j<4; j++)
  {
    outgoingDataPacketBuffers[j] = (id >> (8*(3-j))) & 0xFF;
  }

  Udp.send(remote,localPort,outgoingDataPacketBuffers,dataPacketSize);
    
  if(command[1] == 1) // relay 1 on
  {
    state1 = !state1;
    digitalWriteFast(RELAY_1, state1);
  }
  else if(command[1] == 2) // relay 2 on
  {
    state2 = !state2;
    digitalWriteFast(RELAY_2, state2);
  }
  else if(command[1] == 3) // relay 3 on
  {
    state3 = !state3;
    digitalWriteFast(RELAY_3, state3);
  }  
  else if(command[1] == 4) // relay 4 on
  {
    state4 = !state4;
    digitalWriteFast(RELAY_4, state4);
  }  
  else if(command[1] == 5) // relay 5 on
  {
    state5 = !state5;
    digitalWriteFast(RELAY_5, state5);
  }
  else if(command[1] == 6) // relay 6 on
  {
    state6 = !state6;
    digitalWriteFast(RELAY_6, state6);
  }
  else if(command[1] == 7)
  {
    state7 = !state7;
    digitalWriteFast(RELAY_7, state7);
  }
  else if(command[1] == 11)
  {
    startCheck();
    previousTime = millis();
  }
  else if(command[1] == 12)
  {
    pressurize();
  }
  else if(command[1] == 13)
  {
    purge();
    previousTime = millis();
  }  
  else if(command[1] == 14)
  {
    fire(); 
    previousTime = millis();
  }
  else if(command[1] == 15)
  {
    depressurize(); 
  }             
  else{}   

  elapsedTime = millis() - previousTime;
  
  if(checkState == 1 && elapsedTime >= checkTime)
  {
    endCheck();
    previousTime = millis();
  }
  
  elapsedTime = millis() - previousTime;

  if(fireState == 1 && elapsedTime >= fireTime)
  {
    endFire();    
    previousTime = millis();
  }
  
  elapsedTime = millis() - previousTime;

  if(purgeState == 1 && elapsedTime >= purgeTime)
  { 
    endPurge();
    previousTime = millis();
  }
}
