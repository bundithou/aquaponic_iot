/*
  run on arduino mega
  serial1
  Tx1 Rx1

*/

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
 Serial1.println(value);
 delay(5000);
 id +=1;
}
