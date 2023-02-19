import serial as pyserial
import json
import os
from datastructs import *


serial_port = "COM6"
serial_speed = 115200

if __name__ == "__main__":

    serial = pyserial.Serial(serial_port, serial_speed)

    serial.write('rst\n'.encode())

    slaves = { }

    while True:

        str_msg = serial.readline()
        if '{'.encode() not in str_msg:
            continue
        
        msg = json.loads(str_msg.decode())
        slave_msg = msg["msg"]
        #print(msg)
        
        if slave_msg["type"] == 0:
            slaves[msg["from"]] = Slave(msg["from"], slave_msg["name"])

        elif slave_msg["type"] == 2:
            if msg["from"] in slaves:
                slaves[msg["from"]].set_sensors(slave_msg["sensors"])
                print("\n\n\n\n\n\n\n\n")
                for s in slaves:
                    print(s)
        
        elif slave_msg["type"] == 9:
            if msg["from"] in slaves:
                for s in slave_msg["sensors"]:
                    slaves[msg["from"]].sensors[s["index"]].val = s["val"]
                print("\n\n\n\n\n\n\n\n")
                for k in slaves:
                    slave = slaves[k]
                    s = ''
                    s += 'name: {}\n'.format(slave.name)
                    s += '\tsensors:\n'
                    for i in slave.sensors:
                        s += '\tname: {}\n'.format(i.name)
                        s += '\t\tvalue {}\n'.format(i.val)
                    print(s.strip())

