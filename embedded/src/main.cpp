#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "common.h"
#include "slave.h"
#include "master.h"

// Pin mapping 'pins_arduino.h'
/*
  static const uint8_t D0   = 16;
  static const uint8_t D1   = 5;
  static const uint8_t D2   = 4;
  static const uint8_t D3   = 0;
  static const uint8_t D4   = 2;
  static const uint8_t D5   = 14;
  static const uint8_t D6   = 12;
  static const uint8_t D7   = 13;
  static const uint8_t D8   = 15;
  static const uint8_t D9   = 3;
  static const uint8_t D10  = 1;
*/

// Prototypes
void initMesh();

void printSensors();

uint32_t currentMillis = 0;

painlessMesh mesh;

void onReceive(uint32_t from, const String &msg);

slave_data_t slaveD(&mesh);

master_data_t masterD(&mesh);

node_role_t role;

void setup()
{

  Serial.begin(115200);
  Serial.println("\n----------------"); // Clear serial garbage

  initMesh();

  role = getNodeRole();

#ifdef __FORCE_MASTER__
  role = master;
  masterD.masterSetup();
#else

  switch (role)
  {

  case master:
    masterD.masterSetup();
    break;

  case slave:
    slaveD.slaveSetup();
    break;

  case master_slave:
    masterD.masterSetup();
    slaveD.slaveSetup();
  }

#endif
}

void loop()
{

  currentMillis = millis();

#ifdef __FORCE_MASTER__
  masterD.masterLoop();
#else

  switch (role)
  { // TODO: make so this code can be reused (maybe function with functions to execute for each case as params?)

  case master:
    masterD.masterLoop();
    break;

  case slave:
    slaveD.slaveLoop();
    break;

  case master_slave:
    masterD.masterLoop();
    slaveD.slaveLoop();
  }

#endif

  // Run mesh update
  mesh.update();
}

void initMesh()
{

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&onReceive);
}

void onReceive(uint32_t from, const String &msg)
{

  //Serial.printf("Received from %u msg:\n%s\n", from, msg.c_str());
  //Serial.println();

#ifdef __FORCE_MASTER__
  masterD.onReceive(from, msgJson);
#else

  switch (role)
  {

  case master:
    masterD.onReceive(from, msg);
    break;

  case slave:
    slaveD.onReceive(from, msg);
    break;

  case master_slave:
    masterD.onReceive(from, msg);
    slaveD.onReceive(from, msg);
  }

#endif
}
