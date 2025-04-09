#include <Arduino.h>
#include "FS.h"
#include "SPI.h"
#include <TFT_eSPI.h>   // Thư viện màn hình TFT
#include <ESP32Servo.h> // Thư viện điều khiển Servo
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <ESP32Ping.h>

// WiFi và WebSocket configuration
const char *ssid = "H 08";
const char *password = "000000000";
const char *wsServer = "192.168.1.12";
const uint16_t wsPort = 5000;
const char *wsPath = "/ws";

WebSocketsClient webSocket;
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

// Hàm di chuyển servo mượt mà
void updateServos()
{
  unsigned long currentMillis = millis();

  // Cập nhật vị trí servo theo khoảng thời gian nhất định
  if (currentMillis - lastServoUpdate >= SERVO_UPDATE_INTERVAL)
  {
    lastServoUpdate = currentMillis;

    // Cập nhật Servo 1
    if (currentPos1 < targetPos1)
    {
      currentPos1 = min(currentPos1 + SERVO_SPEED, targetPos1);
      servo1.write(currentPos1);
    }
    else if (currentPos1 > targetPos1)
    {
      currentPos1 = max(currentPos1 - SERVO_SPEED, targetPos1);
      servo1.write(currentPos1);
    }

    // Cập nhật Servo 2
    if (currentPos2 < targetPos2)
    {
      currentPos2 = min(currentPos2 + SERVO_SPEED, targetPos2);
      servo2.write(currentPos2);
    }
    else if (currentPos2 > targetPos2)
    {
      currentPos2 = max(currentPos2 - SERVO_SPEED, targetPos2);
      servo2.write(currentPos2);
    }

    // Cập nhật Servo 3
    if (currentPos3 < targetPos3)
    {
      currentPos3 = min(currentPos3 + SERVO_SPEED, targetPos3);
      servo3.write(currentPos3);
    }
    else if (currentPos3 > targetPos3)
    {
      currentPos3 = max(currentPos3 - SERVO_SPEED, targetPos3);
      servo3.write(currentPos3);
    }
  }
}

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

// Hàm cập nhật trạng thái nút và hiển thị
void updateButtonState(int buttonIndex, bool newState)
{
  switch (buttonIndex)
  {
  case 1:
    Switch1 = newState;
    targetPos1 = newState ? 80 : 0;
    drawButton(BTN1_X, BTN1_Y, newState, "Servo 1");
    break;
  case 2:
    Switch2 = newState;
    targetPos2 = newState ? 80 : 0;
    drawButton(BTN2_X, BTN2_Y, newState, "Servo 2");
    break;
  case 3:
    Switch3 = newState;
    targetPos3 = newState ? 80 : 0;
    drawButton(BTN3_X, BTN3_Y, newState, "Servo 3");
    break;
  }
  displayStatus(); // Chỉ cập nhật phần hiển thị trạng thái
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("WebSocket Disconnected!");
    break;
  case WStype_CONNECTED:
    Serial.println("WebSocket Connected!");
    break;
  case WStype_TEXT:
    String response = String((char *)payload);
    Serial.println("Received: " + response);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      String className = doc["class"].as<String>();
      Serial.println("Class: " + className);

      if (className == "plastic")
      {
        updateButtonState(1, true);
        servo1ActivationTime = millis();
        Serial.println("Servo 1 activated - Plastic");
      }
      else if (className == "glass")
      {
        updateButtonState(2, true);
        servo2ActivationTime = millis();
        Serial.println("Servo 2 activated - Glass");
      }
      else if (className == "common")
      {
        updateButtonState(3, true);
        servo3ActivationTime = millis();
        Serial.println("Servo 3 activated - Common");
      }
    }
    else
    {
      Serial.println("JSON Parse Error!");
    }
    break;

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

void setup()
{
  Serial.begin(115200);

  // Khởi tạo TFT trước
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLUE);
  touch_calibrate();

  // Khởi tạo WiFi
  WiFi.begin(ssid, password);

  // Vẽ giao diện khởi đầu và thông báo kết nối ở giữa màn hình
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM); // Căn giữa văn bản
  tft.drawString("Connecting to WiFi...", tft.width() / 2, tft.height() / 2);

  // Chờ kết nối WiFi
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000)
  {
    delay(500);
    Serial.print(".");
  }

  // Hiệu ứng chuyển màn hình
  screenTransition();

  // Xóa toàn bộ màn hình trước khi vẽ các nút
  tft.fillScreen(TFT_BLUE);

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Cập nhật thông báo trên màn hình
    tft.fillRect(0, 0, tft.width(), 40, TFT_BLUE);
    tft.setCursor(10, 10);
    tft.setTextSize(1);
    tft.print("WiFi connected: ");
    tft.print(WiFi.localIP());

    // Thiết lập WebSocket
    webSocket.begin(wsServer, wsPort, wsPath);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);

    webSocket.enableHeartbeat(15000, 3000, 2);
  }
  else
  {
    Serial.println("Failed to connect to WiFi!");
    tft.fillRect(0, 0, tft.width(), 40, TFT_RED);
    tft.setCursor(10, 10);
    tft.setTextSize(1);
    tft.print("WiFi connection failed!");
  }

  // Khởi tạo servo
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
}

void loop()
{
  // Xử lý WebSocket
  webSocket.loop();

  // Cập nhật vị trí servo mượt mà
  updateServos();

  // Kiểm tra thời gian để reset servo
  unsigned long currentTime = millis();

  // Kiểm tra Servo 1
  if (Switch1 && servo1ActivationTime > 0 && (currentTime - servo1ActivationTime >= SERVO_RESET_TIME))
  {
    updateButtonState(1, false);
    Serial.println("Servo 1 reset after 5 seconds");
    servo1ActivationTime = 0;
  }

  // Kiểm tra Servo 2
  if (Switch2 && servo2ActivationTime > 0 && (currentTime - servo2ActivationTime >= SERVO_RESET_TIME))
  {
    updateButtonState(2, false);
    Serial.println("Servo 2 reset after 5 seconds");
    servo2ActivationTime = 0;
  }

  // Kiểm tra Servo 3
  if (Switch3 && servo3ActivationTime > 0 && (currentTime - servo3ActivationTime >= SERVO_RESET_TIME))
  {
    updateButtonState(3, false);
    Serial.println("Servo 3 reset after 5 seconds");
    servo3ActivationTime = 0;
  }

  // Xử lý cảm ứng
  uint16_t x, y;
  if (tft.getTouch(&x, &y))
  {
    // Kiểm tra thời gian chống dội
    if (currentTime - lastButtonPressTime < DEBOUNCE_DELAY)
    {
      return;
    }

    lastButtonPressTime = currentTime;

    // Kiểm tra nếu nhấn vào nút 1
    if (x > BTN1_X && x < BTN1_X + BTN_W && y > BTN1_Y && y < BTN1_Y + BTN_H)
    {
      updateButtonState(1, !Switch1);
      if (Switch1)
      {
        servo1ActivationTime = millis();
      }
      else
      {
        servo1ActivationTime = 0;
      }
      Serial.println("Nút 1 nhấn");
    }

    // Kiểm tra nếu nhấn vào nút 2
    if (x > BTN2_X && x < BTN2_X + BTN_W && y > BTN2_Y && y < BTN2_Y + BTN_H)
    {
      updateButtonState(2, !Switch2);
      if (Switch2)
      {
        servo2ActivationTime = millis();
      }
      else
      {
        servo2ActivationTime = 0;
      }
      Serial.println("Nút 2 nhấn");
    }

    // Kiểm tra nếu nhấn vào nút 3
    if (x > BTN3_X && x < BTN3_X + BTN_W && y > BTN3_Y && y < BTN3_Y + BTN_H)
    {
      updateButtonState(3, !Switch3);
      if (Switch3)
      {
        servo3ActivationTime = millis();
      }
      else
      {
        servo3ActivationTime = 0;
      }
      Serial.println("Nút 3 nhấn");
    }
  }
}
