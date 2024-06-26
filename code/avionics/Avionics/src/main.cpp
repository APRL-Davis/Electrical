#include <Arduino.h>
#include <SPI.h>
#include "BMI088.h"
#include "SD.h"
#include "SparkFun_BMP581_Arduino_Library.h"

/* accel object */
const int csAccel = 36;
const int csGyro = 35;
Bmi088Accel accel(SPI, csAccel);
Bmi088Gyro gyro(SPI, csGyro);

/* Barometer object*/
BMP581 pressureSensor;
const int CSB = 10;

/* Low pass filter parameters */
float alpha = 0.2; // Smoothing factor for the low pass filter
float accel_X = 0;
float accel_Y = 0;
float accel_Z = 0;
float gyro_X = 0;
float gyro_Y = 0;
float gyro_Z = 0;

long startTime = 0;
long elapsedTime = 0;
bool recordFlag = 0;

void setup() 
{
  int status;
  /* USB Serial to print data */
  Serial.begin(115200);
  delay(2000);
  while(!Serial) {}

  pinMode(CSB, OUTPUT);
  pinMode(csAccel, OUTPUT);
  pinMode(csGyro, OUTPUT);

  digitalWrite(CSB, HIGH);
  digitalWrite(csAccel, HIGH);
  digitalWrite(csGyro, HIGH);

  SPI.begin();

  pressureSensor.beginSPI(CSB);
  if(pressureSensor.beginSPI(CSB) != BMP5_OK)
  {
    Serial.println("Barometer fail to connect");
  }
  else
  {
    Serial.println("Barometer initialized");
  }

  /* start the sensors */
  status = accel.begin();
  if (status < 0) {
    Serial.println("Accel Initialization Error");
    Serial.println(status);
  }
  status = gyro.begin();
  if (status < 0) {
    Serial.println("Gyro Initialization Error");
    Serial.println(status);
    
  }

  // SD.begin(BUILTIN_SDCARD);
  /*Set up SD*/
  if (!SD.begin(BUILTIN_SDCARD))
  {
    Serial.println("Card failed");
  }
  Serial.println("Card initialized");
}

float lowPass(float alpha, float raw, float filtered)
{
  float result;
  result = alpha*raw + (1-alpha)*filtered;
  return result;
}

void loop() 
{
  File dataFile = SD.open("flight_data.txt", FILE_WRITE);
  double altitude = 0;
  // Get measurements from the sensor
    bmp5_sensor_data BMPdata = {0,0};
    int8_t err = pressureSensor.getSensorData(&BMPdata);
    if(err == BMP5_OK)
    {
      altitude = 44330*(1 - pow((BMPdata.pressure/100)/1009.14, 1/5.255));
    }
    else{
      Serial.println("yo mama");
    }
    

  /* read the accel */
  accel.readSensor();

  /* get the raw data */
  float rawX = accel.getAccelX_mss();
  float rawY = accel.getAccelY_mss();
  float rawZ = accel.getAccelZ_mss();

  /* apply low pass filter */

  accel_X = lowPass(alpha, rawX, accel_X);
  accel_Y = lowPass(alpha, rawY, accel_Y);
  accel_Z = lowPass(alpha, rawZ, accel_Z);

  float rocket_Z = accel_Y;
  float rocket_Y = accel_Z;
  float rocket_X = accel_X;

  /* read the gyro*/
  // gyro.readSensor();
  // float gyroX = gyro.getGyroX_rads();
  // float gyroY = gyro.getGyroY_rads();
  // float gyroZ = gyro.getGyroZ_rads();
  // gyro_X = lowPass(alpha, gyroX, gyro_X);
  // gyro_Y = lowPass(alpha, gyroY, gyro_Y);
  // gyro_Z = lowPass(alpha, gyroZ, gyro_Z);

  Serial.print(">Accel X:");
  Serial.println(rocket_X);
  Serial.print(">Accel Y:");
  Serial.println(rocket_Y);
  Serial.print(">Accel Z:");
  Serial.println(rocket_Z);

//&& rocket_Z > 5
  // if(recordFlag == 0 && rocket_Z < -15)
  // {
  //   startTime = millis();
  //   recordFlag = 1;
  // }

  // if(recordFlag)
  // {
  //   /* read the gyro*/
  //   gyro.readSensor();
  //   float gyroX = gyro.getGyroX_rads();
  //   float gyroY = gyro.getGyroY_rads();
  //   float gyroZ = gyro.getGyroZ_rads();
  //   gyro_X = lowPass(alpha, gyroX, gyro_X);
  //   gyro_Y = lowPass(alpha, gyroY, gyro_Y);
  //   gyro_Z = lowPass(alpha, gyroZ, gyro_Z);
  // }

  // if(dataFile && rocket_Z < -15)
  // {
  //   dataFile.print(millis() - startTime);
  //   dataFile.print(" ");
  //   dataFile.print(rocket_X);
  //   dataFile.print(" ");
  //   dataFile.print(rocket_Y);
  //   dataFile.print(" ");
  //   dataFile.print(rocket_Z);
  //   dataFile.print(" ");
  //   dataFile.print(gyro_X);
  //   dataFile.print(" ");
  //   dataFile.print(gyro_Z);
  //   dataFile.print(" ");
  //   dataFile.print(gyro_Y);
  //   dataFile.print(" ");
  //   dataFile.print(altitude);
  //   dataFile.println();

  //   dataFile.close();
  // }
  // else{}

  delay(100);
}