#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>
#include <LittleFS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "config.h"

#include "datastructs.h"
#include "states.h"
#include "protocol.h"
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
    case MSG_ROOT_ID_REQ:
        strcpy(msg_ptr, "root_ID_request");
        break;
    case MSG_ROOT_ID_RESP:
        strcpy(msg_ptr, "root_ID_response");
        break;
    case MSG_SENSOR_LIST_ADV:
        strcpy(msg_ptr, "sensor_list_adv");
        break;
    case MSG_SENSOR_LIST_ACK:
        strcpy(msg_ptr, "sensor_list_ack");
        break;
    case MSG_SENSOR_VALUE_REQ:
        strcpy(msg_ptr, "sensor_value_req");
        break;
    case MSG_SENSOR_VALUE_RESP:
        strcpy(msg_ptr, "sensor_value_resp");
        break;
    case MSG_KEEPALIVE:
        strcpy(msg_ptr, "keepalive");
        break;
    case MSG_KEEPALIVE_ACK:
        strcpy(msg_ptr, "keepalive_ack");
        break;
    case MSG_SLAVE_RESET:
        strcpy(msg_ptr, "slave reset");
        break;
    default:
        strcpy(msg_ptr, "unknown message");
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
