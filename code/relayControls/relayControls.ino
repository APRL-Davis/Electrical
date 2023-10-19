#include <Arduino.h>
#include <wiring.h>

#define RELAY_1 8
#define RELAY_2 3
#define RELAY_3 4
#define RELAY_4 5
#define RELAY_5 6
#define RELAY_6 7
#define fireTrigger 2
#define purgeTrigger 1

relayPins[] = {RELAY_1,RELAY_2,RELAY_3,RELAY_4,RELAY_5,RELAY_6};

unsigned long previousTime;  //some global variables available anywhere in the program
unsigned long currentMillis;

// interrupt pins
volatile bool fireState;
volatile bool purgeState;

// relay states
static bool state1 = 0;
static bool state2 = 0;
static bool state3 = 0;
static bool state4 = 0;
static bool state5 = 0;
static bool state6 = 0;

const unsigned int fireTime = 10000;
const unsigned int purgeTime = 6000;
unsigned int fireStart = 0;

int command;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);

  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);

  pinMode(fireTrigger,INPUT_PULLUP);
  pinMode(purgeTrigger,INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(fireTrigger),startSeq,FALLING);
  attachInterrupt(digitalPinToInterrupt(purgeTrigger),purge,FALLING);
}



void relaysCal()
{
  for (int i=1,i<=6>,i++)
  {
    digitalWrite(relayPins[i],HIGH);
    delay(3000);
    digitalWrite(relayPins[i],LOW);
  }
}

void startSeq()
{
  if(digitalRead(fireTrigger)==LOW)
  {
    fireState =! fireState;
    digitalWrite(RELAY_5, fireState);
    digitalWrite(RELAY_6, fireState);
  }
}

void purge()
{
  if(digitalRead(purgeTrigger)==LOW)
  {
    purgeState =! purgeState;
    digitalWrite(RELAY_3,purgeState);
    digitalWrite(RELAY_4,purgeState);
    digitalWrite(RELAY_5, purgeState);
    digitalWrite(RELAY_6, purgeState);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = currentMillis();
  Serial.println("Enter commands: ");
  if (Serial.available()>0){
    command = Serial.parseInt();                 
  }
  switch (command) {
    case 1: // relay 1 on
      state1 = !state1;
      digitalWrite(RELAY_1, state1);
    case 2: // relay 2 on
      state2 = !state2;
      digitalWrite(RELAY_2, state2);
    case 3: // relay 3 on
      state3 = !state3;
      digitalWrite(RELAY_3, state3);
    case 4: // relay 4 on
      state4 = !state4;
      digitalWrite(RELAY_4, state4);
    case 5: // relay 5 on
      state5 = !state5;
      digitalWrite(RELAY_5, state5);
    case 6: // relay 6 on
      state6 = !state6;
      digitalWrite(RELAY_6, state6);
    case 7:
      relaysCal();
    case 8:
      digitaWrite(fireTrigger, digitalRead(fireTrigger) ^ 1);
    case 9:
      digitalWrite(purgeTrigger, digitalRead(purgeTrigger) ^ 1);
    default:
      break;
  }
  if(digitalRead(fireTrigger) == 0)
  {
    if(currentMillis - previousTime >= fireTime)
    {
      digitaWrite(fireTrigger, digitalRead(purgeTrigger) ^ 1);
      previousTime = currentMillis;
    }
  }
  if(digitalRead(purgeTrigger) == 0)
  {
    if(currentMillis - previousTime >= purgeTime)
    {
      digitaWrite(purgeTrigger, digitalRead(purgeTrigger) ^ 1);
      previousTime = currentMillis;
    }
  }
}

// send and receiver command through serial 
// command is set of 2 numbers. First digit indicates relay number; second digit indicates the desired state of the relay (on/off = 1/2)
// make the control code into a function
// 6 relay
// coding topics: serial, input/output mode + arduino
