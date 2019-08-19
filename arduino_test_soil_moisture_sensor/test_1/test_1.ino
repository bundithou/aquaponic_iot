#include <Wire.h>
#include <OneWire.h>
#define MOISTURE_THRESHOLD 55
int moisture_Pin = 4;
int moisture_value= 0, moisture_state = 0xFF;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
//  Serial.println ();
//  Serial.println ("I2C scanner. Scanning ...");
//  byte count = 0;
//  
//  Wire.begin();
//  for (byte i = 8; i < 120; i++)
//  {
//    Wire.beginTransmission (i);
//    if (Wire.endTransmission () == 0)
//      {
//      Serial.print ("Found address: ");
//      Serial.print (i, DEC);
//      Serial.print (" (0x");
//      Serial.print (i, HEX);
//      Serial.println (")");
//      count++;
//      delay (1); // maybe unneeded?
//      } // end of good response
//  } // end of for loop
//  Serial.println ("Done.");
//  Serial.print ("Found ");
//  Serial.print (count, DEC);
//  Serial.println (" device(s).");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  moisture_value= analogRead(moisture_Pin);
//  Serial.print("MOISTURE RAW VALUE : ");
//  Serial.println(moisture_value);
//  moisture_value= moisture_value/10;
//  Serial.print("MOISTURE LEVEL : ");
  Serial.println(100-(moisture_value/10));
//  if(moisture_value > MOISTURE_THRESHOLD) moisture_state = 0;
//  else moisture_state = 1;
//  Serial.print("MOISTURE state : ");
//  Serial.println(moisture_state);
  delay(1000);
}
