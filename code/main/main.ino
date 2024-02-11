#include "ADS1256.h"
#include <iostream>
#include <cmath>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <inttypes.h>

/*======== ADC ========*/

const int CS1 = 10;
const int CS2 = 40;
const int DRDY = 2;
const int SYNC = 9;

ADS1256 adc(DRDY, 2000000, SYNC, CS1, 2.5); //DRDY, SPI speed, SYNC(PDWN), CS, VREF(float).

long rawConversion = 0; //24-bit raw value
float voltageValue = 0; //human-readable floating point value

int singleEndedChannels[8] = {SING_0, SING_1, SING_2, SING_3, SING_4, SING_5, SING_6, SING_7}; //Array to store the single-ended channels
int inputChannel = 0; //Number used to pick the channel from the above two arrays
char inputMode = ' '; //can be 's' and 'd': single-ended and differential

int pgaValues[7] = {PGA_1, PGA_2, PGA_4, PGA_8, PGA_16, PGA_32, PGA_64}; //Array to store the PGA settings
int pgaSelection = 0; //Number used to pick the PGA value from the above array

/*======== Relays ========*/

const int RELAY_1 = 3; // isok
const int RELAY_2 = 4; // isol
const int RELAY_3 = 5; // main k
const int RELAY_4 = 6; // main l
const int RELAY_5 = 7; // vent k
const int RELAY_6 = 8; // vent l

int relayPins[] = {RELAY_1,RELAY_2,RELAY_3,RELAY_4,RELAY_5,RELAY_6};
  
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
static bool calFlag = 0;

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

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);
IPAddress remote(10,0,0,175);
unsigned int localPort = 5000;      // local port to listen on

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

#define SENSOR_READINGS_PER_PACKET 5 // how many reading sets is in 1 udp packet at 50Hz transmission rate.
size_t currentSensorReadingIndex = 0; // index of the current sensor reading
size_t currentPacketID = 0; // index of the current packet

// sensor data structures for transmission. The struct holds a set of reading from 8 sensors with individual ID
// There will be 5 of these structs in 1 UDP packet
typedef struct sensorData{
  time_t timestamp;
  long sensorValue[8];
  uint16_t sensorID[8];
} sensorData;

typedef struct dataPacket{
  time_t timestamp;
  size_t packetID;
  sensorData sensorDataReadings[SENSOR_READINGS_PER_PACKET];
} dataPacket;

// buffers for receiving and sending data
char packetBuffer[3];  // buffer to hold incoming packet,
String replyMessage = "acknowledged";        // a string to send back
// one buffer for sending and one for sensing writing
dataPacket outgoingDataPacketBuffers;

// Structures and settings for sensors
enum sensorType {PRESSURE, THERMOCOUPLE, LOADCELL};
enum sensorSerialType {sstSPI, sstI2C, sstUART};
enum SPIPins {CSPin = 10, SCKPin = 13, MISOPin = 12, MOSIPin = 11};

typedef struct sensorSettings {
  uint32_t speed;
  // SPI: pins[0] = CS, pins[1] = SCK, pins[2] = MISO, pins[3] = MOSI
  // I2C: pins[0] = SDA, pins[1] = SCL
  // UART: pins[0] = RX, pins[1] = TX
  uint8_t pins[4];
  sensorType type;
  sensorSerialType serialType;
  uint16_t sensorID;
} sensorSettings;

// setup sensors
sensorSettings sensors[12] = {
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 1},
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 2},
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 3}
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 4}
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 5}
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 6}
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 7}
  {2000000, {CS1, 13, 12, 11}, PRESSURE, sstSPI, 8}
};

uint8_t loopCounter = 0;

// Timing settings
IntervalTimer sensorTimer;
volatile bool readyToReadSensors = false;
const int sensorInterval = 50000; // in microseconds (f = 20 Hz => T = 50 ms = 50000 us)

void setup() 
{
  Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  while (!Serial)
  {
    ;//Wait until the serial becomes available
  }

  Serial.println("Serial available");

  // start the Ethernet
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start UDP
  Udp.begin(localPort);

  // set relay pinmodes
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);

  Serial.println("Relays initialized...");

  // initialize adc and set gain + data rate
  adc.InitializeADC();
  adc.setPGA(PGA_1);
  adc.setDRATE(DRATE_2000SPS);

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
  recordingTime = millis();
}

void startCheck()
{
  checkState = 1;
  for (int i=0; i<6; i++)
  {
    digitalWrite(relayPins[i],HIGH);
  }
}

void endCheck()
{
  checkState = 0;
  for (int i=0; i<6; i++)
  {
    digitalWrite(relayPins[i],LOW);
  }
}

void pressurize()
{ 
  pressureState = 1; 
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH); 
}

void depressurize()
{
  pressureState = 0;
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW); 
}

// firing sequence
void fire()
{
  fireState = 1;
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
}

void endFire()
{
  fireState = 0;
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
}

// purge sequence
void purge()
{
  purgeState = 1;
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
  digitalWrite(RELAY_5, HIGH);
  digitalWrite(RELAY_6, HIGH);
}

void endPurge()
{
  purgeState = 0;
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
  digitalWrite(RELAY_5, LOW);
  digitalWrite(RELAY_6, LOW);
}

void sendData()
{
  Udp.begin(remote, localPort);
  Udp.write(outgoingDataPacketBuffers);
  Udp.endPacket();
}

void loop() 
{
  uint8_t command;

  // filling individual readings into set of reading sensorData struct
  for (int i = 0; i < 8; i++)
  {
    sensorData.sensorValue[i] = trunc(adc.convertToVoltage(adc.cycleSingle()));
    sensorData.sensorID[i] = i;
  } 
  sensorData.timestamp = millis() - recordingTime;
  
  // filling data packet
  outgoingDataPacketBuffers.sensorDataReadings[loopCounter] = sensorData;
  outgoingDataPacketBuffers.packetID = 69;
  outgoingDataPacketBuffers.timestamp = millis() - recordingTime;
  
  if(loopCounter == 4)
  {
    sendData();
  }

  if(loopCounter < 4)
  {
    loopCounter += 1;
  }
  else
  {
    loopCounter = 0;
  }

  if (Serial.available() > 0)
  { 
    command = Serial.parseInt();  
    if(command == 21)
    {
      adc.sendDirectCommand(SELFCAL);
    }
    if(command == 22)
    {
      //Variables to store and measure elapsed time and define the number of conversions
      long numberOfSamples = 15000; //Number of conversions
      long finishTime = 0;
      long startTime = micros();

      for (long i = 0; i < numberOfSamples; i++)
      {
        adc.readSingleContinuous();            
        //Note: here we just perform the readings and we don't print the results
      }

      finishTime = micros() - startTime; //Calculate the elapsed time

      adc.stopConversion();

      //Printing the results
      Serial.print("Total conversion time for 150k samples: ");
      Serial.print(finishTime);
      Serial.println(" us");

      Serial.print("Sampling rate: ");
      Serial.print(numberOfSamples * (1000000.0 / finishTime), 3);
      Serial.println(" SPS");
    }
    
    if(command == 1) // relay 1 on
    {
      state1 = !state1;
      digitalWrite(RELAY_1, state1);
    }
    if(command == 2) // relay 2 on
    {
      state2 = !state2;
      digitalWrite(RELAY_2, state2);
    }
    if(command == 3) // relay 3 on
    {
      state3 = !state3;
      digitalWrite(RELAY_3, state3);
    }  
    if(command == 4) // relay 4 on
    {
      state4 = !state4;
      digitalWrite(RELAY_4, state4);
    }  
    if(command == 5) // relay 5 on
    {
      state5 = !state5;
      digitalWrite(RELAY_5, state5);
    }
    if(command == 6) // relay 6 on
    {
      state6 = !state6;
      digitalWrite(RELAY_6, state6);
    }
    if(command == 7)
    {
      startCheck();
      previousTime = millis();
    }
    if(command == 8)
    {
      fire(); 
      previousTime = millis();
    }  
    if(command == 9)
    {
      purge();
      previousTime = millis();
    }  
    if(command == 10)
    {
      pressurize();
    }
    else
    {
      Serial.read();
    }
  }                           

  elapsedTime = millis() - previousTime;
  
  if(checkState == 1 && elapsedTime >= checkTime)
  {
    endCheck();
    previousTime = millis();
  }

  if(fireState == 1 && elapsedTime >= fireTime)
  {
    endFire();    
    previousTime = millis();
    purge();
  }

  if(purgeState == 1 && elapsedTime >= purgeTime)
  { 
    endPurge();
    previousTime = millis();
  }
}
