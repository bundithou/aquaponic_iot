
// on/off button
#define onPin           7
#define offPin          6

// valves
#define Valve1          13 // pond --> tank
#define Valve2          12 // pond --> plant
#define Valve3          11 // tank --> pond

// pumps
#define Power_pump      10
#define Power_Air_Pump  9

//ultrasonic
#define trigPin1        3  // tank 
#define echoPin1        2  // tank

int Delay_Time = 0;

int state = HIGH;      // the system currently closed

int on_reading;
int off_reading;    


void setup() {
  
  Serial.begin(115200);
  Serial.println("Booting");
  pinMode(Valve1,          OUTPUT);
  pinMode(Valve2,          OUTPUT);
  pinMode(Valve3,          OUTPUT);
  pinMode(Power_pump,      OUTPUT);
  pinMode(Power_Air_Pump,  OUTPUT);
  pinMode(onPin,           INPUT);
  pinMode(offPin,          INPUT);
}

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
      wateringPlant(true);                 // 
    }
  }
  
  if (off_reading == LOW){                // if off_button is toggled
    if (state == LOW){                    // if system is opened
      state = HIGH;                       // close the system
      wateringPlant(false);
    }  
  }
  
  delay(500);                             // wait for half a second
}

void closeSystem(){
  digitalWrite(Power_pump, LOW);
  digitalWrite(Power_Air_Pump, LOW);
  delay(3000); 
  digitalWrite(Valve1, LOW);
  digitalWrite(Valve2, LOW);
  digitalWrite(Valve3, LOW);
}

void wateringPlant(boolean Open){
  if (Open){
    digitalWrite(Valve1, LOW);
    digitalWrite(Valve2, HIGH);
    digitalWrite(Valve3, LOW);
    delay(3000); 
    digitalWrite(Power_pump, HIGH);
    digitalWrite(Power_Air_Pump, HIGH);
  } else {
    closeSystem();
  }
}

void fillWaterTank(){
  
  digitalWrite(Valve1, HIGH);
  digitalWrite(Valve2, LOW);
  digitalWrite(Valve3, LOW);
  delay(3000); 
  digitalWrite(Power_pump, HIGH);

  // wait till tank is full

  //close
  closeSystem();
  
}


void fillSoilMoisture(){
  digitalWrite(Valve1, LOW);
  digitalWrite(Valve2, LOW);
  digitalWrite(Valve3, HIGH);

  // wait till soilmoisture value is better
  delay(3000);
  
  //close
  closeSystem();
  
}

void fillOxygen(){
  digitalWrite(Power_Air_Pump, HIGH);

  // wait till Oxygen value is better
}
