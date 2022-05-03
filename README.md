```
This readme is still a *work in progress*. ✨
```

# IoT 2021

Exam project, "Embedded Software for the Internet of Things", UniTN.

## About

This project is an implementation of a sensors system based on ad hoc/mesh network. It aims to be a (mostly) painless and easy to implement solution for challenging network environments requiring a network of (potentially diverse) sensors and monitoring devices.

It aims to be a very low cost solution, only requiring inexpensive microcontrollers to run. It does not need any expensive network equipment.

## Table of contents

* [General Information](#general-information)
* [Requirements](#requirements)
    * [Dependencies](#dependencies)

## General Information

```TODO```: some technical details (root/node, net. comm. protocol, ...)

## Requirements

This solution is based on ESP8266 microcontrollers. ESP32s could (in theory) be used instead, but dependencies will likely change slightly (```TODO```: it's at least one of the dependencies of painlessMesh).

Following is a basic bill of materials:
- ESP8266 (at least two)
- some kind of "sensors" to connect to the node(s) ESPs (LDR, temperature, humidity, ...)

### Dependencies

As for the software, this project is built using the Arduino core for ESP8266 (linked below). Some external libraries are also needed.
- [PlatformIO](https://platformio.org/)
- [Arduino core for ESP8266](https://github.com/esp8266/Arduino)
- [painlessMesh](https://gitlab.com/painlessMesh/painlessMesh)
    - [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
    - [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
    - [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)

    ```TODO```: write versions of the libraries (and possible incompatibilities)
