/*
Main ESP(Nano 32) code to run on the real system.
*/
#include <WiFi.h> //Wifi library
#include <PubSubClient.h>
//////// wifi config///////////////
bool use_mqtt_secondary_server = false;
long lastMsg = 0;
char msg[100];
int value = 0;
String DataString;

const char* data_topic = "aquaponic"; //out
const char* data_request_topic = "command/requestData"; //in
const char* time_set_request_topic = "aquaponic/request"; //out
const char* time_set_topic = "command/timeSet"; //in
const char* online_command_topic = "command/onlineCommand";//in
const char* command_reply_topic = "aquaponic/commandReply";//out

unsigned long last_milli;
unsigned long timeDiff = 0;
unsigned int dataCount = 0;

WiFiClient espClient;

// use for receive data from mqtt
void callback(char* topic, byte* payload, unsigned int len) {

  char payloadCStr[len + 1];

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
    payloadCStr[i] = (char)payload[i];
  }
  Serial.println();
  payloadCStr[len] = '\0';

  String topicStr(topic);
  //String payloadStr(payloadCStr);
  if(topicStr == data_request_topic){
    //tokenize the String
    char *token;
    token = strtok(payloadCStr,"-");
    
    String yyyy = token;
    token = strtok(NULL,"-");
    String mm = token;
    token = strtok(NULL,"-");
    String dd = token;
    //send yyyymmdd to arduino
    Serial.println("sending:"+yyyy+mm+dd);
    Serial2.println(yyyy+mm+dd);
  }
  else if(topicStr == time_set_topic) {
    char *token;
    token = strtok(payloadCStr,",");
    String date = token;
    token = strtok(NULL,",");
    String time = token;
    Serial.println("sending:"+date+","+time);
    Serial2.println(date+","+time);
  }
  else if(topicStr == online_command_topic){
    Serial.println("online command:");
    Serial.println(payloadCStr);
    Serial2.println(payloadCStr);
  }
}

PubSubClient client(mqtt_server, 1883, callback, espClient);

void setup() {
  last_milli = millis();
  Serial.begin(115200);
  Serial.println("setup");
//  Serial.println("20secs delay");
//  delay(20000);
  Serial.println("starting serial2");
  Serial2.begin(115200); 
  setup_wifi();
  client.setCallback(callback);
  client.subscribe(data_request_topic);
  client.subscribe(time_set_topic);
  client.subscribe(online_command_topic);
}

void loop() {
    // String d = "";
    if (!client.connected()) {
      reconnect();
    }
    client.loop(); 
    // read
    if (Serial2.available() > 0) {
  
      //Serial.println("receive: ");
      char bfr[501];
      memset(bfr,0, 501);
      Serial2.readBytesUntil( '\n',bfr,500);
      Serial.println(bfr);

      char *token;
      token = strtok(bfr, "_");

      if(strlen(token) == 3){
        if(atoi(token) == 500){
          token = strtok(NULL, "\0");
          client.publish(command_reply_topic, token);
        }
        else if(atoi(token) == 600){
          token = strtok(NULL, "\0");
          client.publish(time_set_request_topic, token);
        }
      }
      else{
        // sent to mqtt
        client.publish(data_topic, bfr);
      }


      dataCount++;
    }

   unsigned long current = millis();
   //Serial.println((unsigned long)(current - last_milli));
   if((unsigned long)(current - last_milli) >= 60000){
     Serial.println("a minute passed");
     last_milli = current;
     if(/*dataCount > 10 || */dataCount < 1){
      dataCount = 0;
      /*Serial.println("Restarting ESP32");
      delay(1000);
      ESP.restart();*/
      /*Serial2.end();
      delay(1000);
      Serial2.begin(115200);*/
      client.publish(time_set_request_topic, "connection failed");
     }
     dataCount = 0;
   }

 }

 void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// Procedure reconnect ใช้กรณีที่บางทีเราหลุดออกจาก Network แล้วมีการต่อเข้าไปใหม่อีกครั้ง
void reconnect() {
  // Loop until we're reconnected
  unsigned int counter = 0;
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("AquaponicNano32")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(data_topic, "reconnected");
      // ... and resubscribe
      client.subscribe(data_request_topic);
      client.subscribe(time_set_topic);
      client.subscribe(online_command_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      if(++counter > 2){
        Serial.println(" try again in 5 seconds with another server");
        use_mqtt_secondary_server = !use_mqtt_secondary_server;
        Serial.println("Trying "+(use_mqtt_secondary_server) ? "secondary server" : "main server");
      }
      // Wait 5 seconds before retrying
      delay(5000);
      client.setServer((use_mqtt_secondary_server) ? mqtt_secondary_server : mqtt_server , 1883);
    }
  }
}
