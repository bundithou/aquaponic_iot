///////////////////////////////////////////////////////////////////////////////
// Author: RSP @ Embedded System Lab (ESL), KMUTNB, Thailand
// Date: 2014-May-12
// Board: Arduino with ATmega168/328P (5V/16MHz)
// Arduino IDE: version 1.0.5
// Description:
//   This Arduino Sketch shows how to read temperature values from
//   two DS18B20 digital thermometers connected to the same bus (1-wire).
///////////////////////////////////////////////////////////////////////////////

#include <OneWire.h> // Use the Arduino "OneWire" library.

#define DS18S20_ID   (0x10)   // The byte code for the DS18S20 device family.
#define DS18B20_ID   (0x28)   // The byte code for the DS18B20 device family.

#define DQ_PIN       (2)      // Use Arduino D2 pin for I/O (connected to the DQ pin).
OneWire ds( DQ_PIN );         // Create a OneWire object.

#define NUM_DEVICES  (2)      // Specify the number of DS18B20 devices to be used.
byte ds_addr[NUM_DEVICES][8]; // used to stored 8-byte device addresses (for two devices)
char sbuf[20];                // used for sprintf()

// 'buf' must be an array of at least 8 bytes. 
boolean ds_addr_search( byte *buf ) { // Perform a DS18xx device search 
  if ( ds.search(buf) == 1 ) { // ok
    if ( OneWire::crc8( buf, 7 ) == buf[7] ) { // check the CRC byte
       return true; // CRC is valid
    }
  } else {
    ds.reset_search();
  }
  return false;
}

void ds_devices_scan() { // Scan devices on the 1-Wire bus
  for ( uint8_t dev=0; dev < NUM_DEVICES; dev++ ) { 
    uint8_t *addr = ds_addr[dev];
    memset( addr, 0x00, 8 );   
    if ( ds_addr_search( addr ) ) {
      for (uint8_t i=0; i < 8; i++) {
        sprintf( sbuf+3*i, "%02X ", addr[i] );
      }
      Serial.print( "Address found: " );
      Serial.print( sbuf );
      sprintf( sbuf, "(Device %d)", dev ); 
      Serial.println( sbuf );
      if ( addr[0] == DS18B20_ID ) {
        Serial.println( "This device is DS18B20." );
      }
      else if ( addr[0] == DS18S20_ID ) {
        Serial.println( "This device is DS18S20." );
      }
      else {
        Serial.println( "Unrecognized device family!" );
      }
    }
  }
}

void setup(void) {
  Serial.begin(115200);
  ds_devices_scan();
}
 
boolean ds_read_temp( byte *addr, int16_t *temp ) {
  static byte data[9];
  ds.reset();
  ds.select( addr );
  ds.write( 0x44, 1 );  // initiates a temperature conversion, enable pull-up on the 1-Wire bus
  delay( 800 );         // wait for at least 750ms for 12-bit ADC conversion time
  ds.reset();
  ds.select( addr );  
  ds.write( 0xBE );     // read Scratchpad
  for ( byte i=0; i < 9; i++ ) { // read 9 bytes in total
    data[i] = ds.read();
  }
  if ( OneWire::crc8( data, 8) == data[8] ) { // CRC is ok.
    int16_t t = data[1];
    t = (t << 8) + data[0];
    *temp = (t*10)/16;  // for DS18B20, use 0.0625 = 1/16 deg per bit
    return true;
  }
  return false;
}

void loop(void) {
  static int16_t temp;
  for ( uint8_t dev=0; dev < NUM_DEVICES; dev++ ) {
    if ( ds_addr[dev][0] != DS18B20_ID ) continue;
    if ( ds_read_temp( ds_addr[dev], &temp ) ) {
      char pm = (temp < 0) ? '-' : '+'; 
      temp = abs(temp);
      sprintf( sbuf, "Device %d: %c%d.%d C", dev, pm, (temp/10), (temp%10) );
      Serial.println( sbuf );
    } else {
      Serial.println( "Read operation failed!" );
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////
