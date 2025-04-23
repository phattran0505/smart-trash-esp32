#include "ultrasonic.h"
#include "../smart_trash_pin/smart_trash_pin.h"
#include "../display/display.h"
#include <Arduino.h>

// Biến theo dõi trạng thái đầy của thùng rác
bool bin1Full = false;
bool bin2Full = false;
bool bin3Full = false;

// Mức độ đầy của thùng rác (%)
float bin1Level = 0.0;
float bin2Level = 0.0;
float bin3Level = 0.0;

// Biến báo hiệu cần cập nhật hiển thị
bool needDisplayUpdate = false;

// Thời gian cập nhật gần nhất
unsigned long lastUltrasonicUpdate = 0;
const unsigned long ULTRASONIC_UPDATE_INTERVAL = 1000; // 1 giây

// Biến lưu khoảng cách đo được
float distance1 = 0.0;
float distance2 = 0.0;
float distance3 = 0.0;

void initUltrasonic() {
  // Cấu hình các chân cho cảm biến siêu âm
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  
//   pinMode(TRIG_PIN_2, OUTPUT);
//   pinMode(ECHO_PIN_2, INPUT);
  
//   pinMode(TRIG_PIN_3, OUTPUT);
//   pinMode(ECHO_PIN_3, INPUT);
  
  Serial.println("Ultrasonic sensors initialized");
}

float getDistance(int trigPin, int echoPin) {
  // Gửi xung trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Đọc phản hồi ECHO (có timeout)
  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout ~ 5m max
  
  if (duration == 0) {
    // Không nhận được phản hồi, trả về giá trị lớn
    return BIN_HEIGHT + 10.0;
  }
  
  // Tính khoảng cách (cm)
  float distance = duration * 0.034 / 2.0;
  
  // HC-SR04 có giới hạn đo khoảng cách ngắn, thường không chính xác dưới 2-3cm
  // Nếu khoảng cách quá nhỏ, sẽ ảnh hưởng đến phép đo
  if (distance < 2.0) {
    distance = 2.0; // Đặt giới hạn tối thiểu là 2cm
  }
  
  // Giới hạn phạm vi đo từ 0 đến chiều cao thùng + 5cm (dự phòng)
  if (distance > BIN_HEIGHT + 5.0) {
    distance = BIN_HEIGHT + 5.0;
  }
  
  // In ra khoảng cách đo được
  Serial.print("Distance (pin ");
  Serial.print(trigPin);
  Serial.print("): ");
  Serial.print(distance);
  Serial.println(" cm");
  
  return distance;
}

bool isTrashFull(int binIndex) {
  float distance = 0.0;
  
  switch (binIndex) {
    case 1:
      distance = getDistance(TRIG_PIN_1, ECHO_PIN_1);
      distance1 = distance; // Lưu lại khoảng cách đo được
      break;
    case 2:
      distance = getDistance(TRIG_PIN_2, ECHO_PIN_2);
      distance2 = distance; // Lưu lại khoảng cách đo được
      break;
    case 3:
      distance = getDistance(TRIG_PIN_3, ECHO_PIN_3);
      distance3 = distance; // Lưu lại khoảng cách đo được
      break;
    default:
      return false;
  }
  
  // Tính toán mức độ đầy (%) - vẫn giữ để hiển thị
  float fillLevel = 0.0;
  
  // Nếu khoảng cách nhỏ hơn hoặc bằng chiều cao thùng rác
  if (distance <= BIN_HEIGHT) {
    // Tính phần trăm đầy (khoảng cách càng nhỏ thì càng đầy)
    fillLevel = 100.0 * (1.0 - (distance / BIN_HEIGHT));
  }
  
  // Cập nhật mức độ đầy
  switch (binIndex) {
    case 1:
      bin1Level = fillLevel;
      break;
    case 2:
      bin2Level = fillLevel;
      break;
    case 3:
      bin3Level = fillLevel;
      break;
  }
  
  // Trả về true nếu khoảng cách nhỏ hơn hoặc bằng MIN_DISTANCE
  return (distance <= MIN_DISTANCE);
}

void updateUltrasonic() {
  unsigned long currentTime = millis();
  
  // Chỉ cập nhật sau mỗi khoảng thời gian định trước
  if (currentTime - lastUltrasonicUpdate < ULTRASONIC_UPDATE_INTERVAL) {
    return;
  }
  
  lastUltrasonicUpdate = currentTime;
  
  // Lưu trạng thái hiện tại để phát hiện thay đổi
  bool prevBin1Full = bin1Full;
  bool prevBin2Full = bin2Full;
  bool prevBin3Full = bin3Full;
  
  // Kiểm tra mức độ đầy của từng thùng rác
  bin1Full = isTrashFull(1);
  bin2Full = isTrashFull(2);
  bin3Full = isTrashFull(3);
  
  // In thông tin tóm tắt về trạng thái các thùng rác
  Serial.println("\n----- TRASH BIN STATUS -----");
  Serial.print("Bin 1: Distance = ");
  Serial.print(distance1);
  Serial.print(" cm, Level = ");
  Serial.print(bin1Level);
  Serial.println(bin1Full ? "% - CẢNH BÁO: THÙNG RÁC ĐẦY!" : "%");
  
  Serial.print("Bin 2: Distance = ");
  Serial.print(distance2);
  Serial.print(" cm, Level = ");
  Serial.print(bin2Level);
  Serial.println(bin2Full ? "% - CẢNH BÁO: THÙNG RÁC ĐẦY!" : "%");
  
  Serial.print("Bin 3: Distance = ");
  Serial.print(distance3);
  Serial.print(" cm, Level = ");
  Serial.print(bin3Level);
  Serial.println(bin3Full ? "% - CẢNH BÁO: THÙNG RÁC ĐẦY!" : "%");
  Serial.println("---------------------------");
  
  // Chỉ đặt cờ cập nhật khi có sự thay đổi trạng thái
  if (bin1Full != prevBin1Full || bin2Full != prevBin2Full || bin3Full != prevBin3Full) {
    needDisplayUpdate = true;
  }
}
