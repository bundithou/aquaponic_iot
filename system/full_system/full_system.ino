#include <Aqualib.h>

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
float lower_moisture =    50;
float upper_moisture =    90;

//Oxygen
#define oxygenPin       A2
float lower_O2 =          4.0;
float upper_O2 =          5.0;

const int CONTROLLABLE_AMOUNT = 2;   //number of environment measurement that can be controlled
/*  pos           description
 *  0             Oxygen
 *  1             Soil moisture
 */
bool lack_state[CONTROLLABLE_AMOUNT];
bool lack_working[CONTROLLABLE_AMOUNT];
//**** threshold_value [controller index] [lower/upper index (0 for lower, 1 for upper)]
float threshold_value[CONTROLLABLE_AMOUNT][2];

int Delay_Time = 0;
int state = LOW;      // the system currently closed

int on_reading;
int off_reading;  

/*  
 *  
 *
 *
 */

pHsensor pH(pHPin, LED);
o2sensor o2(oxygenPin);
soilMoisturesensor soilMoisture(soilPin);
ultrasonicsensor ultrasonicWaterTank(trigPin1, echoPin1);
ultrasonicsensor ultrasonicFishTank(trigPin2, echoPin2);

void setup() {
  
  Serial.begin(115200);
  Serial.println("Booting");
  pinMode(Valve1,          OUTPUT);
  pinMode(Valve2,          OUTPUT);
  //pinMode(Valve3,          OUTPUT);
  pinMode(Power_pump,      OUTPUT);
  pinMode(Power_Air_Pump,  OUTPUT);
  pinMode(onPin,           INPUT);
  pinMode(offPin,          INPUT);
  for(int i=0;i<CONTROLLABLE_AMOUNT;i++){
    lack_state[i] = false;
    lack_working[i] = false;
  }
  
  threshold_value[0][0] = lower_O2;
  threshold_value[0][1] = upper_O2;
  threshold_value[1][0] = lower_moisture;
  threshold_value[1][1] = upper_moisture;
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

  // calculate sensors before getting the value
  pH.calculatepH();
  o2.calculateO2();

  float loop_pH = pH.getpH();
  float loop_O2 = o2.getO2();
  float loop_soilMoisture = soilMoisture.getSoilMoisture();
  float loop_ultra_water_tank = ultrasonicWaterTank.getDistance();
  float loop_ultra_fish_tank = ultrasonicFishTank.getDistance();

  // monitor sensors
  Serial.print("pH value: ");
  Serial.print(loop_pH);
  Serial.print(" -- ");
  Serial.print("O2 value: ");
  Serial.print(loop_O2);
  Serial.print(" -- ");
  Serial.print("soil moisture value: ");
  Serial.print(loop_soilMoisture);
  Serial.print(" -- ");
  Serial.print("water tank ultrasonic value: ");
  Serial.print(loop_ultra_water_tank);
  Serial.print(" -- ");
  Serial.print("fish tank ultrasonic value: ");
  Serial.println(loop_ultra_fish_tank);


  float packed_value[CONTROLLABLE_AMOUNT];
  /*
   * Please assign the read values into this array
   * The order should follow the index in line (around) 40
   * Note: in final code (or in case that there is not enough memory)
   * the read values from sensors can be assigned into this array directly.
   */
  packed_value[0] = loop_O2;
  packed_value[1] = loop_soilMoisture;
  
  for(int i = 0; i < CONTROLLABLE_AMOUNT ; i++){
    float threshold = threshold_value[i][(lack_working[i]) ? 0 : 1];
    //compare value from sensors, this comparison can be written in switch-cases manner or just an array index looping
    //I do prefer array index loop though. - Lee
    lack_state[i] = (packed_value[i] < threshold) ? true : false;
  }

  if (on_reading == LOW) {                // if on_button is toggled  
    if (state == LOW){                   // if system is closed
      state = HIGH;                        // open the system
    }
  }
  
  if (off_reading == LOW){                // if off_button is toggled
    if (state == HIGH){                    // if system is opened
      state = LOW;                       // close the system
      stopAndResetControllSystem();
    }  
  }

  if(state == HIGH){
    for(int i=0;i<CONTROLLABLE_AMOUNT;i++){
      if(lack_state[i]){
        switch(i){ //if lack
          case 0: //Oxygen
            digitalWrite(Power_Air_Pump, HIGH); //open air pump
            lack_working[0] = true;
            break;
          case 1: //soil moisture
            digitalWrite(Valve2, HIGH); //open solenoid2
            delay(1000);
            digitalWrite(Power_pump, HIGH); //open water pump
            lack_working[1] = true;
            break;
          default: Serial.print("stop the system as soon as possible and check the amount of controllable stuffs and the number of cases");
        }
      }
      else{
        switch(i){ //if enough
          case 0: //Oxygen
            digitalWrite(Power_Air_Pump, LOW); //close air pump
            lack_working[0] = false;
            break;
          case 1: //soil moisture
            digitalWrite(Power_pump, LOW); //close water pump
            delay(1000);
            digitalWrite(Valve2, LOW); // close solenoid
            lack_working[1] = false;
            break;
          default: Serial.print("stop the system as soon as possible and check the amount of controllable stuffs and the number of cases");
        }
      }
    }
  }
  
  delay(500);                             // wait for half a second before next loop
}

void stopAndResetControllSystem(){
  digitalWrite(Power_Air_Pump, LOW); //close air pump
  digitalWrite(Power_pump, LOW); //close water pump
  delay(1000);
  digitalWrite(Valve2, LOW); // close solenoid
  for(int i=0;i<CONTROLLABLE_AMOUNT;i++){
    lack_state[i] = false;
    lack_working[i] = false;
  }
}
