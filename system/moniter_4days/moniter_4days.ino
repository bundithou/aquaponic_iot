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

// on/off buttons
#define buttonOn 8
#define buttonOff 7

#define air_pump 9
bool air_pump_working = false;

////////////////
//peripheral devices
//*Note* We are gonna use same DS pin for both SDcard interface and Shift Reg.
////////////////
// SD card
#define SD_CS 10
#define MOSI 11
#define MISO 12
#define CLK 13

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
  pinMode(air_pump, OUTPUT);
  digitalWrite(air_pump, LOW);
  
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
    myFile.println("tsm, o2_temp, temp value, pH, soil moisture, water tank water distance, fish tank water distance, air pump");
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
    if(on_reading == LOW){ //on_button is toggled, force start the air pump, water pump, and valve to plantbucket
      delay(50);
      digitalWrite(air_pump, HIGH);
//      writeSR(valve3, HIGH);
//      delay(50);
//      writeSR(water_pump, HIGH);
    }
    if(off_reading == LOW){
      delay(50);
      digitalWrite(air_pump, LOW);
//      writeSR(water_pump, LOW);
//      delay(50);
//      writeSR(valve3, LOW);
    }
    
    //////////////////////////////
    //Logging
    //////////////////////////////
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
          myFile.println("tsm, o2_temp, temp value, pH, soil moisture, water tank water distance, fish tank water distance, air_pump");
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
        myFile.print(air_pump_working);
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
    Serial.print(air_pump_working);
    Serial.println("");
  }
}
