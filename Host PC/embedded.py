'''
Main Python program to run on windows and/or linux OS to read data
from ZigBee coordiantor and either:
    1) Collect training data for floor location (classification) or mass (regression)
        machine learning model, or
    2) Send x-y positions, adc counts, mass and floor location to sql database
If training please provide the following arguments in the CLI:
    --training 
    --floor-location or --mass
    -f ['training file']
'''

# Import libraries 
import serial
import time
import numpy as np
from sklearn.linear_model import LinearRegression
from sklearn.neighbors import KNeighborsClassifier
import sys
import pymssql as sql
import pandas as pd

# Global constants
SERVER = ""
USER = ""
PASSWORD = ""
DATABASE = ""

REGRESSION = 1
CLASSIFICATION = 2

CRANE_ID = 0
POS_X = 1
POS_Y = 2
ADC = 3
MASS = 4

MASS_TRAINING_DATA = 'TrainingData/mass-training.csv'
FLOOR_LOCN_TRAINING_DATA = ''

COM_PORT = 'COM14'
BAUDRATE = 38400

'''
MLModels: Class representing the different machine learning models to model both
the mass on the crane hook and the floor location 
'''
class MLModels(object):

    '''
    Constructor for the MLModels class
    '''
    def __init__(self, *args, **kwargs):
        self.linear_regression = LinearRegression()
        self.knn = KNeighborsClassifier()
    
    '''
    Train the given model type based on the data in the given file
    Parameters:
        type (int): 1 for regression model, 0 if classification
        file (str): training data file of csv format
    Returns:
        ML model fitted with training data
    '''
    def train_model(self, type, file):
        
        df = pd.read_csv(file)  # Read the data file

        if type == REGRESSION: # Train the regression model
            X_train = df.iloc[:, 0]
            y_train = df.iloc[:, -1]

            self.linear_regression.fit(X_train.to_numpy().reshape(-1, 1), y_train)
        elif type ==CLASSIFICATION: # Train the classification model
            X_train = df.iloc[:, 0:-1]
            y_train = df.iloc[:, -1]

            self.knn.fit(X_train, y_train)
    
    '''
    Get the mass on the crane hook from the given ADC count
    Parameter:
        adc: ADC count from crane hook
    Return:
        Mass prediction
    '''
    def get_mass(self, adc):
        return self.linear_regression.predict(np.array([int(adc)]).reshape(-1, 1))

    '''
    Get the floor location from the current x, y coordinate
    Parameters:
        posX: x position
        posY: y position
    Return:
        Floor location prediction
    '''
    def get_floor_location(self, posX, posY):
        return self.knn.predict(np.array([int[posX], int(posY)]))    
    
    '''
    Append the collected training data to the correct file name
    Parameters:
        fileName: training data file of type .csv
        dataList: data to be appended to csv file
        trainingType: 1 if regression, 0 if classification
        trainingVar: output of ml model (i.e. floor location or mass)
    '''
    def append_training_data(self, fileName, dataList, trainingType, trainingVar):
        if trainingType == REGRESSION:  # Append regression data to fileName
            df = pd.DataFrame({
                'ADC': [int(dataList[ADC])],
                'Mass': [float(trainingVar)]
            })                    
            with open(fileName, 'a') as f:
                df.to_csv(f, header=f.tell()==0, index=False)
        elif trainingType == CLASSIFICATION:    # Append classification data to fileName
            df = pd.DataFrame({
                'Pos X': [int(dataList[POS_X])],
                'Pos Y': [int(dataList[POS_Y])],
                'Floor Locn': [int(trainingVar)]
            })                    
            with open(fileName, 'a') as f:
                df.to_csv(f, header=f.tell()==0, index=False)

'''
SqlDatabase: Class representing the SQL database to store the relevant database
'''
class SqlDatabase(object):

    '''
    Constructor for SqlDatabase class
    '''
    def __init__(self, *args, **kwargs):

        # Try connecting to SQL database
        try:
            self.con = sql.connect(SERVER, USER, PASSWORD, DATABASE, autocommit=True)
            self.cur = self.con.cursor(as_dict=True)
        except sql.OperationalError:
            print("Error Connecting to SQL Database")
    
    '''
    Send the given data to the SQL database
    Parameters:
        dataList: data to send to database
    '''
    def send_to_database(self, dataList):

        # Try to send data to SQL database
        try:
            self.cur.execute("INSERT INTO dbo.positions (crane_id, update_time, x, y, adc, weight) VALUES (%s, getdate(), %s, %s, %s, %s)" %
                        (dataList[CRANE_ID], dataList[POS_X], dataList[POS_Y], dataList[ADC], dataList[MASS]))
        except Exception as e:
            print(e)
            time.sleep(1)

'''
SerialReader: Class representing the serial communication between the Turtle board and the host PC
'''
class SerialReader(object):

    '''
    Constructor for SerialReader class
    '''
    def __init__(self, *args, **kwargs):

        # Attempt to open serial port
        try:
            self.ser = serial.Serial(COM_PORT, baudrate=BAUDRATE, timeout=1)
        except Exception as e:
            print(e)
            return
    
    '''
    Read data from the serial port until the required data has been parsed correctly
    Returns:
        dataList: [crane ID, x position, y position, adc count]
    '''
    def get_data(self):
        
        # Initialise variables
        readFlag = False
        rxBuffer = ""
        craneID = 0
        rawAdc = 0
        posX = 0
        posY = 0

        # Enter loop
        while True:
            
            # Try to read serial port
            try:
                c = self.ser.read().decode('utf-8')
            except serial.SerialException:
                continue
            except UnicodeDecodeError:
                continue
            
            # Check for new line character
            if c == '\n':
                readFlag = True 
            elif c == '\r':
                # Ignore, prevent returning early
                continue
            else:
                rxBuffer += c # Add received char to buffer
                continue
            
            # Check if message is ready to be parsed
            if readFlag == True:
                readFlag = False
                
                dataList = rxBuffer.split(" ")
                if (len(dataList) < 4):
                    for i in range(len(dataList)):
                        if (dataList[i])[0] == 'i':
                            craneID = (dataList[i])[1::]
                        elif (dataList[i])[0] == 'k':
                            posX = -1
                            posY = -1

                # Extract variables from message
                for i in range(len(dataList)):
                    # if (dataList[i])[0] == '\x16':
                    #     craneID = (dataList[i])[2:3:]
                    if (dataList[i])[0] == 'm':
                        rawAdc = (dataList[i])[1::]
                    elif (dataList[i])[0] == 'x':
                        posX = (dataList[i])[1::]
                    elif (dataList[i])[0] == 'y':
                        posY = (dataList[i])[1::]
                    else:
                        for j in range(len(dataList[i])):
                            if dataList[i][j] == 'i':
                                craneID = (dataList[i])[j + 1]
                                pass
                        continue

                rxBuffer = ""
                return [craneID, posX, posY, rawAdc]    # Return data

'''
Main loop for controlling flow of program
'''
def main():
    
    # Set class objects
    serialReader = SerialReader()
    sqlDatabase = SqlDatabase()
    mlModel = MLModels()

    # Define local variables
    trainingFlag = False
    trainingVariable = ''
    fileName = ''
    trainingType = 0

    # Get system arguments
    for i in range(len(sys.argv)):
        if sys.argv[i] == '--training':
            trainingFlag = True
        elif sys.argv[i] == '-f':
            fileName = sys.argv[i + 1]
            if '.csv' not in fileName:
                print('Please use an appropriate .csv file')
                return
        elif sys.argv[i] == '--floor-location':
            trainingType = CLASSIFICATION
            trainingVariable = sys.argv[i + 1]
        elif sys.argv[i] == '--mass':
            trainingType = REGRESSION
            trainingVariable = sys.argv[i + 1]
    
    # Check for errors
    if trainingFlag == True and (trainingVariable == '' or trainingType == 0 or fileName == ''):
        print('If training please provide a training variable and training type'
              'with --floor-location or --mass and appropriate file name with -f')
        return
    
    # Train models
    mlModel.train_model(REGRESSION, file=MASS_TRAINING_DATA)

    # Enter cyclic executive
    while True:
        dataList = serialReader.get_data()
        dataList.append(mlModel.get_mass(dataList[ADC])[0])
        print(dataList)

        if trainingFlag == True:
            mlModel.append_training_data(fileName, dataList, trainingType, trainingVariable)    #append to training file
        else:
            sqlDatabase.send_to_database(dataList)  # Send to database

# Run the main program
if __name__ == "__main__":
    main()
