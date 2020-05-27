# -*- coding: utf-8 -*-
"""
Created on Sat Sep 14 19:06:47 2019

@author: bundi
"""

"""
cronjob

"""

from datetime import datetime, timedelta
import mysql.connector
import pandas as pd
import sqlalchemy as sql
import app
from linebot.models import (
    MessageEvent, TextMessage, TextSendMessage, BeaconEvent, LocationSendMessage
)
import paho.mqtt.client as mqtt
import json
host = "localhost"
port = 1883
client = mqtt.Client()
client.connect(host)
line_bot_api = app.line_bot_api
engine = sql.create_engine('mysql+mysqlconnector://bundit_hou:password@127.0.0.1:3306/testLog?use_pure=True')

waterlevel_alertflag = False
datalossflag = False

def min_job():
    #df = pd.read_sql("select * from testLog.logtwo order by id desc limit 1440", con = engine)
    #df = df.iloc[::-1]
    #df['hrminsec'] = df['hrminsec'].astype(str).str.split().str[-1]
    #df.to_json(path_or_buf="/home/bundit_hou/Dashboard-master/src/records1.json",orient="records")
    global waterlevel_alertflag
    global datalossflag
    str_msg = ""
    df = pd.read_sql("select * from testLog.logtwo order by datetime desc limit 1", con = engine)
    #print("raw:",df.iloc[0])

    #datatime = datetime.combine(df.iloc[0]['yearmonthdate'], datetime.min.time()) + (df.iloc[0]['hrminsec'])
    datetime = df.iloc[0]['datetime']
    nowtime = (datetime.now())
    timediff = (nowtime + timedelta(hours=7)) - datatime
    if timediff > timedelta(minutes=5) and not datalossflag:
        str_msg = str_msg+" data lost more than 5 mins\n"
        datalossflag = True

    if timediff < timedelta(minutes=3) and datalossflag:
        datalossflag = False
        str_msg = str_msg + " got a new data and need to recovery the rest\n"
    
    if df.iloc[0]['ultrasonic_fish'] > 82 and not waterlevel_alertflag:
        str_msg = str_msg+" low water level at fish tank too low ( > 82 )\n"
        waterlevel_alertflag=True
        
    if df.iloc[0]['ultrasonic_fish'] < 30 and not waterlevel_alertflag:
        waterlevel_alertflag=True
        str_msg = str_msg+" water level at fish tank too high ( < 30 )\n"
        
    if (df.iloc[0]['ultrasonic_fish'] <= 78) and (df.iloc[0]['ultrasonic_fish'] >= 33):
        if waterlevel_alertflag:
            str_msg = str_msg+" water level at fish tank back to normal (33 >= v <= 78)\n"
            waterlevel_alertflag = False
        else:
            waterlevel_alertflag = False
    return str_msg

def week_job():
    return "Todolist:\n- change NaOH for O2 sensor\n- clean all sensors\n"
        
    
def now():
    df = pd.read_sql("select * from testLog.logtwo order by datetime desc limit 1", con = engine)
    return df.iloc[0].to_string()

def summary():
    # if datalossflag:
    #    return "data not enough"
    
    df = pd.read_sql("select * from testLog.logtwo order by datetime limit 1440", con = engine)
    current = df.head(1).iloc[0]
    last = df.tail(1).iloc[0]
    str_msg = "today: "+str(current['datetime'])+"\n" + "current water level: "+str(current['ultrasonic_fish']) +" cm.\n"
    str_msg = str_msg + "previous day: "+str(last['ultrasonic_fish'])+" cm.\n"
    str_msg = str_msg + "O2 min"+str(df['oxygen'].min())+" max"+str(df['oxygen'].max())+" avg"+str(df['oxygen'].mean())+"\n"
    str_msg = str_msg + "temp min"+str(df['temperature'].min())+" max"+str(df['temperature'].max())+" avg"+str(df['temperature'].mean())+"\n"
    str_msg = str_msg + "soil min"+str(df['soil_moisture'].min())+" max"+str(df['soil_moisture'].max())+" avg"+str(df['soil_moisture'].mean())+"\n"
    str_msg = str_msg + "pH min"+str(df['pH'].min())+" max"+str(df['pH'].max())+" avg"+str(df['pH'].mean())+"\n"
    # df = pd.read_sql("select count(*) as count from testLog.logtwo where yearmonthdate = "+str(last['yearmonthdate']), con = engine)
    #if df.iloc[0]['count'] < 1440:
    #    client.publish("command/requestData", str(last['datetime']))
    return str(str_msg)

def report():
    df = pd.read_sql("select * from testLog.logtwo order by datetime desc limit 1440", con = engine)
    sensors = ['oxygen', 'temperature', 'soil_moisture', 'pH']
    report = pd.DataFrame(columns=['min','max','avg'], index=sensors)
    for s in sensors:
        report.loc[s]['min'] = df[s].min()
        report.loc[s]['max'] = df[s].max()
        report.loc[s]['avg'] = df[s].mean()
    return report.to_string()


def commandtomqtt( msg):
    client.publish("command/onlineCommand", msg)
    return None
