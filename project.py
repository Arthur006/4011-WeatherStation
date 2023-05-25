from threading import Thread
import serial
import serial.tools.list_ports
import struct
import time
import random
import tago

PREAMBLE = 170
iotFinished = True
tagioFinished = True

def main():

    global iotFinished
    global tagioFinished

    # Device for TAGO.IO
    device = tago.Device('5014f02e-7bc5-41e2-abd5-4384d6723525')

    # Connect to serial port
    portsData = list(serial.tools.list_ports.comports())
    for port in portsData:
        print("Port Found: " + str(port))

    portName = input("Enter Port Name: ")

    ser = None
    try:
        ser = serial.Serial(port=portName, baudrate=115200, timeout=0.3)
        print("Serial opened with " + portName)
    except:
        print("Error with port")
        return
    
    # Station Readings
    cr_dashboard = {}
    cr_dashboard["ST1"] = None
    cr_dashboard["ST2"] = None
    cr_iot = {}
    cr_iot["ST1"] = None
    cr_iot["ST2"] = None
      
    # Loop and get new weather station readings
    while (1):

        # Read data from serial
        data = ""
        try:
            while (1):
                newData = ser.readline()
                if (len(newData) > 0):
                    temp = list(newData)
                    if (PREAMBLE in temp):
                        data = newData
                else:
                    break
        except:
            print("Serial port connection error")
            ser.close()

        if (len(data) > 0):

            dataConverted = list(data)

            # Find index where data starts
            idx = 0
            for c in dataConverted:
                if (c == PREAMBLE):
                    break
                idx += 1

            receivedVals = dataConverted[idx:(idx + 22)]

            if (len(receivedVals) == 22):

                # Extract weather station values
                recvData = struct.unpack("<BBIIIII", bytes(receivedVals))
                readings = []
                stationId = int(recvData[1])
                for val in recvData[2:]:
                    readings.append(float(val / 1000.0))

                # Update current readings
                stationId = random.randint(1, 2)
                if (stationId == 1):
                    cr_dashboard["ST1"] = readings
                    cr_iot["ST1"] = readings

                elif (stationId == 2):
                    cr_dashboard["ST2"] = readings
                    cr_iot["ST2"] = readings

                # IoT blockchain send
                if (iotFinished):
                    iotFinished = False
                    Thread(target=iot_thread,
                            args=(cr_iot["ST1"], cr_iot["ST2"])).start()
                    cr_iot["ST1"] = None
                    cr_iot["ST2"] = None
                    
                # TagIO send
                if (tagioFinished):
                    tagioFinished = False
                    Thread(target=dashboard_thread,
                            args=(device, cr_dashboard["ST1"],
                            cr_dashboard["ST2"])).start()
                    cr_dashboard["ST1"] = None
                    cr_dashboard["ST2"] = None

def iot_thread(readings1, readings2):

    global iotFinished

    if (readings1 != None):
        # Iot send code
        print("Send 1")
        time.sleep(5)

    if (readings2 != None):
        # Iot send code
        print("Send 2")
        time.sleep(5)

    iotFinished = True

def dashboard_thread(device, readings1, readings2):

    global tagioFinished


    if (readings1 != None):
        data = {
            "variable": "ST1_Temperature",
            "unit"    : '°C',
            "value"   : readings1[0],
        }
        device.insert(data)
        data = {
            "variable": "ST1_Air_Pressure",
            "unit"    : 'kPa',
            "value"   : readings1[1],
        }
        device.insert(data)
        data = {
            "variable": "ST1_Humidity",
            "unit"    : 'RH %',
            "value"   : readings1[2],
        }
        device.insert(data)
        data = {
            "variable": "ST1_Air_Quality_CO2",
            "unit"    : 'ppb',
            "value"   : readings1[3],
        }
        device.insert(data)
        data = {
            "variable": "ST1_Air_Quality_TVOC",
            "unit"    : 'ppb',
            "value"   : readings1[4],
        }
        device.insert(data)

    if (readings2 != None):
        data = {
            "variable": "ST2_Temperature",
            "unit"    : '°C',
            "value"   : readings2[0],
        }
        device.insert(data)
        data = {
            "variable": "ST2_Air_Pressure",
            "unit"    : 'kPa',
            "value"   : readings2[1],
        }
        device.insert(data)
        data = {
            "variable": "ST2_Humidity",
            "unit"    : 'RH %',
            "value"   : readings2[2],
        }
        device.insert(data)
        data = {
            "variable": "ST2_Air_Quality_CO2",
            "unit"    : 'ppb',
            "value"   : readings2[3],
        }
        device.insert(data)
        data = {
            "variable": "ST2_Air_Quality_TVOC",
            "unit"    : 'ppb',
            "value"   : readings2[4],
        }
        device.insert(data)

    tagioFinished = True

# Start main
if __name__ == "__main__":
    main()