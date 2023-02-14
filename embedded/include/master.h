#ifndef __MASTER_H__
#define __MASTER_H__

#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "config.h"

#include "datastructs.h"
#include "states.h"
#include <painlessMesh.h>
#include <ArduinoJson.h>

typedef struct master_data
{

    char *name;
    slave_t slaves[M_MAX_SLAVES_N];
    bool freeslots[M_MAX_SLAVES_N];

    uint8_t state;
    uint32_t currentMillis = 0;
    uint32_t lastSensorsUpdateMillis = 0;
    uint32_t lastBroadcastMillis = 0;
    uint32_t lastKeepaliveMillis = 0;
    painlessMesh *mesh;

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
            for (int i = 0; i < M_MAX_SLAVES_N && !freeslots[i]; i++)
            {

                slaves[i].keepalive_counter--;
                if (slaves[i].keepalive_counter <= 0)
                {
                    slaves[i].keepalive_counter = slaves[i].keepalive_period;
                }
                Serial.printf("slave n: %u kc: %u kp: %u\n\r", i, slaves[i].keepalive_counter, slaves[i].keepalive_period);
            }
        }
    }

    void onReceive(uint32_t from, const JsonDocument &msg)
    {

        uint8_t type = (uint8_t)msg["type"];

        Serial.printf("Message:\n\tfrom: %u\n\ttype: %u\n", from, type);

        switch (type)
        {

        case 0: // TODO check if new slaves can be added, return error if not (protocol to be defined)
            addSlave(from);
            sendMasterAddrResp(from);
            break;

        case 2:
            addSlaveSensors(from, msg); // TODO: return status (e.g. if slave does not exist, do not return ack and return error instead (protocol to be defined))
            sendSensorListAck(from);
            break;
        case 9:
            updateSensorValues(from, msg);
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

    int findSlave(uint32_t addr, bool only_active = true)
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

        StaticJsonDocument<512> msg; // TODO: define size as macro

        msg["id"] = mesh->getNodeId();
        msg["type"] = 3; // TODO: define types more formally

        char msgSerialized[256]; // TODO: define size as macro
        serializeJson(msg, msgSerialized);
        mesh->sendSingle(dest, msgSerialized);
    }

    void addSlave(uint32_t addr)
    {
        uint8_t n_slaves = getNSlaves();
        if (n_slaves < M_MAX_SLAVES_N)
        {
            int8_t freeslot = getFirstFreeSlot();
            this->freeslots[freeslot] = false;
            slaves[freeslot].addr = addr;

            // TODO slave should send other parameters such as name, either here (master req) or in the sensor adv message

            Serial.printf("New slave added to list: \n\taddr: %u\n#slaves: %u\n", addr, n_slaves + 1);
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

        uint32_t s = findSlave(addr);
        if (s < 0)
            return;

        JsonArrayConst sensors = msg["sensors"].as<JsonArrayConst>();
        slaves[s].keepalive_period = msg["min_update_rate"];
        slaves[s].keepalive_counter = slaves[s].keepalive_period;
        for (JsonObjectConst sen : sensors)
        {

            if (slaves[s].n_sensors > M_MAX_SLAVE_SENSORS_N)
                break;

            sensor_t *sensor = &(slaves[s].sensors[slaves[s].n_sensors]);

            // TODO: sensor->name
            sensor->type = sen["type"];
            sensor->val_type = sen["val_type"];
            sensor->update_rate = sen["update_rate"];

            slaves[s].n_sensors++;
        }
        Serial.printf("SLAVE KEEPALIVE: %u", slaves[s].keepalive_period);
    }

    void updateSensorValues(uint32_t addr, const JsonDocument &msg)
    {
        uint32_t s = findSlave(addr);
        if (s < 0)
            return;

        // Update sensors values
        JsonArrayConst sensors = msg["sensors"].as<JsonArrayConst>();
        for (JsonObjectConst sensor : sensors)
        {
            slaves[s].sensors[(uint32_t)sensor["index"]].val = (uint32_t)sensor["val"]; // TODO: should be more generic than int
        }
    }

} master_data_t;

#endif