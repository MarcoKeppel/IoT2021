#from gevent import monkey
#monkey.patch_all()



import serial as pyserial
import json
import threading
from datastructs import *


from tkinter import *
from tkinter import ttk

root = Tk()

# frm = ttk.Frame(root, padding=10)
# label_frame = ttk.LabelFrame(frm, text = "LABELFRAME WIDGET").grid(column=0, row=0)
# frm.grid()

frm = ttk.Frame(root, padding=10)
frm.grid()

# create a LabelFrame inside the frame
lblfrm = ttk.LabelFrame(frm, text='My LabelFrame')
lblfrm.grid()

# add some content to the LabelFrame
lbl = ttk.Label(lblfrm, text='This is my LabelFrame')
lbl.grid()

# ttk.Label(frm, text="Hello World!").grid(column=0, row=0)
# ttk.Button(frm, text="Quit", command=root.destroy).grid(column=1, row=0)


serial_port = "COM6"
serial_speed = 115200

def serial_thread():

    print("SERIAL STARTING")

    serial = pyserial.Serial(serial_port, serial_speed)

    serial.write('rst\n'.encode())

    slaves = { }
    labels = { }

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
                label_txt = ""
                label_txt = ''
                label_txt += 'name: {}\n'.format(slaves[msg["from"]].name)
                label_txt += '\tsensors:\n'
                for i in slaves[msg["from"]].sensors:
                    label_txt += '\tname: {}\n'.format(i.name)
                    label_txt += '\t\tvalue {}\n'.format(i.val)
                labels[msg["from"]] = ttk.Label( root, text = label_txt.strip())
                labels[msg["from"]].grid(column=(len(labels) % 2), row=(int(len(labels) / 2)), padx=10, pady=10)
                print("\n\n\n\n\n\n\n\n")
                for s in slaves:
                    print(s)
        
        elif slave_msg["type"] == 9:
            if msg["from"] in slaves:
                for s in slave_msg["sensors"]:
                    slaves[msg["from"]].sensors[s["index"]].val = s["val"]
                label_txt = ""
                label_txt = ''
                label_txt += 'name: {}\n'.format(slaves[msg["from"]].name)
                label_txt += '\tsensors:\n'
                for i in slaves[msg["from"]].sensors:
                    label_txt += '\tname: {}\n'.format(i.name)
                    label_txt += '\t\tvalue {}\n'.format(i.val)
                labels[msg["from"]].config(text=label_txt)
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


if __name__ == "__main__":

    ts = threading.Thread(target=serial_thread)
    ts.start()

    root.mainloop()
