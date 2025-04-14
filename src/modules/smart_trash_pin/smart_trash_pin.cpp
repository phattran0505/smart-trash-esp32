#include "smart_trash_pin.h"
#include "../config/config.h"
#include "../display/display.h"
#include "../servo_control/servo_control.h"
#include "../connectivity/connectivity.h"

// Initialize components
TFT_eSPI tft = TFT_eSPI();
Servo servo1, servo2, servo3;
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// Status variables
bool Switch1 = false;
bool Switch2 = false;
bool Switch3 = false;
unsigned long servo1ActivationTime = 0;
unsigned long servo2ActivationTime = 0;
unsigned long servo3ActivationTime = 0;
unsigned long lastButtonPressTime = 0;
bool mqttConnected = false;

// Last update times for various operations
unsigned long lastReconnectAttempt = 0;

void smartTrashInit() {
  Serial.begin(115200);
  Serial.println("Starting Smart Trash System...");
  
  // Initialize display
  initDisplay();
  
  // Show connecting message
  showConnectingMessage();

  // Initialize WiFi and MQTT
  initConnectivity();
  
  // Initialize servos
  Serial.println("Initializing servos...");
  initServos();

  // Always draw the main interface
  refreshScreen();
  
  Serial.println("Setup completed!");
}

void smartTrashUpdate() {
  unsigned long currentTime = millis();
  
  // Check and maintain connectivity
  updateConnectivity();
  
  // Update servo positions
  updateServos();
  
  // Handle touch input
  uint16_t x, y;
  if (tft.getTouch(&x, &y)) {
    if (currentTime - lastButtonPressTime < DEBOUNCE_DELAY) {
      return;
    }
    
    lastButtonPressTime = currentTime;
    
    handleButton(x, y, BTN1_X, BTN1_Y, BTN_W, BTN_H, Switch1, servo1ActivationTime, 1);
    handleButton(x, y, BTN2_X, BTN2_Y, BTN_W, BTN_H, Switch2, servo2ActivationTime, 2);
    handleButton(x, y, BTN3_X, BTN3_Y, BTN_W, BTN_H, Switch3, servo3ActivationTime, 3);
  }
}
