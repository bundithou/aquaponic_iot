
HardwareSerial sw(2);

void setup() {
  Serial.begin(115200); 
  Serial.println("setup");
  sw.begin(115200); 
}

void loop() {
  String d = "";
 if (sw.available() > 0) {
   //Serial.println("receive: ");
   char bfr[501];
   memset(bfr,0, 501);
   sw.readBytesUntil( '\n',bfr,500);
   Serial.println(bfr);
 }
   delay(5000);
 }
