#include "ADS1256.h"

/*======== ADC ========*/

ADS1256 adc(2, 2500000, 9, 10, 2.5); //DRDY, SPI speed, SYNC(PDWN), CS, VREF(float).

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
const int RELAY_7 = 9; // purge

int relayPins[] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4, RELAY_5, RELAY_6, RELAY_7};
  
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

void setup() 
{
  Serial.begin(115200); //The value does not matter if you use an MCU with native USB

  while (!Serial)
  {
    ;//Wait until the serial becomes available
  }

  Serial.println("Serial available");

  // set relay pinmodes
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);
  pinMode(RELAY_7, OUTPUT);

  Serial.println("Relays initialized...");

  // initialize adc and set gain + data rate
  adc.InitializeADC();
  adc.setPGA(PGA_1);
  adc.setDRATE(DRATE_100SPS);

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
    digitalWrite(relayPins[i],HIGH);
  }
}

void endCheck()
{
  calFlag =! calFlag;
  for (int i=0; i<7; i++)
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
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
  digitalWrite(RELAY_5, LOW);
  digitalWrite(RELAY_6, LOW);
  digitalWrite(RELAY_7, HIGH);
}

void endPurge()
{
  purgeState = 0;
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
  digitalWrite(RELAY_5, LOW);
  digitalWrite(RELAY_6, LOW);
}

void loop() 
{
  uint8_t command;

  for (int i = 0; i < 8; i++)
  {
    Serial.print(adc.convertToVoltage(adc.cycleSingle()),4); //print the converted single-ended results with 4 digits
    // Serial.print(i);
    Serial.print(" "); //space to separate sensors' values
  }
  Serial.println();//Printing a linebreak - this will put the next 8 conversions in a new line
  
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
    if(command == 7) // relay 7 on
    {
      state7 = !state7;
      digitalWrite(RELAY_7, state7);
    }
    if(command == 11)
    {
      startCheck();
      previousTime = millis();
    }
    
    
    if(command == 12)
    {
      pressurize();
    }
    if(command == 13)
    {
      Serial.println("Purge");
      purge();
      previousTime = millis();
    }  
    if(command == 14)
    {
      Serial.println("Fire");
      startSeq(); 
      previousTime = millis();
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
