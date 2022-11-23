#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "datastructs.h"

#define STATIC_JSON_DOC_SIZE 512

#define SENSORS_UPDATE_PERIOD 1000    // [ms]

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
void initSensors(StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson);

void printSensors();

sensor_t sensors[MAX_SLAVE_SENSORS_N];
int16_t sensors_update_rate[MAX_SLAVE_SENSORS_N] = { 0 };
int sensors_n = 0;

uint32_t lastSensorsUpdateMillis = 0;

const char* configFile = "config.json";     // Config JSON filename (stored in FS)

void setup() {
  
  Serial.begin(115200);
  Serial.println();       // Clear serial garbage

  initNode();
}

void loop() {
  
  if (millis() > lastSensorsUpdateMillis + SENSORS_UPDATE_PERIOD) {

    lastSensorsUpdateMillis = millis();

    for (int i = 0; i < sensors_n; i++) {

      sensors_update_rate[i]--;
      if (sensors_update_rate[i] <= 0) {
        
        sensors_update_rate[i] = sensors[i].update_rate;
        
        // Read sensor value, method should depend on val_type
        // For now assume it's always digitalRead()
        sensors[i].val = digitalRead(sensors[i].pin);

        Serial.printf("sensors[%d].val: %d\n", i, sensors[i].val);
      }
    }
  }
}

void initNode() {

  if(!LittleFS.begin()){
    Serial.println("error: could not init LittleFS");
    return;
  }

  if (!LittleFS.exists(configFile)) {
    Serial.printf("error: config file ('%s') does not exist\n", configFile);
    return;
  }

  File f = LittleFS.open(configFile, "r");
  if (!f) {
    Serial.printf("error: could not open config file (%s)\n", configFile);
  }

  StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson;
  deserializeJson(configJson, f.readString());

  Serial.println((const char*) configJson["name"]);

  initSensors(configJson);

  printSensors();

}

void initSensors(StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson) {

  JsonArray sensorsJson = configJson["sensors"].as<JsonArray>();

  for (JsonObject s : sensorsJson) {

    if (sensors_n >= MAX_SLAVE_SENSORS_N) break;

    // TODO: this should really be its own function
    
    // Serial.printf("initSensors(): '%s'\n", (const char*) s["name"]);
    sensors[sensors_n].name        =       (const char*) s["name"];
    sensors[sensors_n].type        =     (sensor_type_t) s["type"];
    sensors[sensors_n].val_type    = (sensor_val_type_t) s["val_type"];
    sensors[sensors_n].update_rate =           (uint8_t) s["update_rate"];
    sensors[sensors_n].pin         =           (uint8_t) s["pin"];
    sensors_n++;
  }
}

void printSensors() {

  for (int i = 0; i < sensors_n; i++) {

    Serial.printf(
      "{\n\tname: %s\n\ttype: %u\n\tval_type: %u\n\tupdate_rate: %u\n\tpin: %u\n},\n",
      sensors[i].name,
      sensors[i].type,
      sensors[i].val_type,
      sensors[i].update_rate,
      sensors[i].pin
    );
  }
}