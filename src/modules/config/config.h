#ifndef CONFIG_H
#define CONFIG_H

 
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6myBfv59F"
#define BLYNK_TEMPLATE_NAME "smart trash pin"
#define BLYNK_AUTH_TOKEN "sjJmhHtVmAkNZiv1NNnEGXULecOTq-5T"


// WiFi and MQTT configuration
extern const char *ssid;
extern const char *password;
extern const char *mqtt_broker;
extern const char *mqtt_username;
extern const char *mqtt_password;
extern const char *mqtt_topic;
extern const int mqtt_port;

// Timing constants
#define SERVO_RESET_TIME 5000
#define DEBOUNCE_DELAY 300
#define SERVO_SPEED 5
#define SERVO_UPDATE_INTERVAL 15
#define RECONNECT_INTERVAL 5000

// UI constants
#define CALIBRATION_FILE "/TouchCalData3"
#define REPEAT_CAL false

// Button sizes and positions
#define BTN_W 90
#define BTN_H 60
#define BTN_SPACING 25

#define BTN1_X 0
#define BTN1_Y 50

#define BTN2_X (BTN1_X + BTN_W + BTN_SPACING)
#define BTN2_Y BTN1_Y

#define BTN3_X (BTN2_X + BTN_W + BTN_SPACING)
#define BTN3_Y BTN1_Y

// Servo pin definitions
#define SERVO1_PIN 12
#define SERVO2_PIN 13
#define SERVO3_PIN 14

#endif // CONFIG_H 