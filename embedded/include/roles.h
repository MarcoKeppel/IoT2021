#ifndef __ROLES_H__
#define __ROLES_H__

#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "datastructs.h"
#include "states.h"
#include "config.h"


typedef struct {

    const char* name;
    uint32_t masterAddr;
    sensor_t sensors[MAX_SLAVE_SENSORS_N];      // Static size array instead? With size defined by MAX_SLAVE_SENSORS_N macro
    int sensors_n = 0;

    uint8_t state;

    void loadConfig() {

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
        this->name = (const char*) configJson["name"];

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

} slave_data_t;

typedef struct {
    
    char* name;
    slave_t* slaves;        // Static size array instead? With size defined by MAX_SLAVES_N macro

    uint8_t state;

} master_data_t;

#endif
