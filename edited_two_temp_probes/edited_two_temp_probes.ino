

// Including the ESP8266 WiFi library
#include <ESP8266WiFi.h>        //bibrary ที่ต้อง Include จาก การ Update ผ่าน Internet (Key Ctrl+shift+i) หรือ add File
#include <OneWire.h>            //bibrary ที่ต้อง Include จาก การ Update ผ่าน Internet (Key Ctrl+shift+i) หรือ add File
#include <DallasTemperature.h>  //bibrary ที่ต้อง Include จาก การ Update ผ่าน Internet (Key Ctrl+shift+i) หรือ add File
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <IoTtweet.h>

#define DHTTYPE DHT11           // DHT 11

// IoTtweet
const char *userid = "006293";
const char *key = "uyqcf9a41kom";

IoTtweet myiot;

// Replace with your network details
//const char* ssid = "3BB_Wifi";     // ชื่อ Wifi Network ที่ใช้ในการเชื่อมต่อ 
//const char* password = "0907327727";   // รหัสสำหรับลงชื่อเข้าใช้งาน Wifi Network
const char* ssid = "Nopplee";     // ชื่อ Wifi Network ที่ใช้ในการเชื่อมต่อ 
const char* password = "0123123156";   // รหัสสำหรับลงชื่อเข้าใช้งาน Wifi Network

// Data wire is plugged into pin D1 on the ESP8266 12-E - GPIO 5
#define ONE_WIRE_BUS D3

#define DHTPIN D3
DHT_Unified dht(DHTPIN, DHTTYPE);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature DS18B20(&oneWire);
char temperatureCString[2][7];
char temperatureFString[2][7];

// Web Server on port 80
WiFiServer server(80);

// Global varible for temperature from first sensor
float tempC[2];
float tempF[2];

// Device address of the temperature probes
DeviceAddress sensor1 = {0x28, 0xFF, 0x23, 0xE1, 0x24, 0x17, 0x03, 0x52};
DeviceAddress sensor2 = {0x28, 0xF4, 0x12, 0x43, 0x98, 0x08, 0x00, 0x5C};

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);

  DS18B20.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  //dht.begin();

  //version check for IoTtweet
  String libvers = myiot.getVersion();
  Serial.println("IoTtweet Library version : " + String(libvers));
  
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(100);  //Delay time waiting connect to Wifi (100 ms.)

  
  
  // Printing the ESP IP address
  Serial.print("Controller is IP : ");
  Serial.println(WiFi.localIP());

  
  //IoTtweet wifi connection
  bool conn = myiot.begin(ssid,password);
  if(!conn){
    Serial.println("WiFi connection failed.");
  }
  else{
    Serial.println("WiFi connected !");
  }
}

void getTemperature() {
  //float tempC;
  //float tempF;
  do {
    DS18B20.requestTemperatures(); 
    //tempC = DS18B20.getTempCByIndex(0);
    tempC[0] = DS18B20.getTempC(sensor1);
    dtostrf(tempC[0], 2, 2, temperatureCString[0]);
    //tempF = DS18B20.getTempFByIndex(0);
    tempF[0] = DS18B20.getTempF(sensor1);
    dtostrf(tempF[0], 3, 2, temperatureFString[0]);

    tempC[1] = DS18B20.getTempC(sensor2);
    dtostrf(tempC[1], 2, 2, temperatureCString[1]);
    tempF[1] = DS18B20.getTempF(sensor2);
    dtostrf(tempF[1], 3, 2, temperatureFString[1]);
    delay(100);
  } while (tempC[0] == 85.0 || tempC[0] == (-127.0));    // Read Datasheet DS18B20 Temperature Support rang.
}

// runs over and over again
void loop() {
  // Listenning for new clients
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
          Serial.println("requesting temp");
          getTemperature();        
          Serial.println("temp get");
          sensors_event_t event;  
          
         
        
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // your actual web page that displays temperature
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head></head><body><h1>ESP8266 - Temperature</h1><h3>Temperature in Celsius(1): ");
            client.println(temperatureCString[0]);
            client.println("*C</h3><h3>Temperature in Fahrenheit(1): ");
            client.println(temperatureFString[0]);
            client.println("*F</h3><h3>Temperature in Celsius(2): ");
            client.println(temperatureCString[1]);
            client.println("*C</h3><h3>Temperature in Fahrenheit(2): ");
            client.println(temperatureFString[1]);
            client.println("*F</h3></body></html>");
            Serial.print(temperatureCString[0]);
            Serial.print(" ");
            Serial.print(temperatureFString[0]);
            Serial.print(" ");
            //dht.humidity().getEvent(&event);
            //Serial.print(event.relative_humidity);
            //Serial.print("%  ");
            //Serial.print(" ");
            //dht.temperature().getEvent(&event);
            Serial.print(event.temperature);
            Serial.println("C  ");

            //sending data to IoTtweet
            String response = myiot.WriteDashboard(userid,key,tempF[0],tempC[0],event.temperature,0,"","");
            Serial.println(response);
           delay(100);  //delay time send Data 5 (s).       
          
     }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}   
