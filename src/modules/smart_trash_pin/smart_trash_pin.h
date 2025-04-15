#ifndef SMART_TRASH_PIN_H
#define SMART_TRASH_PIN_H

#include "../config/config.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// Function declarations
void smartTrashInit();
void smartTrashUpdate();

// Expose necessary components
extern TFT_eSPI tft;
extern Servo servo1, servo2, servo3;
extern PubSubClient mqttClient;
extern WiFiClientSecure secureClient;

// Status variables
extern bool Switch1, Switch2, Switch3;
extern unsigned long servo1ActivationTime, servo2ActivationTime, servo3ActivationTime;
extern bool mqttConnected;

#endif // SMART_TRASH_PIN_H
