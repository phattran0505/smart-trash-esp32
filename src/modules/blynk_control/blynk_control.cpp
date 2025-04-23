#include "blynk_control.h" 
#include "../config/config.h"
#include "../smart_trash_pin/smart_trash_pin.h"
#include "../servo_control/servo_control.h"
#include "../ultrasonic/ultrasonic.h"
#include "../display/display.h"
#include <BlynkSimpleEsp32.h>

// Biến theo dõi trạng thái đã thông báo để tránh gửi thông báo liên tục
bool bin1NotificationSent = false;
bool bin2NotificationSent = false;
bool bin3NotificationSent = false;

// Define Blynk Auth Token
#ifndef BLYNK_AUTH_TOKEN
#define BLYNK_AUTH_TOKEN "sjJmhHtVmAkNZiv1NNnEGXULecOTq-5T"
#endif

// Initialize Blynk
void initBlynk() {
  Serial.println("Initializing Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("Blynk initialized");
  
  // Đặt lại trạng thái thông báo khi khởi động
  bin1NotificationSent = false;
  bin2NotificationSent = false;
  bin3NotificationSent = false;
}

// Update Blynk status
void updateBlynk() {
  Blynk.run();
  
  // Kiểm tra và gửi thông báo về tình trạng rác đầy
  checkAndNotifyTrashStatus(bin1Full, bin2Full, bin3Full);
}

// Kiểm tra và gửi thông báo khi thùng rác đầy
void checkAndNotifyTrashStatus(bool bin1Full, bool bin2Full, bool bin3Full) {
  // Thùng 1
  if (bin1Full && !bin1NotificationSent) {
    sendTrashFullNotification(1);
    bin1NotificationSent = true;
    Serial.println("Cảnh báo: Thùng rác 1 đầy! Đã gửi email thông báo.");
  } else if (!bin1Full && bin1NotificationSent) {
    bin1NotificationSent = false; // Đặt lại cờ nếu thùng không còn đầy
  }
  
  // Thùng 2
  if (bin2Full && !bin2NotificationSent) {
    sendTrashFullNotification(2);
    bin2NotificationSent = true;
    Serial.println("Cảnh báo: Thùng rác 2 đầy! Đã gửi email thông báo.");
  } else if (!bin2Full && bin2NotificationSent) {
    bin2NotificationSent = false; // Đặt lại cờ nếu thùng không còn đầy
  }
  
  // Thùng 3
  if (bin3Full && !bin3NotificationSent) {
    sendTrashFullNotification(3);
    bin3NotificationSent = true;
    Serial.println("Cảnh báo: Thùng rác 3 đầy! Đã gửi email thông báo.");
  } else if (!bin3Full && bin3NotificationSent) {
    bin3NotificationSent = false; // Đặt lại cờ nếu thùng không còn đầy
  }
}

// Gửi thông báo qua Blynk LogEvent
void sendTrashFullNotification(int binNumber) {
  char eventMessage[64];
  
  // Tạo thông điệp với thông tin chi tiết
  sprintf(eventMessage, "Thùng rác %d đã đầy!. Cần đổ rác.", binNumber);
  
  // Ghi log và gửi email thông qua Blynk LogEvent
  Blynk.logEvent(EVENT_BIN1_FULL, eventMessage);
  
  // In thông báo ra Serial để debug
  Serial.print("Email notification sent: ");
  Serial.println(eventMessage);
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
    servo1.write(180);
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
    servo2.write(180);
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
    servo3.write(180);
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
    servo1.write(180);
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
    servo2.write(180);
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
    servo3.write(180);
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