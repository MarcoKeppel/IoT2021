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

#endif
