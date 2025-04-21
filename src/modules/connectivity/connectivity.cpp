#include "connectivity.h"
#include "../config/config.h"
#include "../smart_trash_pin/smart_trash_pin.h"
#include "../display/display.h"

void initConnectivity() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  // Wait for WiFi connection
  unsigned long startTime = millis();
  int progress = 0;
  int barWidth = 200;
  int barHeight = 10;
  int barX = (tft.width() - barWidth) / 2;
  int barY = tft.height()/2 + 60;
  
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
    
    // Update progress bar
    progress += 10;
    if (progress > barWidth - 4) progress = barWidth - 4;
    
    // Draw progress bar fill
    tft.fillRect(barX + 2, barY + 2, progress, barHeight - 4, TFT_CYAN);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Fill the progress bar completely
    tft.fillRect(barX + 2, barY + 2, barWidth - 4, barHeight - 4, TFT_GREEN);
    
    // Show connection successful message
    tft.fillRect(0, tft.height()/2 - 10, tft.width(), 20, TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Ket noi WiFi thanh cong!", tft.width()/2, tft.height()/2);
    
    // Show IP information - replace SSID info at the same position
    tft.fillRect(0, tft.height()/2 + 20, tft.width(), 30, TFT_BLACK); // Clear the SSID area
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1); // Larger font for better readability
    tft.setTextDatum(MC_DATUM); // Center alignment
    
    String ipStr = WiFi.localIP().toString();
    tft.drawString("IP: " + ipStr, tft.width()/2, tft.height()/2 + 30); // Same position as SSID was
    
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    delay(1500); // Show the success message briefly
    
    // Run screen transition
    screenTransition();
    
    // Set up TLS and MQTT
    Serial.println("Setting up MQTT connection...");
    secureClient.setInsecure();
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(mqttCallback);
    
    // Connect to MQTT
    if (reconnectMQTT()) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.println("MQTT Connection failed!");
    }
  } else {
    Serial.println("\nWiFi Connection failed!");
    
    // Brief notification of failure
    tft.fillRect(barX + 2, barY + 2, progress, barHeight - 4, TFT_RED);
    tft.fillRect(0, tft.height()/2 - 10, tft.width(), 20, TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Ket noi WiFi that bai!", tft.width()/2, tft.height()/2);
    delay(2000);
    
    // Transition to the main interface despite connection failure
    screenTransition();
  }
  
  // Always update connection status at the top of the screen
  updateConnectionStatus();
}

// Hàm xử lý khi nhận được tin nhắn MQTT
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  // Chuyển payload thành chuỗi
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Phân tích JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (!error) {
    // Check if "class" key exists and is not null
    if (!doc["class"].isNull()) {
      String className = doc["class"].as<String>();
      Serial.print("Class detected: ");
      Serial.println(className);
      
      // Kiểm tra class và điều khiển servo tương ứng
      if (className == "plastic") {
        Switch1 = true;
        servo1.write(90);
        servo1ActivationTime = millis();
        updateButton(1, Switch1);
      } else if (className == "glass") {
        Switch2 = true;
        servo2.write(90);
        servo2ActivationTime = millis();
        updateButton(2, Switch2);
      } else if (className == "common") {
        Switch3 = true;
        servo3.write(90);
        servo3ActivationTime = millis();
        updateButton(3, Switch3);
      }
    }
  }
}

// Kết nối lại MQTT nếu bị mất kết nối
boolean reconnectMQTT() {
  Serial.print("Connecting to MQTT...");
  
  // Tạo client ID dựa trên địa chỉ MAC
  String clientId = "ESP32_SmartTrash-" + String(WiFi.macAddress());
  
  // Thử kết nối với thông tin xác thực
  if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
    Serial.println("connected");
    mqttClient.subscribe(mqtt_topic);
    mqttConnected = true;
    updateConnectionStatus();
    return true;
  } else {
    Serial.print("failed, rc=");
    Serial.println(mqttClient.state());
    mqttConnected = false;
    updateConnectionStatus();
    return false;
  }
}

void updateConnectivity() {
  static unsigned long lastReconnectAttempt = 0;
  unsigned long currentTime = millis();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    if (currentTime - lastReconnectAttempt > RECONNECT_INTERVAL) {
      Serial.println("WiFi disconnected. Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      lastReconnectAttempt = currentTime;
      mqttConnected = false;
      updateConnectionStatus();
    }
  } 
  // Check MQTT connection
  else if (!mqttClient.connected()) {
    mqttConnected = false;
    updateConnectionStatus();
    
    if (currentTime - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = currentTime;
      if (reconnectMQTT()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    mqttClient.loop();
  }
} 