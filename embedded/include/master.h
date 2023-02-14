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
    bool freeslots[M_MAX_SLAVES_N] = {true};

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
                if (slaves[i].keepalive_counter <= slaves[i].keepalive_period)
                {
                    Serial.printf("slave n: %u kc: %u kp: %u", i, slaves[i].keepalive_counter, slaves[i].keepalive_period);
                }
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
        int8_t n_slaves = getNSlaves();
        if (n_slaves < M_MAX_SLAVES_N)
        {
            int8_t freeslot = getFirstFreeSlot();
            this->freeslots[freeslot] = false;
            slaves[freeslot].addr = addr;

            // TODO slave should send other parameters such as name, either here (master req) or in the sensor adv message

            Serial.printf("New slave added to list: \n\taddr: %u\n#slaves: %d\n", addr, n_slaves);
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

        for (int i = 0; i < M_MAX_SLAVES_N && !freeslots[i]; i++)
        {

            if (slaves[i].addr == addr)
            {

                JsonArrayConst sensors = msg["sensors"].as<JsonArrayConst>();
                slaves[i].keepalive_period = msg["min_update_rate"];
                slaves[i].keepalive_counter = slaves[i].keepalive_period;
                for (JsonObjectConst s : sensors)
                {

                    if (slaves[i].n_sensors > M_MAX_SLAVE_SENSORS_N)
                        break;

                    sensor_t *sensor = &(slaves[i].sensors[slaves[i].n_sensors]);

                    // TODO: sensor->name
                    sensor->type = s["type"];
                    sensor->val_type = s["val_type"];
                    sensor->update_rate = s["update_rate"];

                    slaves[i].n_sensors++;
                }
                Serial.printf("SLAVE KEEPALIVE: %u", slaves[i].keepalive_period);
                break;
            }
        }
    }

    void updateSensorValues(uint32_t addr, const JsonDocument &msg)
    {
        uint8_t n_slaves = getNSlaves();
        for (int i = 0; i < n_slaves; i++)
        {

            if (slaves[i].addr == addr)
            {

                // TODO: update sensors values
            }
        }
    }

} master_data_t;

#endif