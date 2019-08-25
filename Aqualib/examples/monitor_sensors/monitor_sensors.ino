#include <Aqualib.h>
#include <OneWire.h>

#define onewirebus 8
OneWire oneWire(onewirebus);

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

// call instances
pHsensor pH(phpin, LED);
o2sensor o2(o2pin, &oneWire);
soilMoisturesensor soilMoisture(soilpin);
ultrasonicsensor ultrasonic(trigpin, echopin);


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
