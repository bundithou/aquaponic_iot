import mysql.connector
from datetime import date,time
cnx = mysql.connector.connect(user='bundit_hou', password='password',
                              host='127.0.0.1',
                              database='testLog')

cursor = cnx.cursor()

add_data = ("INSERT INTO log "
               "VALUES (NULL, "+str(date(2019,1,1))+", "+str(time(1,1,1))+", 1.00, 1.00, 1.00, 1.00, 1.00, FALSE, FALSE, FALSE, FALSE, 0)")

cursor.execute(add_data)

cnx.commit()
cursor.close()
cnx.close()
