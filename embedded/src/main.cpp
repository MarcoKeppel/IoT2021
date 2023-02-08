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
void initNode();
void initMesh();
void initSensors(StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson);

void masterLoop();
void slaveLoop();

void printSensors();

uint32_t currentMillis = 0;

painlessMesh mesh;

sensor_t sensors[MAX_SLAVE_SENSORS_N];

int sensors_n = 0;

uint8_t state = 0; // TODO: handle states with better state machine implementation (e.g. states enum)



void onReceive(uint32_t from, const String &msg);

void setup()
{

  Serial.begin(115200);
  Serial.println(); // Clear serial garbage

// DEBUG
#ifdef __FORCE_MASTER__
  state = UINT8_MAX;
#endif

  initMesh();
}

void loop()
{

  currentMillis = millis();

  // TODO: *master_data_t instance*.masterLoop();
  // TODO: *slave_data_t instance*.slaveLoop();

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
