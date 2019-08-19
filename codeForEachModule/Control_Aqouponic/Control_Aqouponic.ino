/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

#define trigPin1  7   //2
#define echoPin1  6   //3


long duration, distance, PondSensor,TankSensor;

#define Valve1          13
#define Valve2          12  
#define Power_pump      11
#define Power_Air_Pump  10

int Delay_Time = 0;
boolean power_on = false;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin (9600);
  pinMode(Valve1,          OUTPUT);
  pinMode(Valve2,          OUTPUT);
  pinMode(Power_pump,      OUTPUT);
  pinMode(Power_Air_Pump,  OUTPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
}

// the loop function runs over and over again forever
void loop() {
  /*
  digitalWrite(Valve1, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(Valve2, HIGH);
  digitalWrite(Power_pump, HIGH);
  digitalWrite(Power_Air_Pump, HIGH);
  */
  /*
  normalSystem(true);
  delay(9000); // waiting time for filling water till the water tank is full 
  normalSystem(false);
  */
  
  SonarSensor(trigPin1, echoPin1);
  PondSensor = distance;
  delay(500);
  Serial.println(" PondSensor  ||  TankSensor");
  Serial.print("   ");
  Serial.print(PondSensor);
  if (PondSensor > 20) {
    if (!power_on){
      normalSystem(true);
      power_on = true;
    }
  }
  else {
    if (power_on){
      normalSystem(false);
      power_on= false;
    }
  }
  
 /* delay(10000);                       // wait for a second
  digitalWrite(Valve1, LOW);         delay(Delay_Time); // turn the LED off by making the voltage LOW
  digitalWrite(Valve2, LOW);         delay(Delay_Time);
  digitalWrite(Power_pump, LOW);     delay(Delay_Time);
  digitalWrite(Power_Air_Pump, LOW); delay(Delay_Time);*/
  delay(1000);                       // wait for a second
}

void normalSystem(boolean Open){
  if (Open){
    digitalWrite(Valve1, HIGH);
    digitalWrite(Valve2, HIGH);
    delay(3000); 
    digitalWrite(Power_pump, HIGH);
    digitalWrite(Power_Air_Pump, HIGH);
  } else {
    digitalWrite(Power_pump, LOW);
    digitalWrite(Power_Air_Pump, LOW);
    delay(3000); 
    digitalWrite(Valve1, LOW);
    digitalWrite(Valve2, LOW);
  }
}

void SonarSensor(int trigPin,int echoPin)
{
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distance = (duration/2) / 29.1;

}
