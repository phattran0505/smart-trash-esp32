#include "servo_control.h"
#include "../config/config.h"
#include "../smart_trash_pin/smart_trash_pin.h"
#include "../display/display.h"
#include "../blynk_control/blynk_control.h"

void initServos() {
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);

  // Set initial positions
  servo1.write(0);
  servo2.write(0);
  servo3.write(0);
}

// Hàm xử lý servo chung
void handleServo(bool &switchState, unsigned long &activationTime, Servo &servo, int buttonIndex) {
  unsigned long currentMillis = millis();
  
  // Kiểm tra thời gian reset
  if (switchState && activationTime > 0 && (currentMillis - activationTime >= SERVO_RESET_TIME)) {
    switchState = false;
    servo.write(0); // Move servo back to 0 position
    activationTime = 0;
    
    // Update interface
    updateButton(buttonIndex, switchState);
    
    // Update Blynk UI
    updateBlynkStatus();
    
    Serial.printf("Servo %d: Auto-OFF after 5s\n", buttonIndex);
  }
}

// Hàm xử lý nút nhấn chung
// Hàm xử lý nút nhấn chung
void handleButton(int x, int y, int btnX, int btnY, int btnW, int btnH, 
                 bool &switchState, unsigned long &activationTime, 
                 int buttonIndex) {

  if (x > btnX && x < btnX + btnW && y > btnY && y < btnY + btnH) {
    Serial.printf("Button %d pressed!\n", buttonIndex);
    switchState = !switchState;
    
    // Get the correct servo based on buttonIndex
    Servo* currentServo;
    switch (buttonIndex) {
      case 1:
        currentServo = &servo1;
        break;
      case 2:
        currentServo = &servo2;
        break;
      case 3:
        currentServo = &servo3;
        break;
      default:
        return;
    }
    
    if (switchState) {
      currentServo->write(180);  // Move servo to ON position (changed from 90 to 180 degrees)
      activationTime = millis();  // Start tracking activation time
      Serial.printf("Touch: Servo %d ON\n", buttonIndex);
    } else {
      currentServo->write(0);   // Move servo to OFF position
      activationTime = 0;
      Serial.printf("Touch: Servo %d OFF\n", buttonIndex);
    }
    
    // Update interface
    updateButton(buttonIndex, switchState);
    
    // Update Blynk UI
    updateBlynkStatus();
  }
}

// Update all servos
void updateServos() {
  handleServo(Switch1, servo1ActivationTime, servo1, 1);
  handleServo(Switch2, servo2ActivationTime, servo2, 2);
  handleServo(Switch3, servo3ActivationTime, servo3, 3);
} 