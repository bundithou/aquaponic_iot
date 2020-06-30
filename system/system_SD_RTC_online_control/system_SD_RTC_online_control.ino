#include <SPI.h>
#include <SD.h>
#include <Aqualib.h>
#include <virtuabotixRTC.h>
#include <avr/wdt.h>
#include <QueueList.h>

File myFile;

// oxygen sensor
#define o2pin   A2
float o2UpperBound = 7.0;
float o2LowerBound = 5.0;
float loop_O2;

// soil moisture
#define soilpin A1
float soilUpperBound = 70.0;
float soilLowerBound = 30.0;
float loop_soilMoisture;

// soil pH
#define pHpin A0
float loop_pH;

// temperature
#define temppin 6
float loop_temperature;

// ultrasonics
#define fishTrigpin 5
#define fishEchopin 4
#define tankTrigpin 3
#define tankEchopin 2
float loop_ultra_fish;
float loop_ultra_tank;

// on/off buttons
#define buttonOn 9
#define buttonOff 8

//ESP EN pin
#define espEnpin 29

//Distance between water level and the sensor in cm.
//If it is lower than that, water pump will never pump water out off the fish tank.
float fishCriticalWaterLevel = 60.0;
float fishTooMuchWaterLevel = 35.0;
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

String logFile = "LOG";
String logFileExtension = ".TXT";
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
unsigned long t = 0;
unsigned long mil = 0;

////////////////
//Data Averaging
////////////////
int timeDiffRecord = 0;
int lastTimeDiffRecord = 0;
#define secondsToRecord = 6;
const int numReadings = 10;
int readIndex = 0;
float readings_temp[numReadings];
float total_temp = 0;
float average_temp;
float readings_soil_mositure[numReadings];
float total_soil_mositure = 0;
float average_soil_mositure;

////////////////
//System control
////////////////
#define water_pump 22
#define air_pump 24
#define valve1 26 //not valve L, this is valve one. I really hate this font.
#define valve2 28

#define water_schedule_hr1 6  //6am
#define water_schedule_hr2 14 //2pm
#define water_level_schedule_hr1 5
#define water_level_schedule_hr2 13
#define soil_water_schedule_hr1 10
#define soil_water_schedule_hr2 22
#define max_watering_time  15 //15seconds
unsigned int manual_water_time = 0;
unsigned int waterTimer = 0;
float max_soil_moisture_after_watering = 0.0;
float max_soil_moisture_after_watering_threshold = 80.0;

#define air_pump_checking_schedule1 6 //6am
#define air_pump_checking_schedule2 18 //6pm
#define expected_half_daily_air_pump_minute 360 //360minutes, 6 hours
#define expected_half_daily_air_pump_rest_minute 180 //3hours
long airNeededTimer = 0;
long airTimer = 0;

#define water_pump_index 0
#define air_pump_index 1
#define valve1_index 2
#define valve2_index 3
unsigned int manual_valve1_time = 0;
unsigned int manual_valve2_time = 0;
unsigned int valve1Timer = 0;
unsigned int valve2Timer = 0;

#define NO_ERR 0
#define CONN_ERR 2
#define SD_CONN_ERR 3
bool automatic_control = true; //flag for automatic control / online manual control
bool on_button_routine_flag = false;
bool off_button_routine_flag = true;
int control_flags[] = {LOW, LOW, LOW, LOW};
int minute_control_flags[] = {LOW, LOW, LOW, LOW};
unsigned int controllable_num = 4;

//commands
#define ONLINE_MANUAL_CONTROL     100
#define AUTOMATIC_CONTROL         200
#define OPEN_WATER_PUMP           111
#define CLOSE_WATER_PUMP          110
#define OPEN_AIR_PUMP             121
#define CLOSE_AIR_PUMP            120
#define OPEN_VALVE1               131
#define CLOSE_VALVE1              130
#define OPEN_VALVE2               141
#define CLOSE_VALVE2              140
#define TIME_SET                  300
#define DO_THRESHOLD_SET          310
#define DO_THRESHOLD_SET_RESPONSE 311
#define ACTUATORS_STATUS_REQUEST  400
#define CONTROL_STATUS_RESPONSE   500


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
//Queue for String that are to be sent to ESP bufferring
QueueList <String> stringQueue;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial1.begin(115200);

  wdt_enable(WDTO_8S);

  pinMode(buttonOn, INPUT);
  pinMode(buttonOff, INPUT);
  pinMode(SD_CS,  OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(CLK, OUTPUT);
  pinMode(espEnpin, OUTPUT);

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

  //delay(1000);
  myFile = SD.open(create_log_file_name(), FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    //year/month/date,hr:min:sec,O2,pH,soilMoisture,ultra_tank,ultra_fish,water_pump,air_pump,valve1,valve2,valve3
    myFile.println("Date, Time, O2, Temperature, pH, Soil moisture, Water level in water tank, Water level in fish tank, Water pump, Air pump, Valve1, Valve2, Status");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
  //setRealStartTime(F(__DATE__), F(__TIME__));
  //myRTC.setDS1302Time(ss, mm, hh, dayofweek(d, m, 2020), d, m, 2020);
  millis_start = millis();
  myFile = SD.open(systemStatusFile, FILE_WRITE);
  if (myFile){
    myFile.print("System Start at ");
    myFile.print(myRTC.year);
    myFile.print("/");
    myFile.print(myRTC.month);
    myFile.print("/");
    myFile.print(myRTC.dayofmonth);
    myFile.print(" ");
    myFile.print(myRTC.hours);
    myFile.print(":");
    myFile.print(myRTC.minutes);
    myFile.print(":");
    myFile.println(myRTC.seconds);
    myFile.print("Current microProcessor millis is");
    myFile.println(millis_start);
    myFile.close();
  }

  for(int i=0;i<10;i++){
    get_sensors_value();
    add_smoothing_data();
    delay(100);
    watchdog_reset(reset_needed);
  }
}

void loop() {
  watchdog_reset(reset_needed);
  update_time();
  if ((mil = (millis())) - t >= 1000){
    unsigned long t_offset = mil-t-1000;
    t = mil-t_offset;
    check_esp_message();
  }
  if (timeDiff >= 1) {
    lastRead_second = myRTC.seconds;
    get_sensors_value();
    get_button_routine_selector();
    if (on_button_routine_flag) {
      on_button_routine();
    }
    else if (off_button_routine_flag) {
      off_button_routine();
    }
    check_minute_control_flags();
    if(timeDiffRecord >= 6){
      lastTimeDiffRecord = myRTC.seconds;
      add_smoothing_data();
    }
    if((myRTC.hours == water_schedule_hr1 || myRTC.hours == water_schedule_hr2)
          && myRTC.minutes == 0){
      if(myRTC.seconds == 0 || myRTC.seconds == 1){
        max_soil_moisture_after_watering = 0.0;
      }
      if(loop_soilMoisture > max_soil_moisture_after_watering){
        max_soil_moisture_after_watering = loop_soilMoisture;
      }
    }
    if((myRTC.hours == water_schedule_hr1 || myRTC.hours == water_schedule_hr2)
          && myRTC.minutes == 1 && (myRTC.seconds == 0 || myRTC.seconds == 1)){
      if(max_soil_moisture_after_watering < max_soil_moisture_after_watering_threshold){
        stringQueue.push("600_water pump is not working\n");
      }
    }
    if(lastRead_minute != myRTC.minutes){
      //minute check for air pump
      //for air pump timer that we really need to have it runs for xxx minutes each day
      lastRead_minute = myRTC.minutes;
      if(minute_control_flags[1]){
        airTimer++;
        airNeededTimer--;
      }
      log_into_SDcard();
      reset_minute_control_flags();
    }
    print_for_serial_monitor();
    dequeue_all_string_to_esp();
  }
}

void watchdog_reset(bool reset_needed) {
  if (!reset_needed) {
    wdt_reset();
  }
}

void check_esp_message(){
    if(Serial1.available() > 0){
      char bfr[501];
      memset(bfr,0, 501);
      Serial1.readBytesUntil('\n',bfr,500);
      String message(bfr);
      Serial.println(message);
      //try open the file
      myFile = SD.open(message+logFileExtension);
      if(myFile){
        unsigned int lineCount = 0;
        while(myFile.available()){
          Serial1.println(myFile.read());
          Serial.println("resending logFile");
          delay(5);
          lineCount++;
          //file may take long time to be transferred
          //this part may cause bug in the program though
          watchdog_reset(reset_needed);
          update_time();
          if(timeDiff >= 1){
            lastRead_second = myRTC.seconds;
            if(lastRead_minute != myRTC.minutes){
              lastRead_minute = myRTC.minutes;
              get_sensors_value();
              check_minute_control_flags();
              log_into_SDcard();
              reset_minute_control_flags();
            }
          }
        }
        myFile.close();
        myFile = SD.open(systemStatusFile, FILE_WRITE);
        if (myFile){
          myFile.print("LogFile:");
          myFile.print(message);
          myFile.print(" ReturnedLines:");
          myFile.println(lineCount);
          myFile.close();
        }
      }
      else{
        //no such file -> command
        online_command(bfr);
      }
    }
}

void update_time() {
  myRTC.updateTime();
  timeDiff = abs(lastRead_second - (myRTC.seconds + 60)) % 60;
  timeDiffRecord = abs(lastTimeDiffRecord - (myRTC.seconds + 60)) % 60;
}

void get_sensors_value() {
  loop_temperature = temperature.getTemperature();
  pH.calculatepH();
  o2.calculateO2(loop_temperature);
  loop_pH = pH.getpH();
  loop_O2 = o2.getO2();
  loop_soilMoisture = soilMoisture.getSoilMoisture();
  loop_ultra_tank = ultraSonicTank.getDistance();
  loop_ultra_fish = ultraSonicFish.getDistance();
}

void get_button_routine_selector() {
  bool on_reading = digitalRead(buttonOn);
  bool off_reading = digitalRead(buttonOff);
  if (on_reading == LOW) {
    if (myRTC.hours > 6 && myRTC.hours < 18){
      //change to on button routine
      //on button routine can only occurs between 8:00 to 16:00 of monday to saturday though
      on_button_routine_flag = true;
      off_button_routine_flag = false;
    }else{
      change_on_to_off_routine();
      on_button_routine_flag = false;
      off_button_routine_flag = true;
    }
  }
  else if (off_reading == LOW) {
    //change to off button 
    change_on_to_off_routine();
    on_button_routine_flag = false;
    off_button_routine_flag = true;
  }
}

void on_button_routine() {
  writeControl(water_pump, HIGH);
  writeControl(valve1, LOW);
  writeControl(valve2, LOW);
  writeControl(air_pump, LOW);
}

void change_on_to_off_routine(){
  writeControl(water_pump, LOW);
}

void off_button_routine() {
  check_air_pump_control();
  check_water_pump_control();
  check_valves_control();
  check_minute_control_flags();
}

void check_air_pump_control() {
  if(automatic_control){
    if((myRTC.hours == air_pump_checking_schedule1 || myRTC.hours == air_pump_checking_schedule2)
            && myRTC.minutes == 0 && (myRTC.seconds == 0/* || myRTC.seconds == 1*/)){
      //routine based air pump checking
      if(airTimer < expected_half_daily_air_pump_minute){
        airNeededTimer = expected_half_daily_air_pump_minute - airTimer;
      }
      airTimer = -airNeededTimer;
    }
    else if(airNeededTimer <= 0){
      if (loop_O2 < o2LowerBound && airTimer < expected_half_daily_air_pump_rest_minute) {
        writeControl(air_pump, HIGH); //open
      }
      else if(loop_O2 > o2UpperBound) {
        writeControl(air_pump, LOW); //close
      }
    }
    else{
      writeControl(air_pump, HIGH); //open
    }
  }else{
    //do nothing(?)
  }
}

void check_water_pump_control() {
  if(automatic_control){
    if (loop_ultra_fish > fishCriticalWaterLevel) {
      writeControl(water_pump, LOW);
    }
    else if ((myRTC.hours == water_schedule_hr1 || myRTC.hours == water_schedule_hr2)
            && myRTC.minutes == 0 && (myRTC.seconds == 0 || myRTC.seconds == 1)) {
      writeControl(water_pump, HIGH);
      waterTimer = 0;
    }
  }

  if (control_flags[water_pump_index] == HIGH) {
    waterTimer++;
  }

  if (!automatic_control && waterTimer > manual_water_time){
    //for online manual control
    writeControl(water_pump, LOW);
    waterTimer = 0;
  }
  else if (waterTimer > max_watering_time) {
    //for automatic control
    writeControl(water_pump, LOW);
    waterTimer = 0;
  }
}

void check_valves_control() {
  //valve
  if(automatic_control){
    if ((myRTC.hours == soil_water_schedule_hr1 || myRTC.hours == soil_water_schedule_hr2)
        && myRTC.minutes == 0 && myRTC.seconds == 0) {
      if (loop_soilMoisture < soilLowerBound) {
        writeControl(valve2, HIGH);
        valve2Timer = 0;
      }
    }

    if ((myRTC.hours == water_level_schedule_hr1 || myRTC.hours == water_level_schedule_hr2)
        && myRTC.minutes == 45 && myRTC.seconds == 0) {
      if (loop_ultra_fish > fishCriticalWaterLevel) {
        writeControl(valve1, HIGH);
        valve1Timer = 0;
      }
    }
  }

  if (control_flags[valve1_index] == HIGH) {
    valve1Timer++;
  }
  if (control_flags[valve2_index] == HIGH) {
    valve2Timer++;
  }

  if(!automatic_control){
    if(valve1Timer > manual_valve1_time){
      writeControl(valve1, LOW);
    }
    if(valve2Timer > manual_valve2_time){
      writeControl(valve2, LOW);
    }
  }
  else{
    if (valve1Timer > 20) {
      writeControl(valve1, LOW);
    }
    if (valve2Timer > 10) {
      writeControl(valve2, LOW);
    }
  }

}

void add_smoothing_data() {
  total_temp = total_temp - readings_temp[readIndex];
  total_soil_mositure = total_soil_mositure - readings_soil_mositure[readIndex];
  readings_temp[readIndex] = loop_temperature;
  readings_soil_mositure[readIndex] = loop_soilMoisture;
  total_temp = total_temp + loop_temperature;
  total_soil_mositure = total_soil_mositure + loop_soilMoisture;
  readIndex++;

  if(readIndex >= numReadings){
    readIndex = 0;
  }

  average_smoothing_data();
}

void average_smoothing_data(){
  average_temp = total_temp / numReadings;
  average_soil_mositure = total_soil_mositure / numReadings;
}

void log_into_SDcard(){
  myFile = SD.open(create_log_file_name(), FILE_WRITE);
  if(myFile){
    myFile.println(create_sensors_actuators_log_string(NO_ERR));
    stringQueue.push(create_sensors_actuators_log_string(NO_ERR));
    myFile.close();
  }
  else{
    //SDcard module failed
    stringQueue.push(create_sensors_actuators_log_string(SD_CONN_ERR));
    Serial.println("SDCard Failed");
    reset_needed = true;
  }
}

void online_command(char* command){
  char *token, *date_token, *time_token;
  String date, time;
  token = strtok(command, "_");
  int command_id = atoi(token);
  switch (command_id)
  {
  case ONLINE_MANUAL_CONTROL:
    automatic_control = false;
    writeControl(water_pump, LOW);
    writeControl(air_pump, HIGH);
    writeControl(valve1, LOW);
    writeControl(valve2, LOW);
    stringQueue.push("500_online,on\n");
    sendAllControlStatusesToEsp();
    break;
  case AUTOMATIC_CONTROL:
    automatic_control = true;
    writeControl(water_pump, LOW);
    writeControl(air_pump, HIGH);
    writeControl(valve1, LOW);
    writeControl(valve2, LOW);
    stringQueue.push("500_online,off\n");
    sendAllControlStatusesToEsp();
    break;
  case TIME_SET:
    date_token = strtok(NULL, ",");
    date = String(date_token);
    time_token = strtok(NULL, ",");
    time = String(time_token);
    setRealStartTime(F(__DATE__), F(__TIME__));
    myRTC.setDS1302Time(ss, mm, hh, dayofweek(d, m, yOff+2000), d, m, yOff+2000);
    break;
  case DO_THRESHOLD_SET:
    o2LowerBound = atoi(strtok(NULL,"_"));
    o2UpperBound = atoi(strtok(NULL,"_"));
    stringQueue.push(String(DO_THRESHOLD_SET_RESPONSE) + "_" + String(myRTC.year) + "/" + String(myRTC.month) + "/" + String(myRTC.dayofmonth)
            + " " + String(myRTC.hours) + ":" + String(myRTC.minutes) + ":"
            + String(myRTC.seconds) + "," + String(o2LowerBound)+ "," + String(o2UpperBound));
    break;
  case ACTUATORS_STATUS_REQUEST:
    sendAllControlStatusesToEsp();
    break;
  default:
    if(!automatic_control){
      int duration = atoi(strtok(NULL,"_"));
      switch (command_id)
      {
      case OPEN_WATER_PUMP:
        set_manual_water_time(duration);
        writeControl(water_pump, HIGH);
        break;
      case CLOSE_WATER_PUMP:
        writeControl(water_pump, LOW);
        break;
      case OPEN_AIR_PUMP:
        writeControl(air_pump, HIGH);
        break;
      case CLOSE_AIR_PUMP:
        writeControl(air_pump, LOW);
        break;
      case OPEN_VALVE1:
        writeControl(valve1, HIGH);
        break;
      case CLOSE_VALVE1:
        writeControl(valve1, LOW);
        break;
      case OPEN_VALVE2:
        writeControl(valve2, HIGH);
        break;
      case CLOSE_VALVE2:
        writeControl(valve2, LOW);
        break;
      
      default:
        Serial.print("unknown manual command:");
        Serial.println(command);
        break;
      }
    }
    else{
      Serial.print("unknown non-manual command:");
      Serial.println(command);
    }
    break;
  }
}

void set_manual_water_time(int seconds){
  manual_water_time = (seconds < max_watering_time) ? seconds : max_watering_time;
}

String create_log_file_name(){
  String yyyy, mm, dd;
  yyyy = String(myRTC.year);
  mm = (myRTC.month < 10) ? "0"+String(myRTC.month) : String(myRTC.month);
  dd = (myRTC.dayofmonth < 10) ? "0"+String(myRTC.dayofmonth) : String(myRTC.dayofmonth);
  return yyyy+mm+dd+".TXT";
}

String create_sensors_actuators_log_string(int statusFlag){
  return String(myRTC.year) + "/" + String(myRTC.month) + "/" + String(myRTC.dayofmonth)
            + " " + String(myRTC.hours) + ":" + String(myRTC.minutes) + ":"
            + String(myRTC.seconds) + "," + String(loop_O2) + "," + String(average_temp)
            + "," + String(loop_pH) + "," + String(loop_soilMoisture) + "," 
            + String(loop_ultra_tank) + "," + String(loop_ultra_fish) + ","
            + String(minute_control_flags[water_pump_index]) + "," + String(minute_control_flags[air_pump_index])
            + "," + String(minute_control_flags[valve1_index]) + "," + String(minute_control_flags[valve2_index])
            + "," + String(statusFlag);
}

void log_esp_communication_failed_into_SDcard(){
  File sysStatFile = SD.open(systemStatusFile, FILE_WRITE);
  if(sysStatFile){
    sysStatFile.println(create_esp_communication_failed_log_string());
  }
}

String create_esp_communication_failed_log_string(){
  return "Serial communication with ESP errored at"
            + String(myRTC.year) + "/" + String(myRTC.month) + "/" 
            + String(myRTC.dayofmonth) + " " + String(myRTC.hours) + ":" 
            + String(myRTC.minutes) + ":" + String(myRTC.seconds); 
}

void send_data_log_to_esp(int statusFlag){
  Serial1.println(create_sensors_actuators_log_string(statusFlag));
}

void dequeue_all_string_to_esp(){
  while(!stringQueue.isEmpty ())
    Serial1.println(stringQueue.pop());
}

void print_for_serial_monitor(){
  Serial.println(create_sensors_actuators_log_string(NO_ERR));
}

void writeControl(int ch, int output) {
  int index = 0;
  switch (ch) {
    case water_pump: index = 0; break;
    case air_pump: index  = 1; break;
    case valve1: index = 2; break;
    case valve2: index = 3; break;
  }
  if(control_flags[index] != output){
    control_flags[index] = output;
    sendControlStatusToEsp(ch, output);
  }
  digitalWrite(ch, output);
}

void sendControlStatusToEsp(int ch, int output){
  Serial1.print("500_");
  switch(ch){
    case water_pump: Serial1.print("water_pump"); break;
    case air_pump:   Serial1.print("air_pump");   break;
    case valve1:     Serial1.print("valve1");     break;
    case valve2:     Serial1.print("valve2");     break;
  }
  Serial1.println((output) ? ",on" : ",off");
}

void sendAllControlStatusesToEsp(){
  for(int i=0;i<controllable_num;i++){
    Serial1.print(CONTROL_STATUS_RESPONSE);
    switch(i){
      case water_pump_index: Serial1.print("_water_pump"); break;
      case air_pump_index:   Serial1.print("_air_pump");   break;
      case valve1_index:     Serial1.print("_valve1");     break;
      case valve2_index:     Serial1.print("_valve2");     break;
    }
    Serial1.println((control_flags[i]) ? ",on" : ",off");
  }
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
  return ( y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

void check_minute_control_flags() {
  for (int i = 0; i < controllable_num; i++) {
    if (control_flags[i] == HIGH) {
      minute_control_flags[i] = HIGH;
    }
  }
}

void reset_minute_control_flags() {
  for (int i = 0; i < controllable_num; i++) {
    minute_control_flags[i] = LOW;
  }
}

float average_float_array(float* arr, int arr_size) {
  float total = 0;
  for (int i = 0; i < arr_size; i++) {
    total += arr[i];
  }
  return total / arr_size;
}
