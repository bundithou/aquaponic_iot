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
float o2UpperBound = 4.0;
float o2LowerBound = 2.0;

// soil moisture
#define soilpin A1
float soilUpperBound = 70.0;
float soilLowerBound = 25.0;

// soil pH
#define pHpin A0

// temperature
#define temppin 6

// ultrasonics
#define fishTrigpin 5
#define fishEchopin 4
#define tankTrigpin 3
#define tankEchopin 2

//Distance between water level and the sensor in cm.
//If it is lower than that, water pump will never pump water out off the fish tank.
float fishCriticalWaterLevel = 80.0;
float fishTooMuchWaterLevel = 30.0;
float fishSafeWaterLevel = 35.0;

////////////////
//peripheral devices
//*Note* We are gonna use same DS pin for both SDcard interface and Shift Reg.
////////////////
// SD card
#define SD_CS 10
#define MOSI 11
#define MISO 12
#define CLK 13

//Pin connected to latch pin (ST_CP) of 74HC595
#define SR_latchpin 9
//Pin connected to clock pin (SH_CP) of 74HC595
#define SR_clockpin 13
////Pin connected to Data in (DS) of 74HC595
#define SR_datapin  11

////////////////
//outputs connecting with Shift Reg
////////////////
static unsigned int ShiftRegisterOutData[8] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
#define water_pump 1
#define air_pump 2
#define valve1 3 //not valve L, this is valve one. I really hate this font.
#define valve2 4
#define valve3 5

//"""""timer"""""
unsigned long t = 0;
unsigned long mil = 0;

//sensor objects
o2sensor o2(o2pin);
temperaturesensor temperature(temppin);
pHsensor pH(pHpin, 7); //temporary assign unused port for pH LED pin, supposed to remove
soilMoisturesensor soilMoisture(soilpin);
ultrasonicsensor ultraSonicFish(fishTrigpin, fishEchopin);
ultrasonicsensor ultraSonicTank(tankTrigpin, tankEchopin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //pins setup
  pinMode(SD_CS,  OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(CLK, OUTPUT);
  pinMode(SR_latchpin, OUTPUT);
  //pinMode(SR_clockpin, OUTPUT); //same with CLK
  pinMode(SR_datapin, OUTPUT);
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    //while (1);
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  String filename = "LOG_";
  filename.concat(day_p);
  filename.concat(".TXT");
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    //hr:min:sec,O2,pH,soilMoisture,ultra_tank,ultra_fish,water_pump,air_pump,valve1,valve2,valve3
    myFile.println("tsm, o2_temp, temp value, soil moisture, water tank water distance, fish tank water distance, water pump, air pump, valve1, valve2, valve3");
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

    //Air pump
    if (ShiftRegisterOutData[air_pump] && (loop_O2 > o2UpperBound)){
      writeSR(air_pump, LOW); //close
    }
    if (!ShiftRegisterOutData[air_pump] && (loop_O2 < o2LowerBound)) {
      writeSR(air_pump, HIGH); //open
    }

    //valve and pump
    if(loop_ultra_fish > fishCriticalWaterLevel){
      writeSR(water_pump, LOW);
    }
    else{
      //start pump if pump is not started while there is too much water, or soil is too dry
      if(ShiftRegisterOutData[water_pump] && ( loop_ultra_fish < fishTooMuchWaterLevel || loop_soilMoisture < soilLowerBound)){
        writeSR(water_pump, HIGH);
      }

      //if any ->too much water   ->dry soil    --->open valve3
      if(loop_ultra_fish < fishTooMuchWaterLevel || loop_soilMoisture < soilLowerBound){
        writeSR(valve3, HIGH);
      }
      else{
        writeSR(valve3, LOW);
      }
    }
    
    //////////////////////////////
    //Logging
    //////////////////////////////
    String filename = "LOG_";
    filename.concat(day_p);
    filename.concat(".TXT");
    myFile = SD.open(filename, FILE_WRITE);
    // put your main code here, to run repeatedly:
    // if the file opened okay, write to it:
    if (myFile) {
      if(last_day_p != day_p){
        last_day_p = day_p;
        myFile.println("tsm, o2_temp, temp value, soil moisture, water tank water distance, fish tank water distance, water pump, air pump, valve1, valve2, valve3");
      }
      //hr:min:sec,O2,temp,pH,soilMoisture,ultra_tank,ultra_fish,water_pump,air_pump,valve1,valve2,valve3
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
      myFile.print(ShiftRegisterOutData[water_pump]);
      myFile.print(",");
      myFile.print(ShiftRegisterOutData[air_pump]);
      myFile.print(",");
      myFile.print(ShiftRegisterOutData[valve1]);
      myFile.print(",");
      myFile.print(ShiftRegisterOutData[valve2]);
      myFile.print(",");
      myFile.print(ShiftRegisterOutData[valve3]);
      myFile.println("");
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
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
    Serial.print(ShiftRegisterOutData[water_pump]);
    Serial.print(",");
    Serial.print(ShiftRegisterOutData[air_pump]);
    Serial.print(",");
    Serial.print(ShiftRegisterOutData[valve1]);
    Serial.print(",");
    Serial.print(ShiftRegisterOutData[valve2]);
    Serial.print(",");
    Serial.print(ShiftRegisterOutData[valve3]);
    Serial.println("");
  }
}

void writeSR(int ch, int output){
  ShiftRegisterOutData[ch] = output;
  digitalWrite(SR_latchpin, LOW);
  for(int i=7;i>=0;i--){
    digitalWrite(SR_datapin, ShiftRegisterOutData[i]);
    digitalWrite(SR_clockpin,HIGH);
    digitalWrite(SR_clockpin,LOW);
  }
  digitalWrite(SR_latchpin, HIGH);
}
