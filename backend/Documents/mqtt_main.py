import paho.mqtt.client as mqtt
import json
import mysql.connector
from datetime import datetime

host = "localhost"
port = 1883

prev = ""

def on_connect(self, client, userdata, rc):
  print("MQTT Connected.")
  self.subscribe("aquaponic")

def on_message(client, userdata,msg):
  # put the data to database if package is json
  text = msg.payload.decode("utf-8", "strict")
  print("receive: "+text)

  global prev
  print(prev)
  data = text[:-1].split(',')
  print(data)

  if data[0] != prev:
    
    # data dict
    # 2019/11/18,19:4:0,3.60,33.82,16.44,89.54,11.00,39.00,0,1,0,0,0
    print("inserting data")
    prev = data[0]
    
    # connect to database
    cnx = mysql.connector.connect(user='nopparutlee', password='MAXIMCoffee',
					host='127.0.0.1', database='testLog', use_pure=True)
    cursor = cnx.cursor()
    print(data)

    # sql query script to insert data
    add_data = ("INSERT INTO testLog.logtwo " \
		"VALUES (NULL, \'"+data[0]+"\',\'"+data[1]+"\',"+data[2]+","+data[3]+ \
		","+data[4]+","+data[5]+","+data[6]+","+data[7]+","+data[8]+ \
		","+data[9]+","+data[10]+","+data[11]+")")

    # execute query
    cursor.execute(add_data)
    cnx.commit()

    # disconnect
    cursor.close()
    cnx.close()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(host)
client.loop_forever()
