#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

char* configFile = "config.json";     // Config JSON filename (stored in FS)

void setup() {
  
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
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
    Serial.printf("error: could not open config file ()\n", configFile);
  }

  StaticJsonDocument<512> configJson;
  deserializeJson(configJson, f.readString());

  Serial.println((const char*) configJson["name"]);

}
