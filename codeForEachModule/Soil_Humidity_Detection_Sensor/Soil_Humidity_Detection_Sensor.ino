/* 

int sensorPin = A0;


void setup() {
  Serial.begin(9600); // set up serial port for 9600 baud (speed)
  delay(500); // wait for display to boot up
}

void loop() {
  
  int sensorValue;
  sensorValue = analogRead(sensorPin);
  sensorValue = map(sensorValue, 0, 1023, 0, 100);
  Serial.print("Soil moisture: ");
  Serial.print(sensorValue);
  Serial.println(" %");
  
  delay(500); //wait for half a second, so it is easier to read
}*/

const int sensor_pin = A0;  /* Connect Soil moisture analog sensor pin to A0 of NodeMCU */

void setup() {
  Serial.begin(9600); /* Define baud rate for serial communication */
}   

void loop() {
  float moisture_percentage;

  moisture_percentage = ( 100.00 - ( (analogRead(sensor_pin)/1023.00) * 100.00 ) );

  Serial.print("Soil Moisture(in Percentage) = ");
  Serial.print(moisture_percentage);
  Serial.println("%");

  delay(1000);
}
