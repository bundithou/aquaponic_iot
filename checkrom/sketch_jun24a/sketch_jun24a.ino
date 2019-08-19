#include <EEPROM.h>

struct MyObject{
  float field1;
  byte field2;
  char name[10];
};

void setup(){

  float f = 0.00f;   //Variable to store data read from EEPROM.
  int eeAddress = 0; //EEPROM address to start reading from

  Serial.begin( 9600 );
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print( "Read float from EEPROM: " );

  //Get the float data from the EEPROM at position 'eeAddress'
  EEPROM.get( eeAddress, f );
  Serial.println( f, 3 );  //This may print 'ovf, nan' if the data inside the EEPROM is not a valid float.

  // get() can be used with custom structures too. 
  eeAddress = sizeof(float); //Move address to the next byte after float 'f'.
  MyObject customVar; //Variable to store custom object read from EEPROM.
  EEPROM.get( eeAddress, customVar );

  Serial.println( "Read custom object from EEPROM: " );
  Serial.println( customVar.field1 );
  Serial.println( customVar.field2 );
  Serial.println( customVar.name );
}

void loop(){ /* Empty loop */ }
