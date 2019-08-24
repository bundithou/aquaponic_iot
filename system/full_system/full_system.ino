
// Pin & threshold declaration

// on/off button
#define onPin           7
#define offPin          6

// valves
#define Valve1          12 // pump --> tank
#define Valve2          11 // pump --> plant

// temperature
#define tempPin         10

// pumps
#define Power_pump      9
#define Power_Air_Pump  8

//ultrasonic
#define trigPin1        3  // water tank 
#define echoPin1        2  // water tank
#define trigPin2        5  // fish tank
#define echoPin2        4  // fish tank
float top_waterlvl =    10.0;
float low_waterlvl =    60.0;

//pH
#define pHPin           A0
#define LED             13
float base_pH =         7.1;
float acid_pH =         4.9;

//soil moisture
#define soilPin         A1
float low_moisture =    30;

//Oxygen
#define oxygenPin       A2
float low_O2 =          5.0;

#define CONTROLLABLE_AMOUNT 2;   //number of environment measurement that can be controlled
/*  pos           description
 *  0             Oxygen
 *  1             Soil moisture
 */
bool lack_state[CONTROLLABLE_AMOUNT];
//**** threshold_value [controller index] [lower/upper index (0 for lower, 1 for upper)]
int threshold_value[CONTROLLABLE_AMOUNT][2] = {};   //threshold values here

int Delay_Time = 0;
int state = LOW;      // the system currently closed

int on_reading;
int off_reading;  

/*  
 *  
 *
 *
 */




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
  for(int i=0;i<CONTROLLABLE_AMOUNT;i++){
    lack_state[i] = false;
  }
}

void loop() {

  //read button status
  on_reading = digitalRead(onPin);
  off_reading = digitalRead(offPin);
  //show button status
  Serial.print(state);
  Serial.print("  ");
  Serial.print(on_reading);
  Serial.print("  ");
  Serial.println(off_reading);

  //read sensors

  //show measured values from sensors

  //check if LACK O2 flag is true
    //upper or lower threshold
  //check if O2 level is enough based on the flag (upper/lower threshold)
    //set lack O2 flag

  //check if LACK O2 flag is true
    //upper or lower threshold
  //check if O2 level is enough based on the flag (upper/lower threshold)
    //set lack O2 flag

  //write as a loop
  float packed_value[CONTROLLABLE_AMOUNT];
  /*
   * Please assign the read values into this array
   * The order should follow the index in line (around) 40
   */
  for(int i = 0; i < CONTROLLABLE_AMOUNT ; i++){
    int threshold = threshold_value[i][(lack_state[i]) ? 0 : 1];
    //compare value from sensors, this comparison can be written in switch-cases manner or just an array index looping
    //I do prefer array index loop though. - Lee
    lack_state[i] = (packed_value[i] < threshold) ? true : false;
  }

  if (on_reading == LOW) {                // if on_button is toggled  
    if (state == LOW){                   // if system is closed
      state = HIGH;                        // open the system
      for(int i=0;i<CONTROLLABLE_AMOUNT;i++){
        if(lack_state[i]){
          switch(i){ //if lack
            case 0: //Oxygen
              digitalWrite(Power_Air_Pump, HIGH); //open air pump
              break;
            case 1: //soil moisture
              digitalWrite(Valve2, HIGH); //open solenoid2
              delay(1000);
              digitalWrite(Power_pump, HIGH); //open water pump
              break;
            default: Serial.print("stop the system as soon as possible and check the amount of controllable stuffs and the number of cases");
          }
        }
        else{
          switch(i){ //if lack
            case 0: //Oxygen
              digitalWrite(Power_Air_Pump, LOW); //close air pump
              break;
            case 1: //soil moisture
              digitalWrite(Power_pump, LOW); //close water pump
              delay(1000);
              digitalWrite(Valve2, LOW); // close solenoid
              break;
            default: Serial.print("stop the system as soon as possible and check the amount of controllable stuffs and the number of cases");
          }
        }
      }
    }
  }
  
  if (off_reading == LOW){                // if off_button is toggled
    if (state == HIGH){                    // if system is opened
      state = LOW;                       // close the system
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
    digitalWrite(Valve1, LOW);
    digitalWrite(Valve2, HIGH);
    digitalWrite(Valve3, LOW);
    delay(3000); 
    digitalWrite(Power_pump, HIGH);
    digitalWrite(Power_Air_Pump, HIGH);
}

void fillWaterTank(){
  digitalWrite(Valve1, HIGH);
  digitalWrite(Valve2, LOW);
  digitalWrite(Valve3, LOW);
  delay(3000); 
  digitalWrite(Power_pump, HIGH);
}


void fillSoilMoisture(){
  digitalWrite(Valve1, LOW);
  digitalWrite(Valve2, LOW);
  digitalWrite(Valve3, HIGH);
}

void fillOxygen(){
  digitalWrite(Power_Air_Pump, HIGH);
}
