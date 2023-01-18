#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

<<<<<<< HEAD
#include "roles.h"

=======
#define MESH_SSID "UNITN_IOT"
#define MESH_PASSWORD "isthissecureenough?"
#define MESH_PORT 2468

#include "roles.h"

#define STATIC_JSON_DOC_SIZE 512

#define SENSORS_UPDATE_PERIOD 1000 // [ms]

>>>>>>> fd980640dc79a96805058b1ec9861da61b0bd46b
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

<<<<<<< HEAD
uint8_t state = 0;      // TODO: handle states with better state machine implementation (e.g. states enum)

=======
const char *configFile = "config.json"; // Config JSON filename (stored in FS)

uint8_t state = 0; // TODO: handle states with better state machine implementation (e.g. states enum)
#define BROADCAST_PERIOD 2000
>>>>>>> fd980640dc79a96805058b1ec9861da61b0bd46b
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

<<<<<<< HEAD
void initMesh() {
=======
void initNode()
{

  if (!LittleFS.begin())
  {
    Serial.println("error: could not init LittleFS");
    return;
  }

  if (!LittleFS.exists(configFile))
  {
    Serial.printf("error: config file ('%s') does not exist\n", configFile);
    return;
  }

  File f = LittleFS.open(configFile, "r");
  if (!f)
  {
    Serial.printf("error: could not open config file (%s)\n", configFile);
  }

  StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson;
  deserializeJson(configJson, f.readString());

  Serial.println((const char *)configJson["name"]);
  slaveData.name = (const char *)configJson["name"];

  initSensors(configJson);

  printSensors();
}

void initSensors(StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson)
{

  JsonArray sensorsJson = configJson["sensors"].as<JsonArray>();

  for (JsonObject s : sensorsJson)
  {

    if (sensors_n >= MAX_SLAVE_SENSORS_N)
      break;

    // TODO: this should really be its own function

    // Serial.printf("initSensors(): '%s'\n", (const char*) s["name"]);
    sensors[sensors_n].name = (const char *)s["name"];
    sensors[sensors_n].type = (sensor_type_t)s["type"];
    sensors[sensors_n].val_type = (sensor_val_type_t)s["val_type"];
    sensors[sensors_n].update_rate = (uint8_t)s["update_rate"];
    sensors[sensors_n].pin = (uint8_t)s["pin"];
    sensors_n++;
  }

  slaveData.sensors = sensors;
}

void printSensors()
{

  for (int i = 0; i < sensors_n; i++)
  {

    Serial.printf(
        "{\n\tname: %s\n\ttype: %u\n\tval_type: %u\n\tupdate_rate: %u\n\tpin: %u\n},\n",
        sensors[i].name,
        sensors[i].type,
        sensors[i].val_type,
        sensors[i].update_rate,
        sensors[i].pin);
  }
}

void initMesh()
{
>>>>>>> fd980640dc79a96805058b1ec9861da61b0bd46b

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
