#include <Arduino.h>

const int RELAY_1 = 2; // kero iso
const int RELAY_2 = 3; // lox iso
const int RELAY_3 = 4; // kero main
const int RELAY_4 = 5; // lox main
const int RELAY_5 = 6; // ker vent
const int RELAY_6 = 7; // lox vent

int relayPins[] = {RELAY_1,RELAY_2,RELAY_3,RELAY_4,RELAY_5,RELAY_6};
long randNumber;

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

const unsigned int fireTime = 10000;
const unsigned int purgeTime = 3000;

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

  Serial.println("Enter commands: ");
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
  digitalWrite(RELAY_3, fireState);
  digitalWrite(RELAY_4, fireState);
}

// toggle purge sequence
void purge()
{

  purgeState =! purgeState;
  // digitalWrite(RELAY_3,purgeState);
  // digitalWrite(RELAY_4,purgeState);
  digitalWrite(RELAY_5, purgeState);
  digitalWrite(RELAY_6, purgeState);
}

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();
  if (Serial.available()>0){
    command = Serial.parseInt();                 
  }
  switch (command) {
    case 1: // relay 1 on
      state1 = !state1;
      digitalWrite(RELAY_1, state1);
      Serial.println("Enter commands: ");
      break;
    case 2: // relay 2 on
      state2 = !state2;
      digitalWrite(RELAY_2, state2);
      Serial.println("Enter commands: ");
      break;
    case 3: // relay 3 on
      state3 = !state3;
      digitalWrite(RELAY_3, state3);
      Serial.println("Enter commands: ");
      break;
    case 4: // relay 4 on
      state4 = !state4;
      digitalWrite(RELAY_4, state4);
      Serial.println("Enter commands: ");
      break;
    case 5: // relay 5 on
      state5 = !state5;
      digitalWrite(RELAY_5, state5);
      Serial.println("Enter commands: ");
      break;
    case 6: // relay 6 on
      state6 = !state6;
      digitalWrite(RELAY_6, state6);
      Serial.println("Enter commands: ");
      break;
    case 7:
      Serial.println("Calibrating...");
      relaysCal();
      Serial.println("Enter commands: ");
      break;
    case 8:
      Serial.println("Fire");
      startSeq(); 
      previousTime = millis();
      break;  
    case 9:
      Serial.println("Purge");
      purge();
      previousTime = millis();
      break;
    case 10:
      Serial.println("Pressurizing");
      pressurize();
    default:
      break;
  }

  elapsedTime = currentMillis-previousTime;
  
  if(fireState == 1 && elapsedTime >= fireTime)
  {
    startSeq();
    Serial.println("Main off");
    previousTime = millis();
    // purge();
  }
  if(purgeState == 1 && elapsedTime >= purgeTime)
  {
    purge();
    previousTime = millis();
  }
}
