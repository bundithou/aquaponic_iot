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

#define onPin           7
#define offPin          6

#define Valvepump       11 
#define Valvetank       12 
#define Power_pump      9
#define Power_Air_Pump  8

int Delay_Time = 0;

int state = LOW;      // the system currently closed

int on_reading  ;
int off_reading;    

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  Serial.println("Booting");

  pinMode(Valvepump,          OUTPUT);
  pinMode(Valvetank,          OUTPUT);
  pinMode(Power_pump,      OUTPUT);
  pinMode(Power_Air_Pump,  OUTPUT);
  pinMode(onPin, INPUT);
  pinMode(offPin, INPUT);

  digitalWrite(Power_pump, LOW);
  digitalWrite(Power_Air_Pump, LOW); 
  digitalWrite(Valvepump, LOW);
  digitalWrite(Valvetank, LOW);

  
}

// the loop function runs over and over again forever
void loop() {

  on_reading = digitalRead(onPin);
  off_reading = digitalRead(offPin);
  
  Serial.print(state);
  Serial.print("  ");
  Serial.print(on_reading);
  Serial.print("  ");
  Serial.println(off_reading);

  if (on_reading == LOW) {                // if on_button is toggled  
    delay(50);
    if (state == LOW){                   // if system is closed
      state = HIGH;                        // open the system
      //normalSystem(true);                 // 
      Serial.println("system on");
      digitalWrite(Valvepump, HIGH);
      digitalWrite(Valvetank, HIGH);
      Serial.println("valvesHigh");
      delay(5000); 
      digitalWrite(Power_pump, HIGH);
      Serial.println("pumpHigh");
      digitalWrite(Power_Air_Pump, HIGH);
    }
  }
  
  if (off_reading == LOW){                // if off_button is toggled
    delay(50);
    if (state == HIGH){     
      state =LOW;                       // close the system
      //normalSystem(false);
      Serial.println("system off");
      digitalWrite(Power_pump, LOW);
      digitalWrite(Power_Air_Pump, LOW);
      delay(500); 
      digitalWrite(Valvepump, LOW);
      digitalWrite(Valvetank, LOW);
    }  
  }
  
  delay(1000);                             // wait for half a second
}

void normalSystem(boolean Open){
  if (Open){
    Serial.println("system on");
    digitalWrite(Valvepump, HIGH);
    digitalWrite(Valvetank, HIGH);
    Serial.println("valvesHigh");
    delay(5000); 
    digitalWrite(Power_pump, HIGH);
    Serial.println("pumpHigh");
    digitalWrite(Power_Air_Pump, HIGH);
  } else {
    Serial.println("system off");
    digitalWrite(Power_pump, LOW);
    digitalWrite(Power_Air_Pump, LOW);
    delay(500); 
    digitalWrite(Valvepump, LOW);
    digitalWrite(Valvetank, LOW);
  }
}
