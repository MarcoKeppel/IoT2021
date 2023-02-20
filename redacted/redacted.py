#from gevent import monkey
#monkey.patch_all()



import serial as pyserial
import json
import threading
import time
from datastructs import *


import gui


serial_port = "COM6"
serial_speed = 115200

event = threading.Event()

app = gui.SlavesManager()

def serial_thread():

    global app

    print("SERIAL STARTING")

    # serial = pyserial.Serial()
    # serial.port = serial_port
    # serial.baudrate = serial_speed
    # # serial.timeout = 1
    # serial.setDTR(False)
    # serial.open()
    serial = pyserial.Serial(serial_port, serial_speed)

    time.sleep(1)

    serial.reset_input_buffer()

    serial.write('upd\n'.encode())

    slaves = { }

    while True:

        if event.is_set():
            serial.close()
            break

        if not serial.in_waiting:
            time.sleep(0.01)
            continue

        str_msg = serial.readline()
        if '{'.encode() not in str_msg:
            continue
        
        msg = json.loads(str_msg.decode())
        # slave_msg = msg["msg"]
        #print(msg)
        app.send_msg(msg)
        #print(msg)
        
        # if slave_msg["type"] == 0:
        #     slaves[msg["from"]] = Slave(msg["from"], slave_msg["name"])

        # elif slave_msg["type"] == 2:
        #     if msg["from"] in slaves:
        #         slaves[msg["from"]].set_sensors(slave_msg["sensors"])
        #         print("\n\n\n\n\n\n\n\n")
        #         for s in slaves:
        #             print(s)
        
        # elif slave_msg["type"] == 9:
        #     if msg["from"] in slaves:
        #         for s in slave_msg["sensors"]:
        #             slaves[msg["from"]].sensors[s["index"]].val = s["val"]
        #         print("\n\n\n\n\n\n\n\n")
        #         for k in slaves:
        #             slave = slaves[k]
        #             s = ''
        #             s += 'name: {}\n'.format(slave.name)
        #             s += '\tsensors:\n'
        #             for i in slave.sensors:
        #                 s += '\tname: {}\n'.format(i.name)
        #                 s += '\t\tvalue {}\n'.format(i.val)
        #             print(s.strip())


#slaves = 

if __name__ == "__main__":

    ts = threading.Thread(target=serial_thread, args=[ ])
    ts.start()
    app.run()
    event.set()
    ts.join()
