#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "roles.h"

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

void setup()
{

  Serial.begin(115200);
  Serial.println(); // Clear serial garbage

  initMesh();

#ifdef __FORCE_MASTER__
  masterD.masterSetup();
#else
  slaveD.slaveSetup();
#endif
}

void loop()
{

  currentMillis = millis();

#ifdef __FORCE_MASTER__
  masterD.masterLoop();
#else
  slaveD.slaveLoop();
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

  Serial.printf("Received from %u msg:\n%s\n", from, msg.c_str());
  Serial.println();

  StaticJsonDocument<512> msgJson;
  deserializeJson(msgJson, msg);

  // TODO: *master_data_t instance*.onReceive(from, msgJson);
  // TODO: *slave_data_t instance*.onReceive(from, msgJson);
}
