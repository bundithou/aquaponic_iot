#include <Aqualib.h>

// ph sensor
#define phpin   A0  
#define LED     13

// oxygen sensor
#define o2pin   A2

// soil moisture sensor
#define soilpin A1

// ultrasonic sensor
#define trigpin 12
#define echopin 11

// temperature
#define temppin 8

// call instances
pHsensor pH(phpin, LED);
o2sensor o2(o2pin, temppin); // o2sensor o2(o2pin); if no temperature pin
soilMoisturesensor soilMoisture(soilpin);
ultrasonicsensor ultrasonic(trigpin, echopin);
temperaturesensor temperature(temppin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("sensor monitoring!!");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // calculate sensors before getting the value
  pH.calculatepH();
  o2.calculateO2();
  
  // monitor sensors
  Serial.print("temperature value: ");
  Serial.print(temperature.getTemperature());
  Serial.print(" -- ");
  Serial.print("pH value: ");
  Serial.print(pH.getpH());
  Serial.print(" -- ");
  Serial.print("O2 value: ");
  Serial.print(o2.getO2());
  Serial.print(" -- ");
  Serial.print("soil moisture value: ");
  Serial.print(soilMoisture.getSoilMoisture());
  Serial.print(" -- ");
  Serial.print("ultrasonic value: ");
  Serial.println(ultrasonic.getDistance());
  delay(2000);
}
