#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 18;
const int LOADCELL_SCK_PIN = 19;

HX711 lox;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lox.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  lox.set_offset(136001);
  lox.set_scale(25.744497);
  delay(3000);   
}

void calibrate()
{
  Serial.println("\n\nCALIBRATION\n===========");
  Serial.println("remove all weight from the loadcell");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("and press enter\n");
  while (Serial.available() == 0);

  Serial.println("Determine zero weight offset");
  lox.tare(20);  // average 20 measurements.
  uint32_t offset = lox.get_offset();

  Serial.print("OFFSET: ");
  Serial.println(offset);
  Serial.println();


  Serial.println("place a weight on the loadcell");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("enter the weight in (whole) grams and press enter");
  uint32_t weight = 0;
  while (Serial.peek() != '\n')
  {
    if (Serial.available())
    {
      char ch = Serial.read();
      if (isdigit(ch))
      {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  Serial.print("WEIGHT: ");
  Serial.println(weight);
  lox.calibrate_scale(weight, 20);
  float scale = lox.get_scale();

  Serial.print("SCALE:  ");
  Serial.println(scale, 6);

  Serial.print("\nuse scale.set_offset(");
  Serial.print(offset);
  Serial.print("); and scale.set_scale(");
  Serial.print(scale, 6);
  Serial.print(");\n");
  Serial.println("in the setup of your project");

  Serial.println("\n\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  char message;
  if(Serial.available() > 0)
  {
    message = Serial.read();
  }  

  if(message == 'c')
  {
    calibrate();
  }

  Serial.print("Weight: ");
  Serial.println(lox.get_units(10)*.0022); //0.0022 is conversion factor from grams to lbs 
  delay(100);

}
