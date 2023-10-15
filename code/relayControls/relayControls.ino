#include <Arduino.h>
#include <wiring.h>

#define RELAY_1 2
#define RELAY_2 3
#define RELAY_3 4
#define RELAY_4 5
#define RELAY_5 6
#define RELAY_6 7
relayPins[] = {RELAY_1,RELAY_2,RELAY_3,RELAY_4,RELAY_5,RELAY_6};

// relay states
static boolean state1 = 0;
static boolean state2 = 0;
static boolean state3 = 0;
static boolean state4 = 0;
static boolean state5 = 0;
static boolean state6 = 0;

unsigned int fireTime = 10000;
unsigned int fireStart = 0;
int command;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
delay(1000);

pinMode(RELAY_1, OUTPUT);
pinMode(RELAY_2, OUTPUT);
pinMode(RELAY_3, OUTPUT);
pinMode(RELAY_4, OUTPUT);
pinMode(RELAY_5, OUTPUT);
pinMode(RELAY_6, OUTPUT);

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

void startSeq(String command)
{
  if(command == "fire")
  fireStart = millis();
    while(millis()-fireStart<fireTime)
    {
      digitalWrite(RELAY_5, HIGH)
      digitalWrite(RELAY_6, HIGH)
    }
    digitalWrite(RELAY_5, LOW)
    digitalWrite(RELAY_6, LOW) 
}

void purge(String command)
{
  if(command == "p")
    digitalWrite(RELAY_3,HIGH);
    digitalWrite(RELAY_4,HIGH);
    digitalWrite(RELAY_5, HIGH)
    digitalWrite(RELAY_6, HIGH)
    delay(6000);
    digitalWrite(RELAY_3,HIGH);
    digitalWrite(RELAY_4,HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Enter numerical command: ");
  while(Serial.available()>0)
  {
    command = Serial.parseInt();
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
      default:
        break;
    }

    if(command == "fire")
    {
      fireStart = millis
    }
    startSeq(command);
      
  }
}

// send and receiver command through serial 
// command is set of 2 numbers. First digit indicates relay number; second digit indicates the desired state of the relay (on/off = 1/2)
// make the control code into a function
// 6 relay
// coding topics: serial, input/output mode + arduino
