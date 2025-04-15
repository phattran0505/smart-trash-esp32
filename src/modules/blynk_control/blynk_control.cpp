// Define Blynk settings before including BlynkSimpleEsp32.h
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6myBfv59F"
#define BLYNK_TEMPLATE_NAME "smart trash pin"
#define BLYNK_AUTH_TOKEN "sjJmhHtVmAkNZiv1NNnEGXULecOTq-5T"

#include "blynk_control.h" 
#include "../smart_trash_pin/smart_trash_pin.h"
#include "../servo_control/servo_control.h"
#include "../display/display.h"
#include <BlynkSimpleEsp32.h>

// Initialize Blynk
void initBlynk() {
  Serial.println("Initializing Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("Blynk initialized");
}

// Update Blynk status
void updateBlynk() {
  Blynk.run();
}

// Update Blynk UI with current servo status
void updateBlynkStatus() {
  Blynk.virtualWrite(V1, Switch1 ? 1 : 0);
  Blynk.virtualWrite(V2, Switch2 ? 1 : 0);
  Blynk.virtualWrite(V3, Switch3 ? 1 : 0);
}

// Handle virtual pin writes from other code
void blynkWriteV1(int value) {
  if (value == 1) {
    // Turn on servo 1
    Switch1 = true;
    servo1.write(90);
    servo1ActivationTime = millis();
    updateButton(1, Switch1);
    Serial.println("Local code: Servo 1 ON (auto-off in 5s)");
  } else {
    // Turn off servo 1
    Switch1 = false;
    servo1.write(0);
    servo1ActivationTime = 0;
    updateButton(1, Switch1);
    Serial.println("Local code: Servo 1 OFF");
  }
}

void blynkWriteV2(int value) {
  if (value == 1) {
    // Turn on servo 2
    Switch2 = true;
    servo2.write(90);
    servo2ActivationTime = millis();
    updateButton(2, Switch2);
    Serial.println("Local code: Servo 2 ON (auto-off in 5s)");
  } else {
    // Turn off servo 2
    Switch2 = false;
    servo2.write(0);
    servo2ActivationTime = 0;
    updateButton(2, Switch2);
    Serial.println("Local code: Servo 2 OFF");
  }
}

void blynkWriteV3(int value) {
  if (value == 1) {
    // Turn on servo 3
    Switch3 = true;
    servo3.write(90);
    servo3ActivationTime = millis();
    updateButton(3, Switch3);
    Serial.println("Local code: Servo 3 ON (auto-off in 5s)");
  } else {
    // Turn off servo 3
    Switch3 = false;
    servo3.write(0);
    servo3ActivationTime = 0;
    updateButton(3, Switch3);
    Serial.println("Local code: Servo 3 OFF");
  }
}

// SERVO 1 control via V1
BLYNK_WRITE(V1) {
  int value = param.asInt();
  
  if (value == 1) {
    // Turn ON servo 1
    Switch1 = true;
    servo1.write(90);
    servo1ActivationTime = millis(); // Start activation timer
    Serial.println("Blynk: Servo 1 ON (auto-off in 5s)");
  } else {
    // Turn OFF servo 1
    Switch1 = false;
    servo1.write(0);
    servo1ActivationTime = 0;
    Serial.println("Blynk: Servo 1 OFF");
  }
  
  // Update interface
  updateButton(1, Switch1);
}

// SERVO 2 control via V2
BLYNK_WRITE(V2) {
  int value = param.asInt();
  
  if (value == 1) {
    // Turn ON servo 2
    Switch2 = true;
    servo2.write(90);
    servo2ActivationTime = millis(); // Start activation timer
    Serial.println("Blynk: Servo 2 ON (auto-off in 5s)");
  } else {
    // Turn OFF servo 2
    Switch2 = false;
    servo2.write(0);
    servo2ActivationTime = 0;
    Serial.println("Blynk: Servo 2 OFF");
  }
  
  // Update interface
  updateButton(2, Switch2);
}

// SERVO 3 control via V3
BLYNK_WRITE(V3) {
  int value = param.asInt();
  
  if (value == 1) {
    // Turn ON servo 3
    Switch3 = true;
    servo3.write(90);
    servo3ActivationTime = millis(); // Start activation timer
    Serial.println("Blynk: Servo 3 ON (auto-off in 5s)");
  } else {
    // Turn OFF servo 3
    Switch3 = false;
    servo3.write(0);
    servo3ActivationTime = 0;
    Serial.println("Blynk: Servo 3 OFF");
  }
  
  // Update interface
  updateButton(3, Switch3);
} 