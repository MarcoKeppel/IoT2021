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

slave_data_t slaveData;

sensor_t sensors[MAX_SLAVE_SENSORS_N];
int16_t sensors_update_rate[MAX_SLAVE_SENSORS_N] = {0};
int sensors_n = 0;

uint32_t lastSensorsUpdateMillis = 0;

uint8_t state = 0;      // TODO: handle states with better state machine implementation (e.g. states enum)

uint32_t lastBroadcastMillis = 0;

void sendMasterAddrReq();
void sendSensorValUpdate();
void sendSensorListAdv();

void onReceive(uint32_t from, const String &msg);
void onReceiveMaster(uint32_t from, const JsonDocument &msg);
void onReceiveSlave(uint32_t from, const JsonDocument &msg);

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

  // masterLoop();
  slaveLoop();

  // Run mesh update
  mesh.update();
}

void initMesh() {

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&onReceive);
}

void sendMasterAddrReq()
{

  StaticJsonDocument<512> msg; // TODO: define size as macro

  msg["id"] = mesh.getNodeId();
  msg["type"] = 0; // TODO: define types more formally

  char msgSerialized[256]; // TODO: define size as macro
  serializeJson(msg, msgSerialized);
  mesh.sendBroadcast(msgSerialized, true);
}

void sendSensorValUpdate()
{

  lastSensorsUpdateMillis = currentMillis;

  // Create JSON doc...
  StaticJsonDocument<512> msg; // TODO: define size as macro
  msg["id"] = mesh.getNodeId();
  msg["type"] = 9; // TODO: define types more formally
  JsonArray sensorsArray = msg.createNestedArray("sensors");

  // ...then cycle through all sensors and add those that need to be updated
  for (int i = 0; i < sensors_n; i++)
  {

    // 1 period has passed!
    sensors_update_rate[i]--;

    if (sensors_update_rate[i] <= 0)
    {

      sensors_update_rate[i] = sensors[i].update_rate;

      // Read sensor value, method should depend on val_type
      // For now assume it's always digitalRead()
      sensors[i].val = digitalRead(sensors[i].pin);

      // Serial.printf("sensors[%d].val: %d\n", i, sensors[i].val);

      // Add data to message
      JsonObject sensorObject = sensorsArray.createNestedObject();
      sensorObject["index"] = i;
      sensorObject["val"] = sensors[i].val;
    }
  }

  // If some sensors have been updated, send message
  if (!(sensorsArray.size() <= 0))
  {

    char msgSerialized[256]; // TODO: define size as macro
    // String msgSerialized;
    serializeJson(msg, msgSerialized);
    mesh.sendSingle(slaveData.masterAddr, msgSerialized);
  }
}

void sendSensorListAdv()
{
}

void masterLoop()
{
}

void slaveLoop()
{

  switch (state)
  {

  case SS_MASTER_REQ:
    if (currentMillis >= lastBroadcastMillis + BROADCAST_PERIOD)
    {
      lastBroadcastMillis = currentMillis;
      sendMasterAddrReq();
    }
    break;

  case SS_SENS_ADV:
    sendSensorListAdv();
    break;

  case SS_SENS_UPD:
    if (currentMillis > lastSensorsUpdateMillis + SENSORS_UPDATE_PERIOD)
    {
      sendSensorValUpdate();
    }
    break;
  }
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
    slaveData.masterAddr = from;
    state = 1;
    break;
  }
}
