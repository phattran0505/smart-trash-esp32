#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "../config/config.h"


#define TRIG_PIN_1 27
#define ECHO_PIN_1 26

#define TRIG_PIN_2 25
#define ECHO_PIN_2 33

#define TRIG_PIN_3 32   
#define ECHO_PIN_3 35

#define BIN_HEIGHT 10.0       // Chiều cao thùng rác (cm)
#define MIN_DISTANCE 6.0      // Khoảng cách xác định thùng rác đầy (cm) - khi khoảng cách <= 6cm thì thùng đầy

// Khai báo hàm
void initUltrasonic();
void updateUltrasonic();
float getDistance(int trigPin, int echoPin);
bool isTrashFull(int binIndex);

// Biến theo dõi trạng thái đầy của thùng rác
extern bool bin1Full, bin2Full, bin3Full;
extern float bin1Level, bin2Level, bin3Level; // Mức độ đầy của thùng rác (%)
extern float distance1, distance2, distance3; // Khoảng cách đo được từ cảm biến (cm)
extern bool needDisplayUpdate; // Cờ báo hiệu cần cập nhật hiển thị

#endif