/*
run on esp
serial2
tx 17
rx 16
*/

void setup() {
  Serial.begin(115200); 
  Serial.println("setup");
  Serial2.begin(115200); 
}

void loop() {
  String d = "";
  // read
 if (Serial2.available() > 0) {
   //Serial.println("receive: ");
   char bfr[501];
   memset(bfr,0, 501);
   Serial2.readBytesUntil( '\n',bfr,500);
   Serial.println(bfr);
 }
 //send
 Serial2.println("ack");
   delay(5000);
 }
