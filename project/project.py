from threading import Thread
import serial
import serial.tools.list_ports
import struct
import time
from web3 import Web3

CONTRACT_ADDR = "0x3933826f6772e639CCf75786A4c9ADa2a9c2276F"
CONTRACT_KEY = "0x5a2c5f2d1c6d465b52e4a66ca107b05f76a60fffea7dbbaa2474eb4163ad220f"
CONTRACT_ABI = [
	{
		"inputs": [
			{
				"internalType": "string",
				"name": "_data",
				"type": "string"
			}
		],
		"name": "store",
		"outputs": [],
		"stateMutability": "nonpayable",
		"type": "function"
	},
	{
		"inputs": [],
		"name": "data",
		"outputs": [
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			}
		],
		"stateMutability": "view",
		"type": "function"
	},
	{
		"inputs": [],
		"name": "retrieve",
		"outputs": [
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			}
		],
		"stateMutability": "view",
		"type": "function"
	}
]
contract = w3.eth.contract(address=addr, abi=contract_abi)

PREAMBLE = 170
iotFinished = True

def main():

    global iotFinished

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
            print(receivedVals)

            if (len(receivedVals) == 22):

                # Extract weather station values
                recvData = struct.unpack("<BBIIIII", bytes(receivedVals))
                readings = []
                stationId = int(recvData[1])
                for val in recvData[2:]:
                    readings.append(float(val / 1000.0))

                t = bytes(receivedVals)
                print(int.from_bytes(t[2:6], byteorder='big'))

                # IoT blockchain send
                if (iotFinished):
                    iotFinished = False
                    Thread(target=iot_thread,
                            args=(stationId, readings)).start()

def iot_thread(stationId, readings):

    global iotFinished

    print("okay")
    print(stationId)
    print(readings)
    time.sleep(5)
    iotFinished = True

# Start main
if __name__ == "__main__":
    main()