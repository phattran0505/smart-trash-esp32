#include "display.h"
#include "../config/config.h"
#include "../smart_trash_pin/smart_trash_pin.h"

void initDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Use pre-defined calibration data instead of calibrating each time
  uint16_t calData[5] = {339, 3407, 269, 3451, 7};
  tft.setTouch(calData);
}

// Hiệu ứng chuyển màn hình - Wipe transition
void screenTransition() {
  // Hiệu ứng wipe từ phải sang trái
  for (int x = 0; x <= tft.width(); x += 8) {
    tft.fillRect(tft.width() - x, 0, tft.width(), tft.height(), TFT_BLACK);
    delay(5); // Tốc độ hiệu ứng
  }

  delay(200); // Tạm dừng ngắn

  // Hiệu ứng wipe từ dưới lên
  for (int y = 0; y <= tft.height(); y += 6) {
    tft.fillRect(0, tft.height() - y, tft.width(), tft.height(), TFT_BLUE);
    delay(5); // Tốc độ hiệu ứng
  }
}

// Vẽ nút ON/OFF
void drawButton(int x, int y, bool state, const char *label) {
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
void displayStatus() {
  // Chỉ xóa vùng hiển thị trạng thái ở phía dưới
  tft.fillRect(0, 130, tft.width(), 30, TFT_BLUE); // Xóa toàn bộ hàng chữ

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);

  String status = "";
  int count = 0;

  if (Switch1) {
    status += "Servo 1";
    count++;
  }
  if (Switch2) {
    if (count > 0)
      status += " va ";
    status += "Servo 2";
    count++;
  }
  if (Switch3) {
    if (count > 0)
      status += " va ";
    status += "Servo 3";
    count++;
  }

  if (count == 0) {
    tft.drawString("Ca ba Servo deu OFF", 160, 145);
  } else if (count == 3) {
    tft.drawString("Ca ba Servo deu ON", 160, 145);
  } else {
    tft.drawString(status + " dang ON", 160, 145);
  }
}

// Cập nhật nút cụ thể và trạng thái thay vì vẽ lại toàn bộ màn hình
void updateButton(int buttonIndex, bool state) {
  int x, y;
  const char *label;

  switch (buttonIndex) {
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
void updateConnectionStatus() {
  // Clear the status area at the top
  tft.fillRect(0, 0, tft.width(), 30, TFT_BLUE);
  
  // Set text properties
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  
  // Display WiFi status
  tft.setCursor(10, 10);
  if (WiFi.status() == WL_CONNECTED) {
    tft.print("WiFi: ");
    tft.print(WiFi.localIP());
    
    // Display MQTT status
    tft.setCursor(10, 20);
    if (mqttConnected) {
      tft.print("MQTT: Connected");
    } else {
      tft.setTextColor(TFT_RED);
      tft.print("MQTT: Disconnected");
    }
  } else {
    tft.setTextColor(TFT_RED);
    tft.print("WiFi: Disconnected");
    
    // Return to white color for other elements
    tft.setTextColor(TFT_WHITE);
  }
}

// Hàm vẽ lại toàn bộ giao diện
void refreshScreen() {
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

// Hiển thị thông báo kết nối WiFi
void showConnectingMessage() {
  static int dots = 0;
  static unsigned long lastUpdate = 0;
  
  // Initial display
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Logo or title
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SMART TRASH SYSTEM", tft.width()/2, 50);
  
  // Connecting message
  tft.setTextSize(1);
  tft.drawString("Dang ket noi WiFi...", tft.width()/2, tft.height()/2);
  
  // SSID info
  tft.setTextSize(1);
  tft.drawString("SSID: " + String(ssid), tft.width()/2, tft.height()/2 + 30);
  
  // Draw a progress bar
  int barWidth = 200;
  int barHeight = 10;
  int barX = (tft.width() - barWidth) / 2;
  int barY = tft.height()/2 + 60;
  
  tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
  
  // System version/info
  tft.setTextSize(1);
  tft.drawString("Version 1.0", tft.width()/2, tft.height() - 20);
} 