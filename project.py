from threading import Thread
import serial
import serial.tools.list_ports
import struct
import time
import random
import tago
from web3 import Web3

w3 = Web3(Web3.HTTPProvider('http://localhost:7545'))

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
            # print(dataConverted)

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
                    if (val == recvData[-1]):
                        readings.append(float((val / 1000.0) - 1))
                    else:
                        readings.append(float(val / 1000.0))

                readings[2] = readings[2] * 100

                # Update current readings
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


addr = "0x8CA3bEaf4DA08e65A72fF02A4B2BCDF2b85EcA88"
private_key = "0xa378967cb38bd7140578a42162329d8157a167eaedd0917724ab318a76bc9504"

contract_abi = [
    {
        "inputs": [
            {
                "internalType": "string",
                "name": "sensorID",
                "type": "string"
            }
        ],
        "name": "retrieve",
        "outputs": [
            {
                "components": [
                    {
                        "internalType": "uint256",
                        "name": "timestamp",
                        "type": "uint256"
                    },
                    {
                        "internalType": "uint256",
                        "name": "temp",
                        "type": "uint256"
                    },
                    {
                        "internalType": "uint256",
                        "name": "humidity",
                        "type": "uint256"
                    },
                    {
                        "internalType": "uint256",
                        "name": "pressure",
                        "type": "uint256"
                    },
                    {
                        "internalType": "uint256",
                        "name": "tvoc",
                        "type": "uint256"
                    },
                    {
                        "internalType": "uint256",
                        "name": "c02",
                        "type": "uint256"
                    }
                ],
                "internalType": "struct IoTData.WeatherData[]",
                "name": "",
                "type": "tuple[]"
            }
        ],
        "stateMutability": "view",
        "type": "function"
    },
    {
        "inputs": [
            {
                "internalType": "string",
                "name": "sensorID",
                "type": "string"
            },
            {
                "internalType": "uint256",
                "name": "temp",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "humidity",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "pressure",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "tvoc",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "c02",
                "type": "uint256"
            }
        ],
        "name": "store",
        "outputs": [],
        "stateMutability": "nonpayable",
        "type": "function"
    },
    {
        "inputs": [
            {
                "internalType": "string",
                "name": "",
                "type": "string"
            },
            {
                "internalType": "uint256",
                "name": "",
                "type": "uint256"
            }
        ],
        "name": "weatherData",
        "outputs": [
            {
                "internalType": "uint256",
                "name": "timestamp",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "temp",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "humidity",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "pressure",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "tvoc",
                "type": "uint256"
            },
            {
                "internalType": "uint256",
                "name": "c02",
                "type": "uint256"
            }
        ],
        "stateMutability": "view",
        "type": "function"
    }
]

contract = w3.eth.contract(address=addr, abi=contract_abi)

function_name = "store"

def iot_thread(readings1, readings2):

    global iotFinished

    account = w3.eth.account.from_key(private_key)
    sender_addr = account.address

    nonce = w3.eth.get_transaction_count(sender_addr)

    if (readings1 != None):
        # Iot send code
        print("Send 1")

        transaction = contract.functions.store("ST1", int(readings1[0]), int(readings1[2]), int(readings1[1]), int(readings1[3]), int(readings1[4])).build_transaction({
            'from': sender_addr,
            'nonce': nonce,
        })

        signed_txn = w3.eth.account.sign_transaction(transaction, private_key)

        transaction_hash = w3.eth.send_raw_transaction(signed_txn.rawTransaction)

    if (readings2 != None):
        # Iot send code
        print("Send 2")
        transaction = contract.functions.store("ST2", int(readings2[0]), int(readings2[2]), int(readings2[1]), int(readings2[3]), int(readings2[4])).build_transaction({
            'from': sender_addr,
            'nonce': nonce,
        })

        signed_txn = w3.eth.account.sign_transaction(transaction, private_key)

        transaction_hash = w3.eth.send_raw_transaction(signed_txn.rawTransaction)

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
