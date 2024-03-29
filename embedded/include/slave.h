#ifndef __SLAVE_H__
#define __SLAVE_H__

#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "config.h"

#include "datastructs.h"
#include "states.h"
#include "protocol.h"
#include "common.h"

#include <painlessMesh.h>
#include <ArduinoJson.h>
typedef struct slave_data
{

    char name[SLAVE_NAME_LEN];
    uint32_t masterAddr;
    sensor_t sensors[M_MAX_SLAVE_SENSORS_N]; // Static size array instead? With size defined by MAX_SLAVE_SENSORS_N macro
    int16_t sensors_update_rate[M_MAX_SLAVE_SENSORS_N] = {0};
    int sensors_n = 0;

    uint8_t state;
    uint32_t currentMillis = 0;
    uint32_t lastSensorsUpdateMillis = 0;
    uint32_t lastBroadcastMillis = 0;
    uint32_t lastAdvMillis = 0;

    uint32_t minUpdateRate = 3;

    painlessMesh *mesh;

    char msg_str[100];
    char state_str[100];

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

                // TODO REMOVE ME
                printSensors();
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

    void onReceive(uint32_t from, const String &msg_serialized)
    {

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;
        deserializeJson(msg, msg_serialized);

        uint8_t type = (uint8_t)msg["type"];

        stateType(state_str, state);
        msgType(msg_str, type);
        Serial.printf("msgtype: %s, curstate %s\n\r", msg_str, state_str);
        switch (state)
        {

        case SS_MASTER_REQ:
            if (type == MSG_ROOT_ID_RESP)
            {
                this->masterAddr = from;
                Serial.printf("found master on: %u\n\r", from);
                this->state = SS_SENS_ADV;
            }
            break;

        case SS_SENS_ADV:
            if (type == MSG_SENSOR_LIST_ACK)
            {
                Serial.printf("master %u has recieved sensor adv\n\r", from);
                this->state = SS_SENS_UPD;
            }
            break;

        case SS_SENS_UPD:
            if (type == MSG_KEEPALIVE)
            {
                Serial.printf("we received a KEEPALIVE! ;)\n\r");
                sendKeepaliveAck();
            }
            break;
        }

        // Special command: slave reset (MSG_SLAVE_RESET 255)
        if (type == MSG_SLAVE_RESET) {
            Serial.printf("!slave reset command from master!\n");
            this->state = SS_MASTER_REQ;
        }

        stateType(state_str, state);
        Serial.printf("newstate: %s\n\r", state_str);
    }
    void sendKeepaliveAck()
    {

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;

        msg["id"] = mesh->getNodeId();
        msg["type"] = MSG_KEEPALIVE_ACK;

        char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(masterAddr, msgSerialized);
    }

    void sendMasterAddrReq()
    {

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;

        msg["id"] = mesh->getNodeId();
        msg["type"] = MSG_ROOT_ID_REQ;
        msg["name"] = this->name;

        char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
        serializeJson(msg, msgSerialized);
        mesh->sendBroadcast(msgSerialized, true);
    }

    void sendSensorListAdv()
    {
        Serial.println("sending sens...");
        char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;
        // Serial.printf("sens num: %d", sensors_n);
        msg["type"] = MSG_SENSOR_LIST_ADV;

        msg["min_update_rate"] = this->minUpdateRate;

        JsonArray sensorsArray = msg.createNestedArray("sensors");

        // ...then cycle through all sensors and add those that need to be updated
        for (int i = 0; i < sensors_n; i++)
        {

            // Add data to message
            JsonObject sensorObject = sensorsArray.createNestedObject();
            sensorObject["name"] = sensors[i].name;
            sensorObject["type"] = sensors[i].type;
            sensorObject["val_type"] = sensors[i].val_type;
            sensorObject["update_rate"] = sensors[i].update_rate;
            // sensorObject["timeout_period"] = sensors[i].timeout_period;
        }

        // Serial.println((const char *)msg["name"]);
        //  Serial.println((const char *)msg["type"]);
        //  Serial.println((const char *)msg["update_rate"]);
        //  Serial.printf("------------------- mad: %d\n\r", masterAddr);
        //  // Serial.println(msg["type"]);
        serializeJson(msg, msgSerialized);
        Serial.println("serializd message...");
        mesh->sendSingle(masterAddr, msgSerialized);
        Serial.println("message sent! :)");
    }

    void sendSensorValUpdate()
    {

        lastSensorsUpdateMillis = currentMillis;

        // Create JSON doc...
        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;
        msg["id"] = mesh->getNodeId();
        msg["type"] = MSG_SENSOR_VALUE_RESP;
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
                sensors[i].read();

                // Serial.printf("sensors[%d].val: %d\n", i, sensors[i].val);

                // Add data to message
                JsonObject sensorObject = sensorsArray.createNestedObject();
                sensorObject["index"] = i;
                switch (sensors[i].val_type) {

                    case v_int:
                        sensorObject["val"] = sensors[i].getValue<int32_t>();
                        break;
                    
                    case v_uint:
                        sensorObject["val"] = sensors[i].getValue<uint32_t>();
                        break;

                    case v_real:
                        sensorObject["val"] = sensors[i].getValue<float_t>();

                    case v_bool:
                        sensorObject["val"] = sensors[i].getValue<bool>();
                }
            }
        }

        // If some sensors have been updated, send message
        if (!(sensorsArray.size() <= 0))
        {

            char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
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
        strcpy(this->name, (const char *)configJson["name"]);

        initSensors(configJson);

        printSensors();
    }

    void initSensors(StaticJsonDocument<STATIC_JSON_DOC_SIZE> configJson)
    {

        JsonArray sensorsJson = configJson["sensors"].as<JsonArray>();
        uint32_t tmpMinUpdateRate = 2000;
        for (JsonObject s : sensorsJson)
        {

            if (sensors_n >= M_MAX_SLAVE_SENSORS_N)
                break;

            // TODO: this should really be its own function

            // Serial.printf("initSensors(): '%s'\n", (const char*) s["name"]);
            strcpy(sensors[sensors_n].name, (const char *)s["name"]);
            sensors[sensors_n].type = (sensor_type_t)s["type"];
            sensors[sensors_n].val_type = (sensor_val_type_t)s["val_type"];
            sensors[sensors_n].update_rate = (uint8_t)s["update_rate"];
            sensors[sensors_n].pin = (uint8_t)s["pin"];

            // Set pin as input
            pinMode((uint8_t)s["pin"], INPUT);

            // stores fastest update rate
            if (sensors[sensors_n].update_rate < tmpMinUpdateRate)
            {
                tmpMinUpdateRate = sensors[sensors_n].update_rate;
            }
            sensors_n++;
        }
        this->minUpdateRate = tmpMinUpdateRate;

        Serial.printf("min updrt: %u", minUpdateRate);
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

#endif