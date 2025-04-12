#include <Arduino.h>
#include "FS.h"
#include "SPI.h"
#include <TFT_eSPI.h>   // Thư viện màn hình TFT
#include <ESP32Servo.h> // Thư viện điều khiển Servo
#include <WiFi.h>
#include <WiFiClientSecure.h> // Thêm WiFiClientSecure để hỗ trợ TLS
#include <ArduinoJson.h>
#include <PubSubClient.h>

// WiFi và MQTT configuration
const char *ssid = "AEPTIT";
const char *password = "20242024";
const char *mqtt_broker = "x2b4961d.ala.us-east-1.emqxsl.com";
const char *mqtt_username = "phattran";
const char *mqtt_password = "5ShX9GjPkZ9MHbs";
const char *mqtt_topic = "trash_classification/result";
const int mqtt_port = 8883;

WiFiClientSecure secureClient; // Sử dụng WiFiClientSecure thay vì WiFiClient thông thường
PubSubClient mqttClient(secureClient);
TFT_eSPI tft = TFT_eSPI();    // Khởi tạo màn hình
Servo servo1, servo2, servo3; // Đối tượng điều khiển Servo

// Thời gian để reset servo (5 giây = 5000ms)
#define SERVO_RESET_TIME 5000
unsigned long servo1ActivationTime = 0;
unsigned long servo2ActivationTime = 0;
unsigned long servo3ActivationTime = 0;

// Thời gian chống dội cho nút (300ms)
#define DEBOUNCE_DELAY 300
unsigned long lastButtonPressTime = 0;

#define CALIBRATION_FILE "/TouchCalData3"
#define REPEAT_CAL false

// Kích thước và vị trí nút
#define BTN_W 90
#define BTN_H 60
#define BTN_SPACING 25

#define BTN1_X 0
#define BTN1_Y 50

#define BTN2_X (BTN1_X + BTN_W + BTN_SPACING)
#define BTN2_Y BTN1_Y

#define BTN3_X (BTN2_X + BTN_W + BTN_SPACING)
#define BTN3_Y BTN1_Y

// Chân kết nối servo
#define SERVO1_PIN 12
#define SERVO2_PIN 13
#define SERVO3_PIN 14

// Trạng thái servo
bool Switch1 = false;
bool Switch2 = false;
bool Switch3 = false;

// Biến lưu vị trí hiện tại của servo
int currentPos1 = 0;
int currentPos2 = 0;
int currentPos3 = 0;

// Biến lưu vị trí đích của servo
int targetPos1 = 0;
int targetPos2 = 0;
int targetPos3 = 0;

// Thời gian cuối cùng cập nhật vị trí servo
unsigned long lastServoUpdate = 0;
// Tốc độ di chuyển servo (mỗi 15ms di chuyển 5 độ)
#define SERVO_SPEED 5
#define SERVO_UPDATE_INTERVAL 15

// Biến theo dõi tình trạng kết nối MQTT
bool mqttConnected = false;
unsigned long lastReconnectAttempt = 0;
#define RECONNECT_INTERVAL 5000

// Hiệu chỉnh cảm ứng
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  if (!SPIFFS.begin())
  {
    Serial.println("Định dạng lại hệ thống file");
    SPIFFS.format();
    SPIFFS.begin();
  }

  if (SPIFFS.exists(CALIBRATION_FILE))
  {
    if (REPEAT_CAL)
    {
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f)
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL)
  {
    tft.setTouch(calData);
  }
  else
  {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Chạm vào các góc để hiệu chỉnh");

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

// Hiệu ứng chuyển màn hình - Wipe transition
void screenTransition()
{
  // Hiệu ứng wipe từ phải sang trái
  for (int x = 0; x <= tft.width(); x += 8)
  {
    tft.fillRect(tft.width() - x, 0, tft.width(), tft.height(), TFT_BLACK);
    delay(5); // Tốc độ hiệu ứng
  }

  delay(200); // Tạm dừng ngắn

  // Hiệu ứng wipe từ dưới lên
  for (int y = 0; y <= tft.height(); y += 6)
  {
    tft.fillRect(0, tft.height() - y, tft.width(), tft.height(), TFT_BLUE);
    delay(5); // Tốc độ hiệu ứng
  }
}

// Vẽ nút ON/OFF
void drawButton(int x, int y, bool state, const char *label)
{
  if (state)
  {
    tft.fillRect(x, y, BTN_W, BTN_H, TFT_GREEN);
  }
  else
  {
    tft.fillRect(x, y, BTN_W, BTN_H, TFT_RED);
  }

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + (BTN_W / 2), y + 15);
  tft.drawString(state ? "ON" : "OFF", x + (BTN_W / 2), y + 40);
  tft.drawRect(x, y, BTN_W, BTN_H, TFT_BLACK);
}

// Hiển thị trạng thái Servo
void displayStatus()
{
  // Chỉ xóa vùng hiển thị trạng thái ở phía dưới
  tft.fillRect(0, 130, tft.width(), 30, TFT_BLUE); // Xóa toàn bộ hàng chữ

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);

  String status = "";
  int count = 0;

  if (Switch1)
  {
    status += "Servo 1";
    count++;
  }
  if (Switch2)
  {
    if (count > 0)
      status += " va ";
    status += "Servo 2";
    count++;
  }
  if (Switch3)
  {
    if (count > 0)
      status += " va ";
    status += "Servo 3";
    count++;
  }

  if (count == 0)
  {
    tft.drawString("Ca ba Servo deu OFF", 160, 145);
  }
  else if (count == 3)
  {
    tft.drawString("Ca ba Servo deu ON", 160, 145);
  }
  else
  {
    tft.drawString(status + " dang ON", 160, 145);
  }
}

// Cập nhật nút cụ thể và trạng thái thay vì vẽ lại toàn bộ màn hình
void updateButton(int buttonIndex, bool state)
{
  int x, y;
  const char *label;

  switch (buttonIndex)
  {
  case 1:
    x = BTN1_X;
    y = BTN1_Y;
    label = "Servo 1";
    break;
  case 2:
    x = BTN2_X;
    y = BTN2_Y;
    label = "Servo 2";
    break;
  case 3:
    x = BTN3_X;
    y = BTN3_Y;
    label = "Servo 3";
    break;
  default:
    return;
  }

  // Chỉ vẽ lại nút cụ thể
  drawButton(x, y, state, label);

  // Cập nhật trạng thái
  displayStatus();
}

// Hiển thị trạng thái kết nối
void updateConnectionStatus()
{
  tft.fillRect(0, 0, tft.width(), 30, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  
  if (WiFi.status() == WL_CONNECTED)
  {
    tft.print("WiFi: ");
    tft.print(WiFi.localIP());
    
    // Hiển thị trạng thái MQTT
    tft.setCursor(10, 20);
    if (mqttConnected)
    {
      tft.print("MQTT: Connected");
    }
    else
    {
      tft.setTextColor(TFT_RED);
      tft.print("MQTT: Disconnected");
    }
  }
  else
  {
    tft.setTextColor(TFT_RED);
    tft.print("WiFi: Disconnected");
  }
}

// Hàm vẽ lại toàn bộ giao diện
void refreshScreen()
{
  // Xóa toàn bộ màn hình
  tft.fillScreen(TFT_BLUE);

  // Vẽ lại thông tin kết nối ở trên cùng
  updateConnectionStatus();

  // Vẽ lại các nút
  drawButton(BTN1_X, BTN1_Y, Switch1, "Servo 1");
  drawButton(BTN2_X, BTN2_Y, Switch2, "Servo 2");
  drawButton(BTN3_X, BTN3_Y, Switch3, "Servo 3");

  // Hiển thị trạng thái
  displayStatus();
}

// Hàm xử lý khi nhận được tin nhắn MQTT
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  // Chuyển payload thành chuỗi
  String message;
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  
  // Phân tích JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (!error)
  {
    if (doc.containsKey("class"))
    {
      String className = doc["class"].as<String>();
      Serial.print("Class detected: ");
      Serial.println(className);
      
      // Reset vị trí hiện tại về 0
      currentPos1 = 0;
      currentPos2 = 0;
      currentPos3 = 0;
      
      // Kiểm tra class và điều khiển servo tương ứng
      if (className == "plastic")
      {
        Switch1 = true;
        targetPos1 = 90;
        servo1ActivationTime = millis();
        servo1.write(90);
        updateButton(1, Switch1);
      }
      else if (className == "glass")
      {
        Switch2 = true;
        targetPos2 = 90;
        servo2ActivationTime = millis();
        servo2.write(90);
        updateButton(2, Switch2);
      }
      else if (className == "common")
      {
        Switch3 = true;
        targetPos3 = 90;
        servo3ActivationTime = millis();
        servo3.write(90);
        updateButton(3, Switch3);
      }
    }
  }
}

// Kết nối lại MQTT nếu bị mất kết nối
boolean reconnectMQTT()
{
  Serial.print("Connecting to MQTT...");
  
  // Tạo client ID dựa trên địa chỉ MAC
  String clientId = "ESP32_SmartTrash-" + String(WiFi.macAddress());
  
  // Thử kết nối với thông tin xác thực
  if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password))
  {
    Serial.println("connected");
    mqttClient.subscribe(mqtt_topic);
    mqttConnected = true;
    updateConnectionStatus();
    return true;
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(mqttClient.state());
    mqttConnected = false;
    updateConnectionStatus();
    return false;
  }
}

// Hàm xử lý servo chung
void handleServo(int &currentPos, int &targetPos, bool &switchState, unsigned long &activationTime, Servo &servo, int buttonIndex)
{
  unsigned long currentMillis = millis();
  
  // Cập nhật vị trí servo
  if (currentPos < targetPos)
  {
    currentPos = min(currentPos + SERVO_SPEED, targetPos);
    servo.write(currentPos);
    if (currentPos == targetPos && switchState && activationTime == 0)
    {
      activationTime = currentMillis;
    }
  }
  else if (currentPos > targetPos)
  {
    currentPos = max(currentPos - SERVO_SPEED, targetPos);
    servo.write(currentPos);
  }
  
  // Kiểm tra thời gian reset
  if (switchState && activationTime > 0 && (currentMillis - activationTime >= SERVO_RESET_TIME))
  {
    if (currentPos == 90)
    {
      switchState = false;
      targetPos = 0;
      activationTime = 0;
      updateButton(buttonIndex, switchState);
    }
  }
}

// Hàm xử lý nút nhấn chung
void handleButton(int x, int y, int btnX, int btnY, int btnW, int btnH, 
                 bool &switchState, int &targetPos, unsigned long &activationTime, 
                 int buttonIndex)
{
  if (x > btnX && x < btnX + btnW && y > btnY && y < btnY + btnH)
  {
    switchState = !switchState;
    targetPos = switchState ? 90 : 0;
    activationTime = 0;
    updateButton(buttonIndex, switchState);
  }
}

// Hàm di chuyển servo mượt mà
void updateServos()
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastServoUpdate >= SERVO_UPDATE_INTERVAL)
  {
    lastServoUpdate = currentMillis;
    handleServo(currentPos1, targetPos1, Switch1, servo1ActivationTime, servo1, 1);
    handleServo(currentPos2, targetPos2, Switch2, servo2ActivationTime, servo2, 2);
    handleServo(currentPos3, targetPos3, Switch3, servo3ActivationTime, servo3, 3);
  }
}

// Hiển thị thông báo kết nối WiFi
void showConnectingMessage()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Connecting WiFi...", tft.width()/2, tft.height()/2);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Smart Trash System...");
  
  // Khởi tạo TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  touch_calibrate();

  // Hiển thị thông báo kết nối
  showConnectingMessage();

  // Khởi tạo WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  // Chờ kết nối WiFi
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000)
  {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Chạy hiệu ứng chuyển màn hình
    screenTransition();
    
    // Thiết lập TLS và MQTT
    Serial.println("Setting up MQTT connection...");
    secureClient.setInsecure();
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(mqttCallback);
    
    // Kết nối MQTT
    if (reconnectMQTT())
    {
      Serial.println("MQTT Connected!");
    }
    else
    {
      Serial.println("MQTT Connection failed!");
    }
  }
  else
  {
    Serial.println("\nWiFi Connection failed!");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("WiFi Connection Failed!", tft.width()/2, tft.height()/2);
    delay(2000);
  }

  // Khởi tạo servo
  Serial.println("Initializing servos...");
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);

  // Đặt servo ở vị trí ban đầu
  currentPos1 = 0;
  currentPos2 = 0;
  currentPos3 = 0;
  targetPos1 = 0;
  targetPos2 = 0;
  targetPos3 = 0;
  servo1.write(currentPos1);
  servo2.write(currentPos2);
  servo3.write(currentPos3);

  // Vẽ nút ban đầu
  drawButton(BTN1_X, BTN1_Y, Switch1, "Servo 1");
  drawButton(BTN2_X, BTN2_Y, Switch2, "Servo 2");
  drawButton(BTN3_X, BTN3_Y, Switch3, "Servo 3");

  // Hiển thị trạng thái ban đầu
  displayStatus();
  updateConnectionStatus();
  
  Serial.println("Setup completed!");
}

void loop()
{
  unsigned long currentTime = millis();
  
  // Kiểm tra kết nối WiFi
  if (WiFi.status() != WL_CONNECTED)
  {
    if (currentTime - lastReconnectAttempt > RECONNECT_INTERVAL)
    {
      Serial.println("WiFi disconnected. Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      lastReconnectAttempt = currentTime;
      mqttConnected = false;
      updateConnectionStatus();
    }
  }
  // Kiểm tra kết nối MQTT
  else if (!mqttClient.connected())
  {
    mqttConnected = false;
    updateConnectionStatus();
    
    if (currentTime - lastReconnectAttempt > RECONNECT_INTERVAL)
    {
      lastReconnectAttempt = currentTime;
      if (reconnectMQTT())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    mqttClient.loop();
  }
  
  // Cập nhật vị trí servo mượt mà
  updateServos();
  
  // Xử lý cảm ứng
  uint16_t x, y;
  if (tft.getTouch(&x, &y))
  {
    if (currentTime - lastButtonPressTime < DEBOUNCE_DELAY)
    {
      return;
    }
    
    lastButtonPressTime = currentTime;
    
    handleButton(x, y, BTN1_X, BTN1_Y, BTN_W, BTN_H, Switch1, targetPos1, servo1ActivationTime, 1);
    handleButton(x, y, BTN2_X, BTN2_Y, BTN_W, BTN_H, Switch2, targetPos2, servo2ActivationTime, 2);
    handleButton(x, y, BTN3_X, BTN3_Y, BTN_W, BTN_H, Switch3, targetPos3, servo3ActivationTime, 3);
  }
}
