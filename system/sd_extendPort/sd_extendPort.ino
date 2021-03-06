#include <SPI.h>
#include <SD.h>
#include <Aqualib.h>

////////////////
//Things that have not been done
////////////////
// -only pump water from fish tank as routine, use water tank for when the soil is abnormally dry.
// -fill fish tank water from the water tank when there is not enough water for fish
// -calculate water level from ultrasonic, not just leave the distance get from the ultrasonic as-is
// 
//
//
//
////////////////

File myFile;
unsigned int min_p = 0;
unsigned int sec_p = 0;
unsigned int hr_p = 0;
unsigned int day_p = 0;
unsigned int last_day_p = 0;

// oxygen sensor
#define o2pin   A2
float o2UpperBound = 1.8;
float o2LowerBound = 1.2;

// soil moisture
#define soilpin A1
float soilUpperBound = 80.0;
float soilLowerBound = 30.0;

// soil pH
#define pHpin A0

// temperature
#define temppin 6

// ultrasonics
#define fishTrigpin 5
#define fishEchopin 4
#define tankTrigpin 3
#define tankEchopin 2

// on/off buttons
#define buttonOn 9
#define buttonOff 8
 
//Distance between water level and the sensor in cm.
//If it is lower than that, water pump will never pump water out off the fish tank.
float fishCriticalWaterLevel = 80.0;
float fishTooMuchWaterLevel = 40.0;
float fishSafeWaterLevel = 45.0;

float waterTankTooLessWater = 33.0;

////////////////
//peripheral devices 

//*Note* We are gonna use same DS pin for both SDcard interface and Shift Reg.
////////////////
// SD card
#define SD_CS 53
#define MOSI 51
#define MISO 50
#define CLK 52

////////////////
//System control
////////////////
#define water_pump 22
#define air_pump 24
#define valve1 26 //not valve L, this is valve one. I really hate this font.
#define valve2 28

#define water_schedule_hr1 6  //6am
#define water_schedule_hr2 14 //2pm
#define max_watering_time  15 //30seconds
unsigned int waterTimer = 0;

#define water_pump_index 0
#define air_pump_index 1
#define valve1_index 2
#define valve2_index 3

int control_flags[] = {LOW,LOW,LOW,LOW};

//"""""timer"""""
unsigned long t = 0;
unsigned long mil = 0;
unsigned long t_SD = 0;
unsigned long mil_SD = 0;

//sensor objects
o2sensor o2(o2pin);
temperaturesensor temperature(temppin);
pHsensor pH(pHpin);
soilMoisturesensor soilMoisture(soilpin);
ultrasonicsensor ultraSonicFish(fishTrigpin, fishEchopin);
ultrasonicsensor ultraSonicTank(tankTrigpin, tankEchopin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //pins setup
  pinMode(buttonOn, INPUT);
  pinMode(buttonOff, INPUT);
  pinMode(SD_CS,  OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(CLK, OUTPUT);

  pinMode(water_pump, OUTPUT);
  pinMode(air_pump, OUTPUT);
  pinMode(valve1, OUTPUT);
  pinMode(valve2, OUTPUT);
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    //while (1);
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  String filename = "LOG_";
  filename.concat(day_p)  ;
  filename.concat(".TXT");
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    //hr:min:sec,O2,pH,soilMoisture,ultra_tank,ultra_fish,water_pump,air_pump,valve1,valve2,valve3
    myFile.println("tsm, o2_temp, temp value, pH, soil moisture, water tank water distance, fish tank water distance, water pump, air pump, valve1, valve2, valve3");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.print(filename);
    Serial.println(":");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  t = millis();
}

void loop() {

  //////////////////////////////
  //Reading from sensors
  //////////////////////////////
  float loop_temperature = temperature.getTemperature();
  pH.calculatepH();
  o2.calculateO2(loop_temperature);

  float loop_pH = pH.getpH();
  float loop_O2 = o2.getO2();
  float loop_soilMoisture = soilMoisture.getSoilMoisture();
  float loop_ultra_tank = ultraSonicTank.getDistance();
  float loop_ultra_fish = ultraSonicFish.getDistance();
  
  if ((mil = (millis())) - t >= 1000){
    unsigned long t_offset = mil-t-1000;
//    Serial.print("mil,offset: ");
//    Serial.print(mil);
//    Serial.print(",");
//    Serial.println(t_offset);
    t = mil-t_offset;
    
    //////////////////////////////
    //"""""time"""""
    //////////////////////////////
    sec_p++;
    if(sec_p >= 60){
      sec_p=0;
      min_p++;
    }
    if(min_p >= 60){
      min_p = 0;
      hr_p++; 
    }
    if(hr_p >= 24){
      hr_p = 0;
      day_p++;
    }
      
    //////////////////////////////
    //Control logic & command
    //////////////////////////////

    //Force start/stop
    int on_reading = digitalRead(buttonOn);
    int off_reading = digitalRead(buttonOff);

    if(waterTimer >= 15){
      writeControl(water_pump, LOW);
    }
    else if(control_flags[water_pump_index]){
      waterTimer++;
    }
    
    if(on_reading == LOW){ //on_button is toggled, force start the air pump, water pump, and valve to plantbucket
      delay(50);
      writeControl(air_pump, HIGH);
      writeControl(water_pump, HIGH);
      waterTimer = 0;
    }
    if(off_reading == LOW){
      delay(50);
      writeControl(air_pump, LOW);
      writeControl(water_pump, LOW);
    }

    //Air pump
    if (control_flags[air_pump_index] && (loop_O2 > o2UpperBound)){
      writeControl(air_pump, LOW); //close
    }
    if (!control_flags[air_pump_index] && (loop_O2 < o2LowerBound)) {
      writeControl(air_pump, HIGH); //open
    }

    //water pump
    if(loop_ultra_fish > fishCriticalWaterLevel){
      writeControl(water_pump, LOW);
      
    }
    else{
      //if it is the time to water plant
      if((hr_p == water_schedule_hr1 || hr_p == water_schedule_hr2) && min_p == 0 && sec_p == 0){
        writeControl(water_pump, HIGH);
      }
    }

    //valve
    if(loop_soilMoisture < soilLowerBound){
      writeControl(valve2, HIGH);
    }
    else if(loop_soilMoisture > soilUpperBound){
      writeControl(valve2, LOW);
    }

    if(loop_ultra_fish > fishCriticalWaterLevel && loop_ultra_tank < waterTankTooLessWater){
      writeControl(valve1, HIGH);
    }
    else{
      writeControl(valve1, LOW);
    }
    
    //////////////////////////////
    //Logging
    //////////////!////////////////
    if ((mil_SD = (millis())) - t_SD >= 60000){
      unsigned long t_offset_SD = mil_SD - t_SD-60000;
      t_SD = mil_SD - t_offset_SD;
      String filename = "LOG_";
      filename.concat(day_p);
      filename.concat(".TXT");
      myFile = SD.open(filename, FILE_WRITE);
      // put your main code here, to run repeatedly:
      // if the file opened okay, write to it:
      if (myFile) {
        if(last_day_p != day_p){
          last_day_p = day_p;
          myFile.println("tsm, o2_temp, temp value, pH, soil moisture, water tank water distance, fish tank water distance, water pump, air pump, valve1, valve2");
        }
        //hr:min:sec,O2,temp,pH,soilMoisture,ultra_tank,ultra_fish,water_pump,air_pump,valve1,valve2,
        myFile.print(hr_p);
        myFile.print(":");
        myFile.print(min_p);
        myFile.print(":");
        myFile.print(sec_p);
        myFile.print(",");
        myFile.print(loop_O2);
        myFile.print(",");
        myFile.print(loop_temperature);
        myFile.print(",");
        myFile.print(loop_pH);
        myFile.print(",");
        myFile.print(loop_soilMoisture);
        myFile.print(",");
        myFile.print(loop_ultra_tank);
        myFile.print(",");
        myFile.print(loop_ultra_fish);
        myFile.print(",");
        myFile.print(control_flags[water_pump_index]);
        myFile.print(",");
        myFile.print(control_flags[air_pump_index]);
        myFile.print(",");
        myFile.print(control_flags[valve1_index]);
        myFile.print(",");
        myFile.print(control_flags[valve2_index]);
        myFile.println("");
        myFile.close();
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
      }
    }
  
    //////////////////////////////
    //Serial monitering
    //////////////////////////////
    Serial.print(hr_p);
    Serial.print(":");
    Serial.print(min_p);
    Serial.print(":");
    Serial.print(sec_p);
    Serial.print(",");
    Serial.print(loop_O2);
    Serial.print(",");
    Serial.print(loop_temperature);
    Serial.print(",");
    Serial.print(loop_pH);
    Serial.print(",");
    Serial.print(loop_soilMoisture);
    Serial.print(",");
    Serial.print(loop_ultra_tank);
    Serial.print(",");
    Serial.print(loop_ultra_fish);
    Serial.print(",");
    Serial.print(control_flags[water_pump_index]);
    Serial.print(",");
    Serial.print(control_flags[air_pump_index]);
    Serial.print(",");
    Serial.print(control_flags[valve1_index]);
    Serial.print(",");
    Serial.print(control_flags[valve2_index]);
    Serial.println("");
  }
}

void writeControl(int ch, int output){
  int index = 0;
  switch(ch){
    case water_pump: index = 0; break;
    case air_pump: index  = 1; break;
    case valve1: index = 2; break;
    case valve2: index = 3; break;
  }
  control_flags[index] = output;
  digitalWrite(ch, output);
}
