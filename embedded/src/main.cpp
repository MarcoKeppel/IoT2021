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
uint32_t lastSensorsUpdateMillis = 0;
uint32_t lastBroadcastMillis = 0;

painlessMesh mesh;

// slave_data_t slaveData;

sensor_t sensors[MAX_SLAVE_SENSORS_N];

int sensors_n = 0;

uint8_t state = 0; // TODO: handle states with better state machine implementation (e.g. states enum)

void sendMasterAddrReq();
void sendSensorValUpdate();
void sendSensorListAdv();

void onReceive(uint32_t from, const String &msg);
void onReceiveMaster(uint32_t from, const JsonDocument &msg);
void onReceiveSlave(uint32_t from, const JsonDocument &msg);

slave_data_t slaveD(&mesh);

void setup()
{

  Serial.begin(115200);
  Serial.println(); // Clear serial garbage

  slaveD.slaveSetup();

// DEBUG
#ifdef __FORCE_MASTER__
  state = UINT8_MAX;
#endif

  initMesh();
}

void loop()
{

  currentMillis = millis();

  // masterLoop();
  // slaveLoop();

  // Run mesh update
  mesh.update();
}

void initMesh()
{

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&onReceive);
}

void masterLoop()
{
}

void onReceive(uint32_t from, const String &msg)
{

  Serial.printf("Received from %u msg:\n%s\n", from, msg.c_str());
  Serial.println();

  StaticJsonDocument<512> msgJson;
  deserializeJson(msgJson, msg);

  // onReceiveMaster(from, msgJson);
  onReceiveSlave(from, msgJson);
}

void onReceiveMaster(uint32_t from, const JsonDocument &msg)
{

  switch ((uint8_t)msg["type"])
  {

  case 0:
    mesh.sendSingle(from, "{\"type\": 1}");
    break;
  }
}

void onReceiveSlave(uint32_t from, const JsonDocument &msg)
{

  switch ((uint8_t)msg["type"])
  {

  case 1:
    // slaveData.masterAddr = from;
    state = 1;
    break;
  }
}
