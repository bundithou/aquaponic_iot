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

const int sensor_pin = A1;  /* Connect Soil moisture analog sensor pin to A0 of NodeMCU */
int max_analog_moisture = -1;
int min_analog_moisture = 1024;


void setup() {
  Serial.begin(115200); /* Define baud rate for serial communication */
}   

void loop() {
  float moisture_percentage;
  int analog_moisture = analogRead(sensor_pin);
  if(min_analog_moisture > analog_moisture){
    min_analog_moisture = analog_moisture;
  }
  if(max_analog_moisture < analog_moisture){
    max_analog_moisture = analog_moisture;
  }

  //moisture_percentage = ( 100.00 - ( (analog_moisture/1023.00) * 100.00 ) );
  moisture_percentage =   (( 591.00-(float)analog_moisture ) / 280.00) * 100.00;
  Serial.print("Analog read: ");
  Serial.print(analog_moisture);
  Serial.print("    Soil Moisture(in Percentage) = ");
  Serial.print(moisture_percentage);
  Serial.println("%");
  Serial.print("Max analog: ");
  Serial.print(max_analog_moisture);
  Serial.print("    Min analog: ");
  Serial.println(min_analog_moisture);

  delay(1000);
}
