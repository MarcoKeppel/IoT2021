import os
import time
import serial as pyserial
from serial.tools.list_ports import comports
import json
from rich.console import Console, Group
from rich.layout import Layout
from rich.prompt import Prompt
from rich.table import Table
from rich.panel import Panel
from rich.columns import Columns
from rich.live import Live
from datastructs import *


console = Console()

serial_port = "COM6"
serial_speed = 115200

msg_tot = 0

slaves = { }

print = console.print

terminal_height = 30

def gen_ui():

    slaves_p = gen_slaves()
    info_p = gen_info()

    panel_group = Group(
        slaves_p,
        info_p,
    )

    panel = Panel(
        panel_group,
        title="[REDACTED]",
        border_style="red",
        title_align="left",
        padding=(1, 2),
        height=terminal_height
    )
    #print(panel)
    return Layout(panel)


def gen_slaves():

    if len(slaves) == 0:
        panel = Panel(
            "",
            title="Slaves",
            border_style="bright_black",
            title_align="left",
            padding=(1, 2),
            expand=False
        )
        return panel

    tables = []

    for k in slaves:

        v = slaves[k]
        t = Table(show_header=True, header_style="bright_black", )

        t.add_column("Sensor")
        t.add_column("Value")

        for s in v.sensors:
            t.add_row(
                s.name, str(s.val)
            )

        p = Panel.fit(
            t,
            title="ID: " + str(k),
            border_style="gold1",
            title_align="center",
            padding=(1, 1),
        )
        tables.append(p)
        tables.append(p)    # DELETEME

    panel = Panel(
        Columns(tables, expand=False),
        title="Slaves",
        border_style="bright_black",
        title_align="left",
        padding=(1, 1),
        expand=False
    )
    return panel


def gen_info():

    t = Table(show_header=False, header_style="bright_black", )
    t.add_row("Serial port", str(serial_port))
    t.add_row("Serial speed", str(serial_speed))
    t.add_row("#slaves", str(len(slaves)))
    t.add_row("#msg received", str(msg_tot))

    panel = Panel(
        t,
        title="Info",
        border_style="bright_black",
        title_align="left",
        padding=(1, 1),
        expand=False
    )
    return panel


live = Live(gen_ui(), auto_refresh=False)


if __name__ == "__main__":

    t = Table(title="List of all serial ports")
    t.add_column("Name")
    t.add_column("Description")
    for port in comports():
        t.add_row(str(port.device), str(port.description))
    l = Layout(t)
    print(l)

    serial_port = Prompt.ask("Enter the serial port (Name)")
    serial_speed = int(Prompt.ask("Enter the serial port speed", choices=["9600", "35400", "115200"], default="115200"))

    print("Terminal height:")
    print(os.get_terminal_size().lines)
    if (os.get_terminal_size().lines) > 8:
        terminal_height = os.get_terminal_size().lines

    print("SERIAL STARTING")

    #print(gen_ui())
    live.start()

    serial = pyserial.Serial(serial_port, serial_speed)

    serial.write('upd\n'.encode())

    while True:

        str_msg = serial.readline()
        if '{'.encode() not in str_msg:
            continue

        msg_tot += 1
        
        msg = json.loads(str_msg.decode())
        slave_msg = msg["msg"]
        #print(msg)
        
        if slave_msg["type"] == 0:
            slaves[msg["from"]] = Slave(msg["from"], slave_msg["name"])

        elif slave_msg["type"] == 2:
            if msg["from"] in slaves:
                slaves[msg["from"]].set_sensors(slave_msg["sensors"])
        
        elif slave_msg["type"] == 9:
            if msg["from"] in slaves:
                for s in slave_msg["sensors"]:
                    slaves[msg["from"]].sensors[s["index"]].val = s["val"]
            live.update(gen_ui())
            live.refresh()
            #print(gen_ui())
