#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// Function declarations
void initConnectivity();
void updateConnectivity();
boolean reconnectMQTT();
void mqttCallback(char *topic, byte *payload, unsigned int length);

#endif // CONNECTIVITY_H 