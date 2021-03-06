
#define trigPin1  5   //2
#define echoPin1  4   //3
#define trigPin2  3   //4 
#define echoPin2  2   //5


long duration, PondSensor,TankSensor;
double distance;
void setup()
{ 
Serial.begin (9600);
pinMode(trigPin1, OUTPUT);
pinMode(echoPin1, INPUT);
pinMode(trigPin2, OUTPUT);
pinMode(echoPin2, INPUT);
}


void loop() {
SonarSensor(trigPin1, echoPin1);
PondSensor = distance;
delay(500);
SonarSensor(trigPin2, echoPin2);
TankSensor = distance;
delay(500);
Serial.println(" PondSensor  ||  TankSensor");
Serial.print("   ");
Serial.print(PondSensor);
  Serial.print("           ||   ");
Serial.println(TankSensor);

}

void SonarSensor(int trigPin,int echoPin)
{
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distance = (duration/2.0) / 29.1;

}
