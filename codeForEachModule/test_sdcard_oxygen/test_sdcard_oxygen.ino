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
int hou_p = 0;
int min_p = 0;
int sec_p = 0;

// oxygen sensor
#define o2pin   A2
float o2upperBound = 4.0;
float o2lowerBound = 1.5;

// temperature
//#define temppin 10

//pump
#define Power_Air_Pump  8

bool flag_on = false;

int t = 0;
int acc = 0;
float accO2 = 0;

o2sensor o2(o2pin);
//temperaturesensor temperature(temppin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  pinMode(Power_Air_Pump,  OUTPUT);
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
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
  myFile = SD.open("TEST_O2.TXT");
  if (myFile) {
    Serial.println("test_o2.txt:");

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
  o2.calculateO2(25.0);
  
  float o2_v = o2.getO2();
  accO2 += o2_v;
  
  //float temp_v = temperature.getTemperature();

  //o2.calculateO2(temp_v);
  //float o2_v2 = o2.getO2();
  
  
  if (millis() - t >= 1000){
    t = millis()+(millis()-t-1000);
    float avgO2 = accO2 / float(acc);
    acc = 0;
    accO2 = 0;
    sec_p++;
    if(sec_p >= 60){
      sec_p=0;
      min_p++;
      if (flag_on){
        digitalWrite(Power_Air_Pump, LOW); //close
        flag_on = false;
      } else {
        digitalWrite(Power_Air_Pump, HIGH);
        flag_on=true;
      }
    }
    if(min_p >= 60){
      min_p = 0;
      hou_p++;
    }
    myFile = SD.open("TEST_O2.TXT", FILE_WRITE);
    // put your main code here, to run repeatedly:
    // if the file opened okay, write to it:
    if (myFile) {
      myFile.print(hou_p);
      myFile.print(":");
      myFile.print(min_p);
      myFile.print(":");
      myFile.print(sec_p);
      myFile.print(",");
      myFile.print(o2_v);
      myFile.println("");
      myFile.close();
      Serial.print(hou_p);
      Serial.print(":");
      Serial.print(min_p);
      Serial.print(":");
      Serial.print(sec_p);
      Serial.print(",");
      Serial.print(o2_v);
      Serial.println("");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  }
}
