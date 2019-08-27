#include <SoftwareSerial.h>

SoftwareSerial sw(2, 3); // RX, TX
int id = 0;
void setup() {
 Serial.begin(115200);
 Serial.println("Interfacfing arduino with nodemcu");
 sw.begin(115200);
}

void loop() {
  int value = id*2;
 Serial.println("Sending data to nodemcu");
 Serial.print("{\"sensorid\":");
 Serial.print(id);//sensor id
 Serial.print(",");
 Serial.print("\"value\":");
 Serial.print(value);//offset
 Serial.print("}");
 Serial.println();
 
 sw.print("{\"sensorid\":");
 sw.print(id);//sensor id
 sw.print(",");
 sw.print("\"value\":");
 sw.print(value);//offset
 sw.print("}");
 sw.println();
 delay(5000);
}
