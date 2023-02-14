#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "config.h"

#include "datastructs.h"
#include "states.h"
#include <painlessMesh.h>
#include <ArduinoJson.h>

extern node_role_t role;

node_role_t getNodeRole()
{

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

    if (jsonRole.isNull())
    {

        Serial.printf("warning: 'role' not specified in config file. Default value '%d' will be used.\n", NODE_DEFAULT_ROLE);
        role = NODE_DEFAULT_ROLE;
    }
    else
    {
        role = (node_role_t)jsonRole;
    }

    Serial.printf("role: %d\n", role);
    return role;
}

void msgType(char *msg_ptr, uint32_t id)
{

    switch (id)
    {
    case 0:
        strcpy(msg_ptr, "root_ID_request");
        break;
    case 1:
        strcpy(msg_ptr, "root_ID_response");
        break;
    case 2:
        strcpy(msg_ptr, "root_ID_request");
        break;
    case 3:
        strcpy(msg_ptr, "sensor_list_ack");
        break;
    case 8:
        strcpy(msg_ptr, "sensor_value_req");
        break;
    case 9:
        strcpy(msg_ptr, "sensor_value_resp");
        break;
    case 10:
        strcpy(msg_ptr, "keepalive");
        break;
    case 11:
        strcpy(msg_ptr, "keepalive_ack");
        break;
    case 255:
        strcpy(msg_ptr, "slave");
        break;
    default:
        strcpy(msg_ptr, "unkown message");
        break;
    }
}

void stateType(char *msg_ptr, uint32_t id)
{

    if (role == master)
    {
        switch (id)
        {
        case 0:
            strcpy(msg_ptr, "root_ID_request");
            break;
        case 1:
            strcpy(msg_ptr, "root_ID_response");
            break;
        default:
            strcpy(msg_ptr, "unkown message");
            break;
        }
    }
    else if (role == slave)
    {
        switch (id)
        {
        case 0:
            strcpy(msg_ptr, "SS_INIT");
            break;
        case 1:
            strcpy(msg_ptr, "SS_MASTER_REQ");
            break;
        case 2:
            strcpy(msg_ptr, "SS_SENS_ADV");
            break;
        case 3:
            strcpy(msg_ptr, "SS_SENS_UPD");
            break;
        default:
            strcpy(msg_ptr, "unkown message");
            break;
        }
    }
}
#endif
