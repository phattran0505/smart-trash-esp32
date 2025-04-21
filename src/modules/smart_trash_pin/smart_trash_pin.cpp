#include "smart_trash_pin.h"
#include "../config/config.h"
#include "../display/display.h"
#include "../servo_control/servo_control.h"
#include "../connectivity/connectivity.h"
#include "../blynk_control/blynk_control.h"
#include "../ultrasonic/ultrasonic.h"

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

// Debug variables
unsigned long lastTouchDebug = 0;

// Last update times for various operations
unsigned long lastReconnectAttempt = 0;

void smartTrashInit() 
{
  Serial.begin(9600);
  Serial.println("Starting Smart Trash System...");

  // Initialize display
  initDisplay();

  // Show connecting message immediately
  showConnectingMessage();

  // Initialize WiFi and MQTT
  initConnectivity();

  // Initialize Blynk if WiFi is connected
  if (WiFi.status() == WL_CONNECTED)
  {
    initBlynk();
  }

  // Initialize servos
  Serial.println("Initializing servos...");
  initServos();

  // Initialize ultrasonic sensors
  Serial.println("Initializing ultrasonic sensors...");
  initUltrasonic();

  // Always draw the main interface
  refreshScreen();

  Serial.println("Setup completed!");
}

void smartTrashUpdate()
{
  unsigned long currentTime = millis();

  // Check and maintain connectivity
  updateConnectivity();

  // Update Blynk if WiFi is connected
  if (WiFi.status() == WL_CONNECTED)
  {
    updateBlynk();
  }

  // Update servo positions
  updateServos();

  // Update ultrasonic sensors
  updateUltrasonic();

  // Chỉ cập nhật màn hình khi cần thiết
  if (needDisplayUpdate) {
    displayStatus();
    needDisplayUpdate = false; // Reset the flag
  }

  // Handle touch input
  uint16_t x, y;
  if (tft.getTouch(&x, &y))
  {
    if (currentTime - lastButtonPressTime < DEBOUNCE_DELAY)
    {
      return;
    }
    
    lastButtonPressTime = currentTime;

    // Set the flag to update display when a button is pressed
    needDisplayUpdate = true;
    
    handleButton(x, y, BTN1_X, BTN1_Y, BTN_W, BTN_H, Switch1, servo1ActivationTime, 1);
    handleButton(x, y, BTN2_X, BTN2_Y, BTN_W, BTN_H, Switch2, servo2ActivationTime, 2);
    handleButton(x, y, BTN3_X, BTN3_Y, BTN_W, BTN_H, Switch3, servo3ActivationTime, 3);
  }
}

