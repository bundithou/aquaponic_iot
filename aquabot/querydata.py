\
import mysql.connector
import pandas as pd
import sqlalchemy as sql
from datetime import datetime, timedelta
import pytz
engine = sql.create_engine('mysql+mysqlconnector://bundit_hou:password@127.0.0.1:3306/testLog?use_pure=True')

df = pd.read_sql("select *  from testLog.log where yearmonthdate = '2019-12-15'", con = engine)
print("raw:",df.iloc[0])
#print("head:", df.head(1).iloc[0])
#print("last:", df.tail(1))
#print("date:", df.iloc[0]['yearmonthdate'])
#print("time:", df.iloc[0]['hrminsec'])
#print("date type:", type(df.iloc[0]['yearmonthdate']))
#print("time type:", type(df.iloc[0]['hrminsec']))

#datatime = datetime.combine(df.iloc[0]['yearmonthdate'], datetime.min.time()) + (df.iloc[0]['hrminsec'])
#bkktimezone = pytz.timezone("Asia/Bangkok")
#nowtime = (datetime.now())

#print(datatime.replace(tzinfo=bkktimezone))
#print("time diff, ", (nowtime + timedelta(hours=7)) - datatime)
#print(datatime - nowtime.replace(tzinfo=None))
