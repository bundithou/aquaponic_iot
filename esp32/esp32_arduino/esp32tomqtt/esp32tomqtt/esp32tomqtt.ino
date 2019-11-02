/*
run on esp

*/
#include <WiFi.h> //Wifi library
#include <PubSubClient.h>
//////// wifi config///////////////
const char* ssid = "Nopplee";
const char* password = "0123123156";

const char* mqtt_server = "35.198.234.67"; //<-- IP หรือ Domain ของ Server MQTT
long lastMsg = 0;
char msg[100];
int value = 0;
String DataString;

WiFiClient espClient;

// use for receive data from mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}
PubSubClient client(mqtt_server, 1883, callback, espClient);
void setup() {
  Serial.begin(115200); 
  Serial.println("setup");
  Serial2.begin(115200); 
  client.setCallback(callback);
  client.subscribe("command");
}

void loop() {
  String d = "";
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
   client.publish("aquaponic", bfc);
 }
 //send
 //Serial2.println("ack");
 //  delay(5000);
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
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("aquaponic", "hello world");
      // ... and resubscribe
      client.subscribe("command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
