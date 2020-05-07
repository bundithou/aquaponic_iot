
import paho.mqtt.client as mqtt
import json
import mysql.connector
from datetime import datetime
import pytz
import requests

tz_BKK = pytz.timezone('Asia/Bangkok') 

host = "localhost"
port = 1883

prev = ""

def on_connect(self, client, userdata, rc):
  print("MQTT Connected.")
  self.subscribe("aquaponic/request")

def on_message(client, userdata,msg):
  
  text = msg.payload.decode("utf-8", "strict")
  print("receive: "+text)
  '''
  list of request message = ["connection failed", "water pump is not working"]
  "connection failed" 
      cause    : receive this message whenever the system's clock does not work properly.
      response : send the correct time back to the system to reset it's clock.
  
  "water pump is not working"
      cause    : receive this message whenever the water pump does not work.
      response : trigger the line bot to warn and ask for toubleshooting.
  '''
  
  if text == "connection failed":
      print(0)
      datetime_BKK = datetime.now(tz_BKK)
      str_time = datetime_BKK.strftime("300_%b %-2d %Y,%H:%M:%S")
      print(str_time)
      client.publish("command/timeSet", str_time)
  
  if text == "water pump is not working":
      print(1)
      re = requests.get("http://127.0.0.1:5000/waterpump")
      print(re)



client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(host)
client.loop_forever()
