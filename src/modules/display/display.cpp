#include "display.h"
#include "../config/config.h"
#include "../smart_trash_pin/smart_trash_pin.h"
#include "../ultrasonic/ultrasonic.h"

// Biến lưu trữ trạng thái hiển thị trước đó
static int prevBin1Level = -1;
static int prevBin2Level = -1;
static int prevBin3Level = -1;
static bool prevBin1Full = false;
static bool prevBin2Full = false;
static bool prevBin3Full = false;

void initDisplay()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Use pre-defined calibration data instead of calibrating each time
  uint16_t calData[5] = {339, 3407, 269, 3451, 7};

  tft.setTouch(calData);

  // Skip calibration and go directly to WiFi connection screen
}

// Hiệu chỉnh cảm ứng
void touch_calibrate()
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");

  uint16_t calData[5];
  tft.calibrateTouch(calData, TFT_RED, TFT_BLACK, 15);

  Serial.println("Calibration complete! Copy these calibration values:");
  Serial.print("{ ");
  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4)
      Serial.print(", ");
  }
  Serial.println(" }");

  tft.setTouch(calData);

  delay(1000);
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

// Hiển thị trạng thái mức rác trong thùng - Thay thế function cũ
void displayStatus()
{
  // Chuyển đổi thành giá trị int để giảm nhấp nháy do thay đổi nhỏ
  int currentBin1Level = int(bin1Level);
  int currentBin2Level = int(bin2Level);
  int currentBin3Level = int(bin3Level);

  // Chỉ cập nhật khi có sự thay đổi hoặc khi cờ cập nhật được đặt
  bool needUpdate = (currentBin1Level != prevBin1Level) ||
                    (currentBin2Level != prevBin2Level) ||
                    (currentBin3Level != prevBin3Level) ||
                    (bin1Full != prevBin1Full) ||
                    (bin2Full != prevBin2Full) ||
                    (bin3Full != prevBin3Full) ||
                    needDisplayUpdate;

  if (!needUpdate)
  {
    return; // Không cần cập nhật nếu không có thay đổi
  }

  // Cập nhật giá trị trước đó
  prevBin1Level = currentBin1Level;
  prevBin2Level = currentBin2Level;
  prevBin3Level = currentBin3Level;
  prevBin1Full = bin1Full;
  prevBin2Full = bin2Full;
  prevBin3Full = bin3Full;

  // Xóa vùng hiển thị trạng thái ở phía dưới
  tft.fillRect(0, 130, tft.width(), 60, TFT_BLUE);

  // Hiển thị thông tin mức độ đầy của các thùng
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);

  // Thùng 1
  String bin1Status = "Thung 1: " + String(currentBin1Level) + "%";
  if (bin1Full)
  {
    tft.setTextColor(TFT_RED);
    bin1Status += " - FULL!";
  }
  else
  {
    tft.setTextColor(TFT_WHITE);
  }
  tft.drawString(bin1Status, 160, 140);

  // Thùng 2
  String bin2Status = "Thung 2: " + String(currentBin2Level) + "%";
  if (bin2Full)
  {
    tft.setTextColor(TFT_RED);
    bin2Status += " - FULL!";
  }
  else
  {
    tft.setTextColor(TFT_WHITE);
  }
  tft.drawString(bin2Status, 160, 160);

  // Thùng 3
  String bin3Status = "Thung 3: " + String(currentBin3Level) + "%";
  if (bin3Full)
  {
    tft.setTextColor(TFT_RED);
    bin3Status += " - FULL!";
  }
  else
  {
    tft.setTextColor(TFT_WHITE);
  }
  tft.drawString(bin3Status, 160, 180);
  
  // Reset the update flag after display has been updated
  extern bool needDisplayUpdate;
  needDisplayUpdate = false;
}

// Hiển thị trạng thái mức rác trong thùng - khi có cảnh báo
void displayTrashStatus()
{
  // Cập nhật thông tin mức độ đầy thùng rác
  displayStatus();
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

  // Cập nhật trạng thái - không hiển thị thông tin servo nữa
  displayStatus();
}

// Hiển thị trạng thái kết nối
void updateConnectionStatus()
{
  // Clear the status area at the top
  tft.fillRect(0, 0, tft.width(), 30, TFT_BLUE);

  // Set text properties
  tft.setTextColor(TFT_WHITE);

  // Display WiFi status
  if (WiFi.status() == WL_CONNECTED)
  {
    // Show WiFi connected status
    tft.setTextSize(1);
    tft.setCursor(10, 5);
    tft.print("WiFi: Connected");

    // Show IP with larger font
    String ipStr = WiFi.localIP().toString();
    tft.setCursor(10, 25);
    tft.setTextSize(1);
    tft.print("IP: ");
    tft.setTextSize(1);
    tft.print(ipStr);
    
    // Display MQTT status
    tft.setCursor(10, 15);
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
    tft.setTextSize(1);
    tft.setCursor(10, 5);
    tft.print("WiFi: Disconnected");

    // Return to white color for other elements
    tft.setTextColor(TFT_WHITE);
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

  // Hiển thị trạng thái mức độ đầy thay vì trạng thái servo
  displayStatus();
}

// Hiển thị thông báo kết nối WiFi
void showConnectingMessage()
{
  static int dots = 0;
  static unsigned long lastUpdate = 0;

  // Initial display
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Logo or title
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SMART TRASH PIN SYSTEM", tft.width() / 2, 50);

  // Connecting message
  tft.setTextSize(1);
  tft.drawString("Dang ket noi WiFi...", tft.width() / 2, tft.height() / 2);

  // SSID info
  tft.setTextSize(1);
  tft.drawString("SSID: " + String(ssid), tft.width() / 2, tft.height() / 2 + 30);

  // Draw a progress bar
  int barWidth = 200;
  int barHeight = 10;
  int barX = (tft.width() - barWidth) / 2;
  int barY = tft.height() / 2 + 60;

  tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);

  // System version/info
  tft.setTextSize(1);
  tft.drawString("Version 1.0", tft.width() / 2, tft.height() - 20);
}