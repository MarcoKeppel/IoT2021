#ifndef __ROLES_H__
#define __ROLES_H__

#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "config.h"

#include "datastructs.h"
#include "states.h"
#include <painlessMesh.h>
#include <ArduinoJson.h>

typedef struct slave_data
{

    const char *name;
    uint32_t masterAddr;
    sensor_t sensors[M_MAX_SLAVE_SENSORS_N]; // Static size array instead? With size defined by MAX_SLAVE_SENSORS_N macro
    int16_t sensors_update_rate[M_MAX_SLAVE_SENSORS_N] = {0};
    int sensors_n = 0;

    uint8_t state;
    uint32_t currentMillis = 0;
    uint32_t lastSensorsUpdateMillis = 0;
    uint32_t lastBroadcastMillis = 0;
    uint32_t lastAdvMillis = 0;

    painlessMesh *mesh;

    slave_data(painlessMesh *mesh)
    {
        this->mesh = mesh;

        // Set initial slave state
        this->state = SS_INIT;
    }

    void slaveSetup()
    {

        Serial.println("configging");
        // Load config from FS, and init sensors
        this->loadConfig();

        // Set slave state
        this->state = SS_MASTER_REQ;
    }

    void slaveLoop()
    {
        currentMillis = millis();
        // sendSensorListAdv();
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
            if (currentMillis >= lastAdvMillis + ADV_PERIOD)
            {
                lastAdvMillis = currentMillis;
                sendSensorListAdv();
            }

            break;

        case SS_SENS_UPD:
            if (currentMillis > lastSensorsUpdateMillis + SENSORS_UPDATE_PERIOD)
            {
                sendSensorValUpdate();
            }
            break;
        }
    }

    void onReceive(uint32_t from, const JsonDocument &msg)
    {

        uint8_t type = (uint8_t)msg["type"];
        Serial.printf("msgype: %u, curstate %u\n\r", type, state);
        switch (state)
        {

        case SS_MASTER_REQ:
            if (type == 1)
            { // TODO: macro for msg types
                this->masterAddr = from;
                Serial.printf("m: %u, from %u\n\r", masterAddr, from);
                this->state = SS_SENS_ADV;
#ifdef __S_SKIP_SENSOR_ADV__
                this->state = SS_SENS_UPD;
#endif
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

        Serial.printf("newstate: %u\n\r", state);
    }

    void sendMasterAddrReq()
    {

        StaticJsonDocument<512> msg; // TODO: define size as macro

        msg["id"] = mesh->getNodeId();
        msg["type"] = 0; // TODO: define types more formally

        char msgSerialized[256]; // TODO: define size as macro
        serializeJson(msg, msgSerialized);
        mesh->sendBroadcast(msgSerialized, true);
    }

    void sendSensorListAdv()
    {
        Serial.println("sending sens...");
        char msgSerialized[256];     // TODO: define size as macro
        StaticJsonDocument<512> msg; // TODO: define size as macro
        // Serial.printf("sens num: %d", sensors_n);
        msg["type"] = 2;
        // msg["sensors"] =
        // for (int i = 0; i < sensors_n; i++)
        // {
        //     // Serial.printf(
        //     //     "{\n\tname: %s\n\ttype: %u\n\tval_type: %u\n\tupdate_rate: %u\n\tpin: %u\n},\n",
        //     //     sensors[i].name,
        //     //     sensors[i].type,
        //     //     sensors[i].val_type,
        //     //     sensors[i].update_rate,
        //     //     sensors[i].pin);
        //     // msg["name"] = sensors[i].name;
        //     // TODO chage msg type
        //     // msg["update_rate"] = sensors[i].update_rate;
        // }

        // Serial.println((const char *)msg["name"]);
        // Serial.println((const char *)msg["type"]);
        // Serial.println((const char *)msg["update_rate"]);
        // Serial.printf("------------------- mad: %d\n\r", masterAddr);
        // // Serial.println(msg["type"]);
        serializeJson(msg, msgSerialized);
        Serial.println("serializd message...");
        mesh->sendSingle(masterAddr, msgSerialized);
        Serial.println("message sent! :)");
    }

    void sendSensorValUpdate()
    {

        lastSensorsUpdateMillis = currentMillis;

        // Create JSON doc...
        StaticJsonDocument<512> msg; // TODO: define size as macro
        msg["id"] = mesh->getNodeId();
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
            mesh->sendSingle(masterAddr, msgSerialized);
        }
    }

    void loadConfig()
    {

//  ----------------------------------------------------------------
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

//  TODO: make this code reusable (function) (not only to slave, but also master)
//  ---------------------------------------------------------------

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson;
        deserializeJson(configJson, f.readString());

        Serial.println((const char *)configJson["name"]);
        this->name = (const char *)configJson["name"];

        initSensors(configJson);

        printSensors();
    }

    void initSensors(StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson)
    {

        JsonArray sensorsJson = configJson["sensors"].as<JsonArray>();

        for (JsonObject s : sensorsJson)
        {

            if (sensors_n >= M_MAX_SLAVE_SENSORS_N)
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
} slave_data_t;

typedef struct master_data
{

    char *name;
    slave_t slaves[M_MAX_SLAVES_N];

    uint8_t state;
    uint32_t currentMillis = 0;
    uint32_t lastSensorsUpdateMillis = 0;
    uint32_t lastBroadcastMillis = 0;
    painlessMesh *mesh;

    master_data(painlessMesh *mesh)
    {

        this->mesh = mesh;

        this->state = MS_INIT;
    }

    void masterSetup()
    {
    }

    void masterLoop()
    {
    }

    void onReceive(uint32_t from, const JsonDocument &msg)
    {

        uint8_t type = (uint8_t)msg["type"];

        Serial.printf("Message:\n\tfrom: %u\n\ttype: %u\n", from, type);

        switch (type)
        {

            case 0:
                sendMasterAddrResp(from);
                break;
            
            case 2:
                sendSensorListAck(from);
                break;
        }
    }

    void sendMasterAddrResp(uint32_t dest)
    {

        StaticJsonDocument<512> msg; // TODO: define size as macro

        msg["id"] = mesh->getNodeId();
        msg["type"] = 1; // TODO: define types more formally

        char msgSerialized[256]; // TODO: define size as macro
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(dest, msgSerialized);
    }

    void sendSensorListAck(uint32_t dest) {

        StaticJsonDocument<512> msg; // TODO: define size as macro

        msg["id"] = mesh->getNodeId();
        msg["type"] = 3; // TODO: define types more formally

        char msgSerialized[256]; // TODO: define size as macro
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(dest, msgSerialized);
    }

} master_data_t;

node_role_t getNodeRole() {

//  ----------------------------------------------------------------
    if (!LittleFS.begin())
    {
        Serial.println("error: could not init LittleFS");
        Serial.printf("warning: 'role' not specified in config file. Default value '%d' will be used.\n", NODE_DEFAULT_ROLE);
        return NODE_DEFAULT_ROLE;
    }

    if (!LittleFS.exists(configFile))
    {
        Serial.printf("error: config file ('%s') does not exist\n", configFile);
        Serial.printf("warning: 'role' not specified in config file. Default value '%d' will be used.\n", NODE_DEFAULT_ROLE);
        return NODE_DEFAULT_ROLE;
    }

    File f = LittleFS.open(configFile, "r");
    if (!f)
    {
        Serial.printf("error: could not open config file (%s)\n", configFile);
        Serial.printf("warning: 'role' not specified in config file. Default value '%d' will be used.\n", NODE_DEFAULT_ROLE);
        return NODE_DEFAULT_ROLE;
    }
//  TODO: make this code reusable (function)
//  ---------------------------------------------------------------

    StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson;
    deserializeJson(configJson, f.readString());

    JsonVariant jsonRole = configJson["role"];
    node_role_t role;

    if (jsonRole.isNull()) {

        Serial.printf("warning: 'role' not specified in config file. Default value '%d' will be used.\n", NODE_DEFAULT_ROLE);
        role = NODE_DEFAULT_ROLE;
    }
    else {
        role = (node_role_t) jsonRole;
    }

    Serial.printf("role: %d\n", role);
    return role;
}

#endif
