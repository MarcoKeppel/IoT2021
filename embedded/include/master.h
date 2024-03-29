#ifndef __MASTER_H__
#define __MASTER_H__

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

typedef struct master_data
{

    char name[MASTER_NAME_LEN];
    slave_t slaves[M_MAX_SLAVES_N];
    bool freeslots[M_MAX_SLAVES_N];

    uint8_t state;
    uint32_t currentMillis = 0;
    uint32_t lastSensorsUpdateMillis = 0;
    uint32_t lastBroadcastMillis = 0;
    uint32_t lastKeepaliveMillis = 0;
    painlessMesh *mesh;

    char msg_str[100];
    char state_str[100];
    master_data(painlessMesh *mesh)
    {

        this->mesh = mesh;

        this->state = MS_INIT;
    }

    void masterSetup()
    {
        for (int i = 0; i < M_MAX_SLAVES_N; i++)
        {
            freeslots[i] = true;
        }
    }

    void masterLoop()
    {
        currentMillis = millis();
        // sendSensorListAdv();

        if (currentMillis >= lastKeepaliveMillis + KEEPALIVE_PERIOD)
        {
            lastKeepaliveMillis = currentMillis;
            for (int i = 0; i < M_MAX_SLAVES_N; i++)
            {
                if (freeslots[i])
                    continue;

                if (slaves[i].keepalive_period != -1 && slaves[i].is_ready == true)
                {

                    if (slaves[i].keepalive_counter <= 0)
                    {
                        slaves[i].kill_countdown--;
                        if (slaves[i].kill_countdown <= 0)
                        {
                            // / Serial.printf("UCCIDI LO SLAVE %u \n\r", i);
                            killSlave(i);
                        }
                        sendKeepalive(slaves[i].addr);
                    }
                    else
                    {
                        slaves[i].keepalive_counter--;
                    }
                }
                else
                {
                    // / Serial.printf("slave %u is not ready yet \n\r", i);
                }

                // Serial.printf("kc: %d kill_in: %d\n\r", slaves[i].keepalive_counter, slaves[i].kill_countdown);
            }
        }
    }

    void onReceive(uint32_t from, const String &msg_serialized)
    {

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;
        deserializeJson(msg, msg_serialized);

        uint8_t type = (uint8_t)msg["type"];
        msgType(msg_str, type);
        // Serial.printf("msgtype: %s, from %u\n\r", msg_str, from);
        Serial.printf("{\"from\":%u,\"msg\":%s}\n", from, msg_serialized.c_str());

        if (findSlave(from) == -1 && type != MSG_ROOT_ID_REQ)
        {
            // / Serial.printf("slave not identified, sending reset");
            sendSlaveReset(from);
            return;
        }

        switch (type)
        {

        case MSG_ROOT_ID_REQ: // TODO check if new slaves can be added, return error if not (protocol to be defined)
            addSlave(from, msg);
            sendMasterAddrResp(from);
            break;

        case MSG_SENSOR_LIST_ADV:
            addSlaveSensors(from, msg); // TODO: return status (e.g. if slave does not exist, do not return ack and return error instead (protocol to be defined))
            sendSensorListAck(from);
            break;
        case MSG_SENSOR_VALUE_RESP:
            updateSensorValues(from, msg);
            break;
        case MSG_KEEPALIVE_ACK:
            slaveIsAlive(from);
            break;
        }
    }

    void killSlave(uint32_t i)
    {
        freeslots[i] = true;
        slaves[i].is_ready = false;

        Serial.printf("KILLED SLAVE %u\n", slaves[i].addr);
        Serial.printf("{\"from\":%u,\"msg\":{\"type\":\"KILLED\"}}\n", slaves[i].addr);
    }

    void sendSlaveReset(uint32_t dest)
    {

        int32_t i = findSlave(dest);
        if (i != -1)
        {
            slaves[i].is_ready = false;
        }

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;

        msg["id"] = mesh->getNodeId();
        msg["type"] = MSG_SLAVE_RESET;

        char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(dest, msgSerialized);

        Serial.printf("RESET SLAVE %u\n", dest);
    }

    void slaveIsAlive(uint32_t addr)
    {
        int32_t s = findSlave(addr);
        if (s != -1)
        {
            slaves[s].kill_countdown = KEEPALIVE_KILL_PERIODS;
            slaves[s].keepalive_counter = slaves[s].keepalive_period;
        }

        // Serial.printf("slave %u with address %u saved\n\r", s, addr);
    }

    void sendMasterAddrResp(uint32_t dest)
    {

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;

        msg["id"] = mesh->getNodeId();
        msg["type"] = MSG_ROOT_ID_RESP;

        char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(dest, msgSerialized);
    }

    void sendKeepalive(uint32_t dest)
    {

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;

        msg["id"] = mesh->getNodeId();
        msg["type"] = MSG_KEEPALIVE;

        char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(dest, msgSerialized);
    }

    int32_t findSlave(uint32_t addr, bool only_active = true)
    {

        for (int i = 0; i < M_MAX_SLAVES_N; i++)
        {

            if (slaves[i].addr == addr && ((only_active && !freeslots[i]) || (!only_active)))
            {

                return i;
            }
        }

        return -1;
    }

    void sendSensorListAck(uint32_t dest)
    {

        StaticJsonDocument<STATIC_JSON_DOC_SIZE> msg;

        msg["id"] = mesh->getNodeId();
        msg["type"] = MSG_SENSOR_LIST_ACK;

        char msgSerialized[SERIALIZED_JSON_MSG_SIZE];
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(dest, msgSerialized);
    }

    void addSlave(uint32_t addr, const JsonDocument &msg)
    {
        int32_t s = findSlave(addr, false);

        // If the slave is already saved in memory
        if (s >= 0)
        {

            // / Serial.printf("Found slave at index %d\n", s);

            freeslots[s] = false;
        }
        // If it is not already saved in memory
        else
        {

            // / Serial.printf("Slave not found\n");

            uint8_t n_slaves = getNSlaves();
            if (n_slaves < M_MAX_SLAVES_N)
            {
                int8_t freeslot = getFirstFreeSlot();
                this->freeslots[freeslot] = false;
                slaves[freeslot].addr = addr;
                strcpy(slaves[freeslot].name, (const char *)msg["name"]);

                // / Serial.printf("New slave added to list: \n\taddr: %u\n#slaves: %u\n", addr, n_slaves + 1);
            }
        }
    }

    int8_t getFirstFreeSlot()
    {
        int8_t freeslot = -1;
        for (int i = 0; i < M_MAX_SLAVES_N; i++)
        {
            if (freeslots[i])
            {
                freeslot = i;
                break;
            }
        }
        return freeslot;
    }

    uint8_t getNSlaves()
    {
        uint8_t nslaves = 0;
        for (int i = 0; i < M_MAX_SLAVES_N; i++)
        {
            if (!freeslots[i])
            {
                nslaves++;
            }
        }

        return nslaves;
    }

    void addSlaveSensors(uint32_t addr, const JsonDocument &msg)
    {

        int32_t s = findSlave(addr);
        if (s < 0)
            return;

        JsonArrayConst sensors = msg["sensors"].as<JsonArrayConst>();
        slaves[s].keepalive_period = msg["min_update_rate"];
        slaves[s].keepalive_counter = slaves[s].keepalive_period;
        slaves[s].kill_countdown = KEEPALIVE_KILL_PERIODS;
        for (JsonObjectConst sen : sensors)
        {

            if (slaves[s].n_sensors > M_MAX_SLAVE_SENSORS_N)
                break;

            sensor_t *sensor = &(slaves[s].sensors[slaves[s].n_sensors]);

            strcpy(sensor->name, (const char *)sen["name"]);
            sensor->type = sen["type"];
            sensor->val_type = sen["val_type"];
            sensor->update_rate = sen["update_rate"];

            slaves[s].n_sensors++;
        }

        slaves[s].is_ready = true;
        // / Serial.printf("SLAVE KEEPALIVE: %u", slaves[s].keepalive_period);
    }

    void updateSensorValues(uint32_t addr, const JsonDocument &msg)
    {

        int32_t s = findSlave(addr);
        if (s < 0)
            return;

        // Update sensors values
        JsonArrayConst sensors = msg["sensors"].as<JsonArrayConst>();
        for (JsonObjectConst sensor : sensors)
        {
            uint32_t ind = (uint32_t)sensor["index"];
            switch (slaves[s].sensors[ind].val_type)
            {

            case v_int:
                slaves[s].sensors[ind].val.i = (int32_t)sensor["val"];
                break;

            case v_uint:
                slaves[s].sensors[ind].val.u = (uint32_t)sensor["val"];
                break;

            case v_real:
                slaves[s].sensors[ind].val.f = (float_t)sensor["val"];

            case v_bool:
                slaves[s].sensors[ind].val.b = (bool)sensor["val"];
            }
        }
    }

    void sendSerialRecap()
    {

        digitalWrite(LED_BUILTIN, 1);

        for (int i = 0; i < M_MAX_SLAVES_N; i++)
        {
            if (freeslots[i])
                continue;

            StaticJsonDocument<STATIC_JSON_DOC_SIZE * 2> top_msg;

            top_msg["from"] = slaves[i].addr;

            JsonObject msg = top_msg.createNestedObject("msg");

            msg["id"] = slaves[i].addr;
            msg["type"] = MSG_ROOT_ID_REQ;
            msg["name"] = this->name;

            char msgSerialized[SERIALIZED_JSON_MSG_SIZE * 2];
            serializeJson(top_msg, msgSerialized);

            Serial.println(msgSerialized);

            yield();
        }

        for (int i = 0; i < M_MAX_SLAVES_N; i++)
        {
            if (freeslots[i])
                continue;

            char msgSerialized[SERIALIZED_JSON_MSG_SIZE * 2];
            StaticJsonDocument<STATIC_JSON_DOC_SIZE * 2> top_msg;

            top_msg["from"] = slaves[i].addr;

            JsonObject msg = top_msg.createNestedObject("msg");

            msg["type"] = MSG_SENSOR_LIST_ADV;
            msg["min_update_rate"] = 10; // TODO

            JsonArray sensorsArray = msg.createNestedArray("sensors");

            // ...then cycle through all sensors and add those that need to be updated
            for (int j = 0; j < slaves[i].n_sensors; j++)
            {

                // Add data to message
                JsonObject sensorObject = sensorsArray.createNestedObject();
                sensorObject["name"] = slaves[i].sensors[j].name;
                sensorObject["type"] = slaves[i].sensors[j].type;
                sensorObject["val_type"] = slaves[i].sensors[j].val_type;
                sensorObject["update_rate"] = slaves[i].sensors[j].update_rate;
            }

            serializeJson(top_msg, msgSerialized);

            Serial.println(msgSerialized);

            yield();
        }
    }

} master_data_t;

#endif