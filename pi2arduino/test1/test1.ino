int n = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("connect successful");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(Serial.available()){
    response(Serial.parseInt());
    Serial.flush();
  }
  //Serial.println(n);
  delay(1000);
  //n++;
}

void response( int i ) {
  Serial.println(i+1);
}
