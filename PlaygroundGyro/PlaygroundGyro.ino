#include <Wire.h> //library allows communication with I2C / TWI devices
#include <math.h> //library includes mathematical functions
#include "Keyboard.h"

const int MPU=0x68; //I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ; //16-bit integers
int AcXcal,AcYcal,AcZcal,GyXcal,GyYcal,GyZcal,tcal; //calibration variables
double t,tx,tf,pitch,roll;

byte threshold = 20;
byte activePin = 0;

uint16_t potVal = 0; 

void setup()
{
    //LEDS
    pinMode(2, OUTPUT); 
    pinMode(3, OUTPUT); 
    pinMode(4, OUTPUT); 
    pinMode(5, OUTPUT); 
    //pinMode(A3, INPUT); //potentiometer

    //GYRO OUT
    Wire.begin(); //initiate wire library and I2C
    Wire.beginTransmission(MPU); //begin transmission to I2C slave device
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)  
    Wire.endTransmission(true); //ends transmission to I2C slave device
    Serial.begin(9600); //serial communication at 9600 bauds

}

void loop()
{
    /*if (potVal != analogRead(A3)){
      
      potVal = analogRead(A3);
      //Serial.print("POTENTIOMETER= ");
      //Serial.print(potVal);

      activePin = 5- static_cast<int>(potVal / 1023.0 * 4); //returns cast range 0-3, then map to pin range
      digitalWrite(activePin, HIGH);

    }
    else{*/
      
      Wire.beginTransmission(MPU); //begin transmission to I2C slave device
      Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false); //restarts transmission to I2C slave device
      Wire.requestFrom(MPU,14,true); //request 14 registers in total  
  
      //Acceleration data correction
      AcXcal = -950;
      AcYcal = -300;
      AcZcal = 0;
  
      //Temperature correction
      tcal = -1600;
  
      //Gyro correction
      GyXcal = 480;
      GyYcal = 170;
      GyZcal = 210;
  
  
      //read accelerometer data
      AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) 0x3C (ACCEL_XOUT_L)  
      AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) 0x3E (ACCEL_YOUT_L) 
      AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) 0x40 (ACCEL_ZOUT_L)
    
      //read temperature data 
      Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) 0x42 (TEMP_OUT_L) 
    
      //read gyroscope data
      GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) 0x44 (GYRO_XOUT_L)
      GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) 0x46 (GYRO_YOUT_L)
      GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) 0x48 (GYRO_ZOUT_L) 
  
      //temperature calculation
      tx = Tmp + tcal;
      t = tx/340 + 36.53; //equation for temperature in degrees C from datasheet
      tf = (t * 9/5) + 32; //fahrenheit
  
      //get pitch/roll
      getAngle(AcX,AcY,AcZ);
    
      //printing values to serial port
      /*Serial.print("Angle: ");
      Serial.print("Pitch = "); Serial.print(pitch);
      Serial.print(" Roll = "); Serial.println(roll);
    
      Serial.print("Accelerometer: ");
      Serial.print("X = "); Serial.print(AcX + AcXcal);
      Serial.print(" Y = "); Serial.print(AcY + AcYcal);
      Serial.print(" Z = "); Serial.println(AcZ + AcZcal); 
  
      Serial.print("Temperature in celsius = "); Serial.print(t);  
      Serial.print(" fahrenheit = "); Serial.println(tf);  
    
      Serial.print("Gyroscope: ");
      Serial.print("X = "); Serial.print(GyX + GyXcal);
      Serial.print(" Y = "); Serial.print(GyY + GyYcal);
      Serial.print(" Z = "); Serial.println(GyZ + GyZcal);*/

      activePin = -1;
    
      // axis 1
      if (pitch < -threshold){
        digitalWrite(5, HIGH);
        activePin = 5;
      }
      else if (pitch > threshold){
        digitalWrite(3, HIGH);
        activePin = 3;
      }
  
      // axis 2
      if (roll < -threshold){
        digitalWrite(2, HIGH);
        activePin = 2;
      }
      else if (roll > threshold){
        digitalWrite(4, HIGH);
        activePin = 4;
      }
    
    Serial.println(activePin);
    shutOffLED(2, 5, activePin);
    delay(100);
}

void shutOffLED(byte x, byte y, byte ex){
  for (byte i = x; i <= y; i++){
    if (i != ex){
      digitalWrite(i, LOW);
    }
  }
}


//function to convert accelerometer values into pitch and roll
void getAngle(int Ax,int Ay,int Az) 
{
    double x = Ax;
    double y = Ay;
    double z = Az;

    pitch = atan(x/sqrt((y*y) + (z*z))); //pitch calculation
    roll = atan(y/sqrt((x*x) + (z*z))); //roll calculation

    //converting radians into degrees
    pitch = pitch * (180.0/3.14);
    roll = roll * (180.0/3.14) ;
}
