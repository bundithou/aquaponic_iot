/*
  SD card read/write

 This example shows how to read and write data to and from an SD card file
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */
#include <SPI.h>
#include <SD.h>
#include <Aqualib.h>

File myFile;
int min_p = 0;
int sec_p = 0;

// oxygen sensor
//#define o2pin   A2

// temperature
//#define temppin 10

// soil moisture
#define soilpin A1
float accSoilMoisture = 0; //accumulate values for later average in each second
float upper_moisture = 90;
float lower_moisture = 50;

//ultrasonic - fishtank
#define trigPinFish 5
#define echoPinFish 4
float accFishTankDistance = 0; //accumulate values for later average in each second

//pump
//#define Power_Air_Pump  8
#define Power_Pump 9

bool flag_on = false;

float t = 0;
int acc = 0;

//o2sensor o2(o2pin);
//temperaturesensor temperature(temppin);
soilMoisturesensor soilMoisture(soilpin);
ultrasonicsensor ultrasonicFishTank(trigPinFish, echoPinFish);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  //pinMode(Power_Air_Pump,  OUTPUT);
  pinMode(Power_Pump, OUTPUT);
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test_moisture.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("tsm, soil moisture (RH), Fish ultrasonic distance (cm)");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test_moisture.txt");
  if (myFile) {
    Serial.println("test_moisture.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  t = millis();
}

void loop() {
  accFishTankDistance += ultrasonicFishTank.getDistance();
  accSoilMoisture += soilMoisture.getSoilMoisture();
  acc++;
  
  if (millis() - t >= 1000){
    t = millis()+(millis()-t-1000);
    
    float avgFishTankDistance = accFishTankDistance / float(acc);
    float avgSoilMoisture = accSoilMoisture / float(acc);
    acc = 0;
    accFishTankDistance = 0;
    accSoilMoisture = 0;
    
    sec_p++;
    if(sec_p > 60){
      sec_p=0;
      min_p++;
      if (flag_on){
        digitalWrite(Power_Pump, LOW); //close
        flag_on = false;
      } else {
        digitalWrite(Power_Pump, HIGH);
        flag_on=true;
      }
    }

    myFile = SD.open("test_moisture.txt", FILE_WRITE);
    // put your main code here, to run repeatedly:
    // if the file opened okay, write to it:
    if (myFile) {
      myFile.print(min_p);
      myFile.print(":");
      myFile.print(sec_p);
      myFile.print(",");
      myFile.print(avgSoilMoisture);
      myFile.print(",");
      myFile.print(avgFishTankDistance);
      myFile.println("");
      myFile.close();
      Serial.print(min_p);
      Serial.print(":");
      Serial.print(sec_p);
      Serial.print(",");
      Serial.print(avgSoilMoisture);
      Serial.print(",");
      Serial.print(avgFishTankDistance);
      Serial.println("");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  }
}
