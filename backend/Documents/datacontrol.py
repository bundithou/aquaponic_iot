import sys
from datetime import datetime
import pytz
import mysql.connector
import pandas as pd
import sqlalchemy as sql
import paho.mqtt.client as mqtt
import json
host = "localhost"
port = 1883
client = mqtt.Client()
client.connect(host)
engine = sql.create_engine('mysql+mysqlconnector://bundit_hou:password@127.0.0.1:3306/testLog?use_pure=True')

def validate(date_text):
    try:
        datetime.strptime(date_text, '%Y-%m-%d')
    except ValueError:
        raise ValueError("Incorrect data format, should be YYYY-MM-DD")

def countrecords(date: str):
    validate(date)
    return pd.read_sql("select count(*) as count from testLog.logtwo where date(datetime) = \'"+date+"\'", con = engine)

def listallrecords():
    df = pd.read_sql("SELECT DISTINCT CAST(datetime AS DATE) date FROM testLog.logtwo", con = engine)
    ls_date = df['date']
    ls_records = []
    for date in ls_date:
        date = str(date)
        df = countrecords( date )
        print(date, df.iloc[0]['count'])
        ls_records.append((date, df.iloc[0]['count']))
    
    return ls_records

def requestdata(date: str = None):
    client.publish("command/requestData", date)

def requestallmissing():
    ls_records = listallrecords()
    for date, count in ls_records:
        if count < 1440:
            # ticket:
            #   - delay time after send request for 2 min 
            # fix: None
            requestdata(date)
            print(date, count)

def counttoday():
    date_str = datetime.now().strftime("%Y-%m-%d")
    df = countrecords(date_str)
    return date_str+", "+str(df.iloc[0]['count'])

def countall():
    count_str = 'counted by date\n'
    for date, count in listallrecords():
        count_str = count_str + str(date) + ", " + str(count) + "\n"

    return count_str

if __name__ == "__main__":
    # commands:
    #   -list           : Display date and record quantity 
    #   -request [date] : (format YYYY-MM-DD) Request data by given date and sync with database
    #   [No argv]       : Request all missing data by date and sync with database
    
    ls_arg = sys.argv

    if (len(ls_arg) == 1) and (ls_arg[0] == '-list'):
        listallrecords()

    if (len(ls_arg) == 2) and (ls_arg[0] == '-request'):
        validate(ls_arg[1])
        requestdata(ls_arg[1])

    if (len(ls_arg) == 0):
        requestallmissing()