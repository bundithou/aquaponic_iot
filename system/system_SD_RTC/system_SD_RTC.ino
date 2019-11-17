#include <SPI.h>
#include <SD.h>
#include <Aqualib.h>
#include <virtuabotixRTC.h>
#include <avr/wdt.h>
////////////////
//Things that have not been done
////////////////
// -calculate water level from ultrasonic, not just leave the distance get from the ultrasonic as-is
//
//
//
////////////////

File myFile;

// oxygen sensor
#define o2pin   A2
float o2UpperBound = 4.0;
float o2LowerBound = 2.0;

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
////////////////
// SD card
#define SD_CS 53
#define MOSI 51
#define MISO 50
#define CLK 52

String logFile = "LOG.TXT";
String systemStatusFile = "SYSSTAT.TXT";

// RTC
#define RTC_CLK 23
#define RTC_IO 25
#define RTC_C_E 27

////////////////
//time
////////////////
int timeDiff = 0;
int lastRead_second = 0;
int lastRead_minute = 0;

int yOff, m, d, hh, mm, ss;

unsigned long millis_start;

////////////////
//Data Averaging
////////////////
int timeDiffRecord = 0;
#define secondsToRecord = 6;
const int numReadings = 10;
int readIndex = 0;
float readings_temp[numReadings];
//floatArrayList temperatureRecords(20);
//floatArrayList soilMoistureRecords(20);

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

#define NO_ERR 0
#define CONN_ERR 2
#define SD_CONN_ERR 3
int control_flags[] = {LOW,LOW,LOW,LOW};
int minute_control_flags[] = {LOW,LOW,LOW,LOW};
unsigned int controllable_num = 4;

//for pumping water out for P'Puwa
bool keep_on_pumping_water = LOW;

//Watchdog Helper
bool reset_needed = false;

//sensor objects
o2sensor o2(o2pin);
temperaturesensor temperature(temppin);
pHsensor pH(pHpin);
soilMoisturesensor soilMoisture(soilpin);
ultrasonicsensor ultraSonicFish(fishTrigpin, fishEchopin);
ultrasonicsensor ultraSonicTank(tankTrigpin, tankEchopin);
//RTC
virtuabotixRTC myRTC(RTC_CLK, RTC_IO, RTC_C_E);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10000);
  Serial1.begin(115200);
  //watchdog timer
  wdt_enable(WDTO_8S);
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
  myFile = SD.open(logFile, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    //hr:min:sec,O2,pH,soilMoisture,ultra_tank,ultra_fish,water_pump,air_pump,valve1,valve2,valve3
    myFile.println("Date, Time, O2, Temperature, pH, Soil moisture, Water level in water tank, Water level in fish tank, Water pump, Air pump, Valve1, Valve2, Status");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
//  myFile = SD.open(logFile);
//  if (myFile) {
//    Serial.print(logFile);
//    Serial.println(":");
//
//    // read from the file until there's nothing else in it:
//    if(Serial.available()){
//      while (myFile.available()) {
//        Serial.write(myFile.read());
//      }
//    }
//    // close the file:
//    myFile.close();
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening test.txt");
//  }

  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
//  setRealStartTime(F(__DATE__), F(__TIME__));
//  myRTC.setDS1302Time(ss, mm, hh, dayofweek(d, m, 2019), d, m, 2019);
  millis_start = millis();
  //another file for systemStatusLog
  myFile = SD.open(systemStatusFile, FILE_WRITE);
  if (myFile){
    myFile.print("System Start at ");
    myFile.print(myRTC.year);
    myFile.print(" ");
    myFile.print(myRTC.month);
    myFile.print(" ");
    myFile.print(myRTC.dayofmonth);
    myFile.print(",");
    myFile.print(myRTC.hours);
    myFile.print(":");
    myFile.print(myRTC.minutes);
    myFile.print(":");
    myFile.println(myRTC.seconds);
    myFile.print("Current microProcessor millis is");
    myFile.println(millis_start);
    myFile.close();
  }
}

void loop() {
//  Serial.println(timeDiff);
  //tell watchdog timer that the new loop iteration is started
  if(!reset_needed){
    wdt_reset();
  }
  //update time
  myRTC.updateTime();
  timeDiff = abs(lastRead_second - (myRTC.seconds + 60)) % 60;
//  Serial.print(timeDiff);
//  Serial.print(myRTC.hours);
//  Serial.print(":");
//  Serial.print(myRTC.minutes);
//  Serial.print(":");
//  Serial.println(myRTC.seconds);
  if (timeDiff >= 1){ //every second
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

    //time update
    lastRead_second = myRTC.seconds;
    timeDiffRecord++;
    
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
    
//    if(on_reading == LOW){ //on_button is toggled, force start the air pump, water pump, and valve to plantbucket
//      delay(50);
//      writeControl(air_pump, HIGH);
//      writeControl(water_pump, HIGH);
//      waterTimer = 0;
//    }
//    if(off_reading == LOW){
//      delay(50);
//      writeControl(air_pump, LOW);
//      writeControl(water_pump, LOW);
//    }

    //P' Puwa Request
    if(on_reading == LOW /*&& keep_on_pumping_water == LOW*/){
      Serial.println("on button presseed");
      delay(50);
      keep_on_pumping_water == HIGH;
    }
    if(off_reading == LOW /*&& keep_on_pumping_water == HIGH*/){
      Serial.println("off button presseed");
      delay(50);
      keep_on_pumping_water == LOW;
        writeControl(valve2, LOW);
        writeControl(water_pump, LOW);
    }

    if(keep_on_pumping_water){
        writeControl(valve1, LOW);
        writeControl(valve2, LOW);
        writeControl(water_pump, HIGH);
        writeControl(air_pump, LOW);
    }
    else{
      //Air pump
      if (control_flags[air_pump_index] && (loop_O2 > o2UpperBound)){
        writeControl(air_pump, LOW); //close
      }
      if (!control_flags[air_pump_index] && (loop_O2 < o2LowerBound)) {
        writeControl(air_pump, HIGH); //open
      }
  
      //water pump
      //if there is not enough water in the fish tank, no water should be used from it
      if(loop_ultra_fish > fishCriticalWaterLevel){
        writeControl(water_pump, LOW);
      }
      else{
        //if it is the time to water plant
        if((myRTC.hours == water_schedule_hr1 || myRTC.hours == water_schedule_hr2) && myRTC.minutes == 0 && myRTC.seconds == 0){
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
    }

    //control flag for minute cleanup
    check_minute_control_flags();

    //////////////////////////////
    //Manage value to average
    //////////////////////////////
    if(timeDiffRecord >= 6){
      timeDiffRecord = 0;
      //temperatureRecords.add(loop_temperature);
      //soilMoistureRecords.add(loop_soilMoisture);
      readings_temp[readIndex++] = loop_temperature;
      if(readIndex >= numReadings){
        readIndex = 0;
      }
    }

    
    //////////////////////////////
    //Log into SDcard
    //////////////////////////////
    if (lastRead_minute != myRTC.minutes){
      loop_temperature = average_float_array(readings_temp, numReadings);
      lastRead_minute = myRTC.minutes;
      myFile = SD.open(logFile, FILE_WRITE);
      // put your main code here, to run repeatedly:
      // if the file opened okay, write to it:
      if (myFile) {
        //year:month:dayofmonth,hr:min:sec,O2,temp,pH,soilMoisture,ultra_tank,ultra_fish,water_pump,air_pump,valve1,valve2
        myFile.print(myRTC.year);
        myFile.print("/");
        myFile.print(myRTC.month);
        myFile.print("/");
        myFile.print(myRTC.dayofmonth);
        myFile.print(",");
        myFile.print(myRTC.hours);
        myFile.print(":");
        myFile.print(myRTC.minutes);
        myFile.print(":");
        myFile.print(myRTC.seconds);
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
        myFile.print(minute_control_flags[water_pump_index]);
        myFile.print(",");
        myFile.print(minute_control_flags[air_pump_index]);
        myFile.print(",");
        myFile.print(minute_control_flags[valve1_index]);
        myFile.print(",");
        myFile.print(minute_control_flags[valve2_index]);
        myFile.print(",");
        if(!Serial1.available()){
          //if communication with ESP failed
          File sysStatFile = SD.open(systemStatusFile, FILE_WRITE);
          if(sysStatFile){
            sysStatFile.print("Serail communication with ESP errored at ");
            sysStatFile.print(myRTC.year);
            sysStatFile.print("/");
            sysStatFile.print(myRTC.month);
            sysStatFile.print("/");
            sysStatFile.print(myRTC.dayofmonth);
            sysStatFile.print(",");
            sysStatFile.print(myRTC.hours);
            sysStatFile.print(":");
            sysStatFile.print(myRTC.minutes);
            sysStatFile.print(":");
            sysStatFile.print(myRTC.seconds);
            sysStatFile.println("");
            sysStatFile.close();
          }
          myFile.println(CONN_ERR);
        }
        else{
          String str_for_esp = String(myRTC.year) + "/" + String(myRTC.month) + "/" + String(myRTC.dayofmonth)
                                        + "," + String(myRTC.hours) + ":" + String(myRTC.minutes) + ":"
                                        + String(myRTC.seconds) + "," + String(loop_O2) + "," + String(loop_temperature)
                                        + "," + String(loop_pH) + "," + String(loop_soilMoisture) + "," 
                                        + String(loop_ultra_tank) + "," + String(loop_ultra_fish) + ","
                                        + String(minute_control_flags[water_pump_index]) + "," + String(minute_control_flags[air_pump_index])
                                        + "," + String(minute_control_flags[valve1_index]) + "," + String(minute_control_flags[valve2_index])
                                        + "," + String(NO_ERR);
          Serial1.println(str_for_esp);
          delay(1000);
          Serial1.println(str_for_esp);
          myFile.println(NO_ERR);
        }
        myFile.close();
      }
      else {
        //if SDcard module failed
        //tell ESP that it's failed
        String str_for_esp = String(myRTC.year) + "/" + String(myRTC.month) + "/" + String(myRTC.dayofmonth)
                                        + "," + String(myRTC.hours) + ":" + String(myRTC.minutes) + ":"
                                        + String(myRTC.seconds) + "," + String(loop_O2) + "," + String(loop_temperature)
                                        + "," + String(loop_pH) + "," + String(loop_soilMoisture) + "," 
                                        + String(loop_ultra_tank) + "," + String(loop_ultra_fish) + ","
                                        + String(minute_control_flags[water_pump_index]) + "," + String(minute_control_flags[air_pump_index])
                                        + "," + String(minute_control_flags[valve1_index]) + "," + String(minute_control_flags[valve2_index])
                                        + "," + String(SD_CONN_ERR);
        Serial1.println(str_for_esp);
        delay(1000);
        Serial1.println(str_for_esp);
        reset_needed = true;
      }
      reset_minute_control_flags();
    }
  
    //////////////////////////////
    //Serial monitering
    //////////////////////////////
//    if(Serial.available()){
      Serial.print(myRTC.year);
      Serial.print("/");
      Serial.print(myRTC.month);
      Serial.print("/");
      Serial.print(myRTC.dayofmonth);
      Serial.print(",");
      Serial.print(myRTC.hours);
      Serial.print(":");
      Serial.print(myRTC.minutes);
      Serial.print(":");
      Serial.print(myRTC.seconds);
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
      Serial.print(",");
      if(Serial1.available()){
        Serial.print(NO_ERR);
      }
      else{
        Serial.print(CONN_ERR);
      }
      Serial.println("");
//    }
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

void setRealStartTime (const __FlashStringHelper* date, const __FlashStringHelper* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    char buff[11];
    memcpy_P(buff, date, 11);
    yOff = conv2d(buff + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (buff[0]) {
        case 'J': m = (buff[1] == 'a') ? 1 : ((buff[2] == 'n') ? 6 : 7); break;
        case 'F': m = 2; break;
        case 'A': m = buff[2] == 'r' ? 4 : 8; break;
        case 'M': m = buff[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(buff + 4);
    memcpy_P(buff, time, 8);
    hh = conv2d(buff);
    mm = conv2d(buff + 3);
    ss = conv2d(buff + 6);
}

int dayofweek(int d, int m, int y) 
{ 
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 }; 
    y -= m < 3; 
    return ( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7; 
}

uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

void check_minute_control_flags(){
  for(int i=0; i<controllable_num; i++){
    if(control_flags[i] == HIGH){
      minute_control_flags[i] = HIGH;
    }
  }
}

void reset_minute_control_flags(){
  for(int i=0; i<controllable_num; i++){
    minute_control_flags[i] = LOW;
  }
}

float average_float_array(float* arr, int arr_size){
  float total = 0;
  for(int i=0;i<arr_size;i++){
    total += arr[i];
  }
  return total/arr_size;  
}
