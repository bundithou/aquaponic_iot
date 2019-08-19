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

#define Valve1          13
#define Valve2          12  
#define Power_pump      11
#define Power_Air_Pump  10

int Delay_Time = 0;

int state = HIGH;      // the system currently closed

int on_reading;
int off_reading;    

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  Serial.println("Booting");
  pinMode(Valve1,          OUTPUT);
  pinMode(Valve2,          OUTPUT);
  pinMode(Power_pump,      OUTPUT);
  pinMode(Power_Air_Pump,  OUTPUT);
  pinMode(onPin, INPUT);
  pinMode(offPin, INPUT);
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
    if (state == HIGH){                   // if system is closed
      state = LOW;                        // open the system
      normalSystem(true);                 // 
    }
  }
  
  if (off_reading == LOW){                // if off_button is toggled
    if (state == LOW){                    // if system is opened
      state = HIGH;                       // close the system
      normalSystem(false);
    }  
  }
  
  delay(500);                             // wait for half a second
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
