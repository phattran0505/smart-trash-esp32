#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <ESP32Servo.h>

// Function declarations
void initServos();
void updateServos();
void handleServo(bool &switchState, unsigned long &activationTime, Servo &servo, int buttonIndex);
void handleButton(int x, int y, int btnX, int btnY, int btnW, int btnH, bool &switchState, unsigned long &activationTime, int buttonIndex);

#endif // SERVO_CONTROL_H 