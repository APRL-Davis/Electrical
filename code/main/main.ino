#include "ADS1256.h"

/*======== ADC ========*/

ADS1256 adc(2, 2500000, 9, 10, 2.500); //DRDY, SPI speed, SYNC(PDWN), CS, VREF(float).

long rawConversion = 0; //24-bit raw value
float voltageValue = 0; //human-readable floating point value

int singleEndedChannels[8] = {SING_0, SING_1, SING_2, SING_3, SING_4, SING_5, SING_6, SING_7}; //Array to store the single-ended channels
int inputChannel = 0; //Number used to pick the channel from the above two arrays
char inputMode = ' '; //can be 's' and 'd': single-ended and differential

int pgaValues[7] = {PGA_1, PGA_2, PGA_4, PGA_8, PGA_16, PGA_32, PGA_64}; //Array to store the PGA settings
int pgaSelection = 0; //Number used to pick the PGA value from the above array

/*======== Relays ========*/

const int RELAY_1 = 2;
const int RELAY_2 = 3;
const int RELAY_3 = 4;
const int RELAY_4 = 5;
const int RELAY_5 = 6;
const int RELAY_6 = 7;

int relayPins[] = {RELAY_1,RELAY_2,RELAY_3,RELAY_4,RELAY_5,RELAY_6};

unsigned long previousTime;
unsigned long currentMillis;
unsigned long elapsedTime;

// interrupt pins
volatile bool fireState;
volatile bool purgeState;
volatile bool isolationState;

// relay states/
static bool state1 = 0;
static bool state2 = 0;
static bool state3 = 0;
static bool state4 = 0;
static bool state5 = 0;
static bool state6 = 0;

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

void setup() 
{
  Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  while (!Serial)
  {
    ; //Wait until the serial becomes available
  }

  Serial.println("Serial available");

  // set relay pinmodes
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);

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

  //Freeze the display for 3 sec
  delay(3000);
}

void loop() 
{
  currentMillis = millis();

  for (int i = 0; i < 8; i++)
  {
    Serial.print(adc.cycleSingle(), 4); //print the converted single-ended results with 4 digits
    Serial.print(","); //comma to separate sensors' values
  }
  Serial.println();//Printing a linebreak - this will put the next 8 conversions in a new line
        

  if (Serial.available() > 0)
  {
    char command = Serial.read();
    if(command = 'c')
    {
      adc.sendDirectCommand(SELFCAL);
    }
    if(command = 'v')
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
    switch (command) {
      case '1': // relay 1 on
        state1 = !state1;
        digitalWrite(RELAY_1, state1);
        Serial.println("Enter commands: ");
        break;
      case '2': // relay 2 on
        state2 = !state2;
        digitalWrite(RELAY_2, state2);
        Serial.println("Enter commands: ");
        break;
      case '3': // relay 3 on
        state3 = !state3;
        digitalWrite(RELAY_3, state3);
        Serial.println("Enter commands: ");
        break;
      case '4': // relay 4 on
        state4 = !state4;
        digitalWrite(RELAY_4, state4);
        Serial.println("Enter commands: ");
        break;
      case '5': // relay 5 on
        state5 = !state5;
        digitalWrite(RELAY_5, state5);
        Serial.println("Enter commands: ");
        break;
      case '6': // relay 6 on
        state6 = !state6;
        digitalWrite(RELAY_6, state6);
        Serial.println("Enter commands: ");
        break;
      case '7':
        Serial.println("Calibrating...");
        relaysCal();
        Serial.println("Enter commands: ");
        break;
      case '8':
        Serial.println("Fire");
        startSeq(); 
        previousTime = millis();
        break;  
      case '9':
        Serial.println("Purge");
        purge();
        previousTime = millis();
        break;
      case '10':
        Serial.println("Pressurizing");
        pressurize();
      default:
        break;
    }
  }

  elapsedTime = currentMillis-previousTime;
  
  if(fireState == 1 && elapsedTime >= fireTime)
  {
    startSeq();
    previousTime = millis();
    purge();
  }
  if(purgeState == 1 && elapsedTime >= purgeTime)
  {
    purge();
    previousTime = millis();
  }

}

void relaysCal()
{
  for (int i=0; i<6; i++)
  {
    digitalWrite(relayPins[i],HIGH);
    delay(1000);
    digitalWrite(relayPins[i],LOW);
  }
}

void pressurize()
{
  isolationState =! isolationState;
  digitalWrite(RELAY_1, isolationState);
  digitalWrite(RELAY_2, isolationState);  
}

// toggle firing sequence
void startSeq()
{
  fireState =! fireState;
  digitalWrite(RELAY_5, fireState);
  digitalWrite(RELAY_6, fireState);
}

// toggle purge sequence
void purge()
{

  purgeState =! purgeState;
  digitalWrite(RELAY_3,purgeState);
  digitalWrite(RELAY_4,purgeState);
  digitalWrite(RELAY_5, purgeState);
  digitalWrite(RELAY_6, purgeState);
}
