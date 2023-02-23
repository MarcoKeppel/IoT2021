# Dynamic Autonomous Sensor Hub (DASH)

Exam project, "Embedded Software for the Internet of Things", UniTN.

## About

This project is an implementation of a sensors system based on ad hoc/mesh network. It aims to be a (mostly) painless and easy to implement solution for challenging network environments requiring a network of (potentially diverse) sensors and monitoring devices.

It aims to be a very low cost solution, only requiring inexpensive microcontrollers to run. It does not need any expensive network equipment.

## Table of contents

* [General Information](#general-information)
    * [Directory Structure](#directory-structure)
    * [Devices](#devices)
    * [Protocol](#protocol)
    * [GUI](#gui)
* [Requirements](#requirements)
    * [Dependencies](#dependencies)
* [Resources](#resources)

## General Information

The idea is to have a network able of handle communications indipendently from how many devices are connected and what kind of sensors are in each device. The entire project was developed with modularity in mind, keeping every part as generic as possible to allow a broader array of sensor and devices to be added laater on.

PainlessMesh is used as a base for the custom network, it provides the foundamentals to allow all the devices to communicate with eachother, what's built on top of it is loosely based on DHCP and uses it's own simple protocol.

![pmesh](https://raw.githubusercontent.com/MarcoKeppel/IoT2021/main/readme/ESP-MESH-painlessMesh-basic-example-ESP32-ESP8266.webp)

### Directory Structure

```
.
├── embedded                # PlatformIO project root
│   ├── data                # Data to be uploaded to the ESP flash storage
│   │   └── config.json     # Slave config file
│   ├── include             # .h files (datastructs, master/slave logic, ...)
│   ├── src                 # main.cpp
│   ├── platformio.ini      # PlatformIO config file
│   └── ...
├── redacted                # [REDACTED] tool
│   ├── datastructs.py
│   └── redacted.py
└── README.md
```

### Devices
Network devices can either be of role slave,  master or slave_master. What role the device will play is defined at startup when a configuration json file is loaded from memory (the file also contains a description of all the sensors connected to the device), only one master is allowed per network and the device covering that role is the one that will be communicating via serial with the host machine. If role is slave_master the device will operate both as a master and as a slave.

The master device, after the initialization will keep looping thru the slaves and keep sending them keepalive messages to make sure they are still connected to the network, when a slave fails to respond it will be removed from list and flagged as inactive.
Messages are handled asynchronously outside of the loop and the behaviour is dictated by the protocol.

Slave devices internally work akin to a finite state machine. The device moves between 4 different states:
```
- SS_INIT 0
- SS_MASTER_REQ 1
- SS_SENS_ADV 2 
- SS_SENS_UPD 3
```

![states](https://github.com/MarcoKeppel/IoT2021/blob/main/readme/Screenshot%20from%202023-02-22%2019-06-20.png)

```SS_INIT``` is the initialization state where the file system is initialized and the configuration file is loaded from memory, parsed and variables set accordingly, done that it will quickly move to the next state.

```SS_MASTER_REQ``` when in this state the slave will intermittently broadcast on the network until it'll get a response from the master with its adress.

```SS_SENS_ADV``` once received a response from master the slave with move to this state and it'll send to master informations regarding its sensors

```SS_SENS_UPD``` when master responds with an ack the slave with move to this last and stable state where it'll be sending to masters updates on its sensors accoringly to the timing described in the configuration JSON file

### Protocol
The protocol is very simple but robust, it consists of 9 types of messages:

```
- MSG_ROOT_ID_REQ 0
- MSG_ROOT_ID_RESP 1
- MSG_SENSOR_LIST_ADV 2 
- MSG_SENSOR_LIST_ACK 3
- MSG_SENSOR_VALUE_REQ 8
- MSG_SENSOR_VALUE_RESP 9
- MSG_KEEPALIVE 10
- MSG_KEEPALIVE_ACK 11 
- MSG_SLAVE_RESET 255
```

```MSG_ROOT_ID_REQ```: broadcast from slave asking for master adress

```MSG_ROOT_ID_RESP```: response from master containing address

```MSG_SENSOR_LIST_ADV```: contains informations regarding slave's sensors 

```MSG_SENSOR_LIST_ACK```: ack response of MSG_SENSOR_LIST_ADV

```MSG_SENSOR_VALUE_REQ```: request from master to get updates on sensors (unused)

```MSG_SENSOR_VALUE_RESP```: slave send to master the updated sensor values

```MSG_KEEPALIVE```: master ask slave if it's still connected

```MSG_KEEPALIVE_ACK```: response from slave stating it's still connected

```MSG_SLAVE_RESET```: message from master to force the reset of a slave

Keepalive messages are tailored differently for each slave. Slaves that contain sensors with high update rates will be queried more often.

### GUI

```[REDACTED]``` is the graphical tool to check the status of the slaves. The user interface consists of a neatly formatted command line UI, the main goal of the interface is to present in an clean and presentable way the informations coming via serial from the master device, very little is actually stored in the host machine. It's a very simple tool, as this was not the main focus of this project.

The graphics are implemented using [Rich](https://github.com/Textualize/rich)

## Requirements

This solution is based on ESP8266 microcontrollers. ESP32s could (in theory) be used instead, but dependencies will likely change slightly (At least one of the dependencies of painlessMesh is different, [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) instead of ESPAsyncTCP).

Following is a basic bill of materials:
- ESP8266 (at least two)
- some kind of "sensors" to connect to the node(s) ESPs (LDR, temperature, humidity, ...)

### Dependencies

As for the software, this project is built using the Arduino core for ESP8266 (linked below). Some external libraries are also needed.
- [PlatformIO](https://platformio.org/)
- [Arduino core for ESP8266](https://github.com/esp8266/Arduino)
- [painlessMesh (1.5.0)](https://gitlab.com/painlessMesh/painlessMesh)
    - [ArduinoJson (6.19.4)](https://github.com/bblanchon/ArduinoJson)
    - [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
    - [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)

Recent changes of the ArdionoJson library have introduced incompatibilities with painlessMesh. Pay attention to the version you're using, and use those indicated above when in doubt. When using PlatformIO to manage the dependencies, there should not be any problem as dependencies of dependencies are managed automatically.

## Resources

- Project video: [youtube.com/watch?v=kmNVzmr6Ldo](https://www.youtube.com/watch?v=kmNVzmr6Ldo)
