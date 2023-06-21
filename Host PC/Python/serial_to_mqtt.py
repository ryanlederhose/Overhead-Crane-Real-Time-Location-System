'''
This python script will read data over the specified serial port
at baudrate and parse the data to store into an excel spreadsheet
    @param -b baudrate
    @param -p com port
    @param -t mqtt topic
'''

import serial
from datetime import datetime
import time as tick
import paho.mqtt.client as mqtt
import json
import sys
import numpy as np
from sklearn.linear_model import LinearRegression

'''
    @brief entry point into program
'''
def main():

    global ser
    global client
    global topic

    com_port = ""
    baudrate = 0

    for i in range(len(sys.argv)):
        if sys.argv[i] == '-b':
            baudrate = int(sys.argv[i + 1])
        elif sys.argv[i] == '-p':
            com_port = sys.argv[i + 1]
        elif sys.argv[i] == '-t':
            topic = sys.argv[i + 1]

    #Open serial port
    try:
        ser = serial.Serial(com_port, baudrate=baudrate, timeout=1)
        print("Serial port has been detected")
    except serial.SerialException:
        print("COULD NOT OPEN SERIAL PORT")
        return -1
    
    #Initialise mqtt client
    client = mqtt.Client(client_id="CRANE_3",
                            transport="tcp")
    
    #Set password for mqtt
    client.username_pw_set(username="", password="")

    #Set callbacks for connect and publish
    client.on_connect = on_connect
    client.on_publish = on_publish

    #Connect to mqtt broker
    client.connect(host="", port = 1883, keepalive=10)

    #Start client loop 
    client.loop_start()
    tick.sleep(4)
    print("Client Connection Status:", client.is_connected())

    read_serial()   #start loop

''' 
    This function is used to read data over the serial port
    and parse this data so it is easily sent and deciphered
    over mqtt
'''
def read_serial():

    #Initialise variables
    readFlag = False
    rxBuffer = ""
    data = ""
    tickCount = tick.time()
    global ser
    global client
    count = 1
    readyToSend = False
    craneID = 0
    waitingFlag = 0
    rawAdc = 0
    weightList = []

    jsonList1 = []
    jsonList2 = []
    jsonList3 = []

    # create example data
    x = np.array([670, 1282, 1639, 1884, 2077, 2229, 2370, 3808]).reshape((-1, 1))
    y = np.array([0, 1.488, 2.466, 2.97, 3.168, 3.568, 3.762, 6.9])

    # create linear regression object
    model = LinearRegression()

    # fit the model to the data
    model.fit(x, y)

    #Enter loop
    while True:

        #Try to read serial port
        try:
            c = ser.read().decode('utf-8')
        except serial.SerialException:
            continue
        except UnicodeDecodeError:
            continue
        
        #check for new line character
        if c == '\n':
            readFlag = True 
        elif c == '\r':
            #ignore, prevent returning early
            continue
        else:
            rxBuffer += c #add received char to buffer
            continue
        
        #Check if message is ready to be parsed
        if readFlag == True:
            readFlag = False
            now = datetime.now()    #get time
            
            dataList = rxBuffer.split(" ")
            if (len(dataList) < 4):
                for i in range(len(dataList)):
                    if (dataList[i])[0] == 'i':
                        craneID = (dataList[i])[1::]
                    elif (dataList[i])[0] == 'k':
                        posX = -1
                        posY = -1
                        waitingFlag = 1
                        continue
                if (waitingFlag == 0):                        
                    continue    #incomplete packet, ignore

            #extract variables from message
            for i in range(len(dataList)):
                if (dataList[i])[0] == 'i':
                    craneID = (dataList[i])[1::]
                elif (dataList[i])[0] == 'm':
                    rawAdc = (dataList[i])[1::]
                elif (dataList[i])[0] == 'x':
                    posX = (dataList[i])[1::]
                elif (dataList[i])[0] == 'y':
                    posY = (dataList[i])[1::]
                else:
                    continue
            
            #predict the mass of the coil based on the adc reading
            trueMass = model.predict(np.array([int(rawAdc)]).reshape(-1, 1))

            #put the calculated mass into a buffer of latest fifteen values
            weightList.insert(0, float(trueMass))
            if (len(weightList) > 15):
                weightList.pop(len(weightList) - 1)

            #get the average of the mass
            averageTrueMass = 0
            for i in range(len(weightList)):
                averageTrueMass += weightList[i]
            averageTrueMass = float(averageTrueMass / len(weightList))

            #set dictionary with updated variables
            updateData = {
                "a": int(rawAdc),
                "m": float(averageTrueMass),
                "x": int(posX),
                "y": int(posY),
                "t": str(now)
            }

            #check crane id and send appropriate data
            if craneID == "3":
                jsonList1.append(updateData)
                if len(jsonList1) == 10:
                    send_data(craneID="3", jsonList=jsonList1)
                    count = 1
                    jsonList1.clear()

            #increment count and reset buffer
            count += 1
            rxBuffer = ""

'''
    @brief format the time variable as a string
    @param updateData updated data from serial
    @return updateData
'''
def _format_time_as_string(updateData):
    updateData["t"] = str(updateData["t"])
    return updateData

'''
    @brief callback for mqtt connection attempt
    @param client mqtt client
    @param userdata user data of mqtt
    @param flags response flags
    @param rc return code
'''
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected OK, Returned code=", rc)
    else:
        print("Bad connection, Returned code=", rc)

'''
    @brief callback for successful mqtt publish
    @param client mqtt client
    @param userdata user data of mqtt
    @param mid message id
'''
def on_publish(client, userdata, mid):
    print("\r\nMessage Published\r\n")
    pass

'''
    @brief send the json package over mqtt
    @param craneID id of crane data
    @param jsonList json package containing data
'''
def send_data(craneID, jsonList):
    global topic

    jsonList = [_format_time_as_string(x) for x in jsonList]
    jsonPackage = json.dumps({
                        "id": int(craneID),
                        "updates": jsonList})
    client.publish(topic, jsonPackage)

# run application
if __name__ == "__main__":
    main()
