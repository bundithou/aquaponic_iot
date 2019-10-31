/*
  run on arduino mega
  serial1
  Tx1 Rx1

*/
//#include <SoftwareSerial.h>

//SoftwareSerial sw(2, 3); // RX, TX
int id = 0;
void setup() {
 Serial.begin(115200);
 Serial.println("Interfacfing arduino with nodemcu");
 Serial1.begin(115200);
}

void loop() {
  // read from esp
 if (Serial1.available() > 0) {
   Serial.println("receive: ");
   char bfr[501];
   memset(bfr,0, 501);
   Serial1.readBytesUntil( '\n',bfr,500);
   Serial.println(bfr);
 }
 
 int value = id*2;
 
// Serial.println("Sending data to nodemcu");
// Serial.print("{\"sensorid\":");
// Serial.print(id);//sensor id
// Serial.print(",");
// Serial.print("\"value\":");
// Serial.print(value);//offset
// Serial.print("}");
// Serial.println();
// 
// sw.print("{\"sensorid\":");
// sw.print(id);//sensor id
// sw.print(",");
// sw.print("\"value\":");
// sw.print(value);//offset
// sw.print("}");
  // send to esp
 Serial1.println(value);
 delay(5000);
 id +=1;
}
