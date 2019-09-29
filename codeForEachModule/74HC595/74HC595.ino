#include <SPI.h>
#include <SD.h>
#include <ShiftRegister74HC595.h>

//SPIs
//SDcard
#define SD_CS 10
#define MOSI 11
#define MISO 12
#define CLK 13

//Pin connected to latch pin (ST_CP) of 74HC595
const int latchPin = 9;
//Pin connected to clock pin (SH_CP) of 74HC595
const int clockPin = 13;
////Pin connected to Data in (DS) of 74HC595
const int dataPin = 11;

//Data to write into ShiftRegister, from Q0 to Q7
//unsigned int ShiftRegisterOutData[8] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if(Serial.available() > 0){
    int bitToSet = Serial.read() - 48;
    if(bitToSet >= 0 && bitToSet <= 7){
      //ShiftRegisterOutData[bitToSet] = !ShiftRegisterOutData[bitToSet];
      write_SR(bitToSet, HIGH);
    }
  }
  //write_SR(ShiftRegisterOutData);
  delay(1000);
}

//void write_SR(unsigned int data[]){
//  digitalWrite(latchPin, LOW);
//  for(int i=7;i>=0;i--){
//    digitalWrite(dataPin, data[i]);
//    digitalWrite(clockPin,HIGH);
//    digitalWrite(clockPin,LOW);
//  }
//  digitalWrite(latchPin, HIGH);
//}

void write_SR(int ch, int output){
  static unsigned int ShiftRegisterOutData[8] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
  ShiftRegisterOutData[ch] = output;
  digitalWrite(latchPin, LOW);
  for(int i=7;i>=0;i--){
    digitalWrite(dataPin, ShiftRegisterOutData[i]);
    digitalWrite(clockPin,HIGH);
    digitalWrite(clockPin,LOW);
  }
  digitalWrite(latchPin, HIGH);
}
