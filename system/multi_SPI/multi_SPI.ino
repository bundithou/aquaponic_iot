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
unsigned int min_p = 0;
unsigned int sec_p = 0;
unsigned int hr_p = 0;
unsigned int day_p = 0;

//SPIs
//SDcard
#define SD_CS 10
//MCP3208
#define MCP_CS 9

// oxygen sensor
#define o2pin   A2
float o2upperBound = 4.0;
float o2lowerBound = 2.0;

// temperature
#define temppin 6

// ultrasonics
#define fish_trig 5
#define fish_echo 4
#define water_trig 3
#define water_echo 2

//pump
#define Power_Air_Pump  8

bool flag_on = false;

unsigned long t = 0;
unsigned long acc = 0;

unsigned long mil = 0;
double accO2 = 0;

o2sensor o2(o2pin);
temperaturesensor temperature(temppin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  pinMode(Power_Air_Pump,  OUTPUT);
  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    //while (1);
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("TEST_O2.TXT", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("tsm, o2 value, temp value, o2_temp");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("LOG_"+day_p+".TXT");
  if (myFile) {
    Serial.println("LOG_"+day_p+".TXT:");

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

  //force SD card to stop working
  //aka. select only the mcp
  digitalWrite(SD_CS, HIGH); //disable SD card
  digitalWrite(MCP_CS, LOW);

  //////////////////////
  //reading from sensors
  //////////////////////
  //temperature
  float temperature = temperaturesensor.getTemperature();

  //oxygen
  o2.calculateO2(temperature);
  double o2_v = o2.getO2();

  //ultrasonics

  //pH

  //soil moisture
  
  
  if ((mil = (millis())) - t >= 1000){
    t = mil;

    //"time" management since we are yet to use RTC
    sec_p++;
    if(sec_p >= 60){
      sec_p=0;
      min_p++;
    }
    if(min_p >= 60){
      min_p = 0;
      hr_p++; 
    }
    if(hr_p >= 60){
      hr_p = 0;
      day_p++; 
    }

    //Control stuffs
    if (flag_on && (avgO2 > o2upperBound)){
      digitalWrite(Power_Air_Pump, LOW); //close
      flag_on = false;
    }
    if (!flag_on && (avgO2 < o2lowerBound)) {
      digitalWrite(Power_Air_Pump, HIGH);
      flag_on=true;
    }

    //Write the file
    digitalWrite(SD_CS, LOW); //enable SD card
    digitalWrite(MCP_CS, HIGH); //disable MCP
    myFile = SD.open("LOG_"+day_p+".TXT", FILE_WRITE);
    // put your main code here, to run repeatedly:
    // if the file opened okay, write to it:
    if (myFile) {
      myFile.print(hr_p);
      myFile.print(":");
      myFile.print(min_p);
      myFile.print(":");
      myFile.print(sec_p);
      myFile.print(",");
      myFile.print(avgO2);
      myFile.print(",");
      myFile.print(flag_on);
      myFile.println("");
      myFile.close();
      Serial.print(hr_p);
      Serial.print(":");
      Serial.print(min_p);
      Serial.print(":");
      Serial.print(sec_p);
      Serial.print(",avg=");
      Serial.print(avgO2);
      Serial.print(",cur O2=");
      Serial.print(o2_v);
      Serial.print(",");
      Serial.print(flag_on);
      Serial.println("");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
      Serial.print(hr_p);
      Serial.print(":");
      Serial.print(min_p);
      Serial.print(":");
      Serial.print(sec_p);
      Serial.print(",avg=");
      Serial.print(avgO2);
      Serial.print(",cur O2=");
      Serial.print(o2_v);
      Serial.print(",");
      Serial.print(flag_on);
      Serial.println("");
    }
  }
}
