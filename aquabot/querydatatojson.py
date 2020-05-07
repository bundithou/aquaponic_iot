import mysql.connector
import pandas as pd
import sqlalchemy as sql
from datetime import datetime, timedelta
import pytz
engine = sql.create_engine('mysql+mysqlconnector://bundit_hou:password@127.0.0.1:3306/testLog?use_pure=True')

df = pd.read_sql("select * from testLog.log order by id desc limit 1440", con = engine)
df = df.iloc[::-1]
print(type(df.iloc[0]['hrminsec']))
df['hrminsec'] = df['hrminsec'].astype(str).str.split().str[-1]
df.to_json(path_or_buf="/home/bundit_hou/Dashboard-master/src/records1.json",orient="records")
print("raw:",df.iloc[0])
