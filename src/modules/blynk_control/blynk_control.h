#ifndef BLYNK_CONTROL_H
#define BLYNK_CONTROL_H

#include "../config/config.h"

// Định nghĩa event keys cho Blynk LogEvent
#define EVENT_BIN1_FULL "alert_trash"
#define EVENT_BIN2_FULL "alert_trash"
#define EVENT_BIN3_FULL "alert_trash"

// Function declarations
void initBlynk();
void updateBlynk();
void updateBlynkStatus();
void blynkWriteV1(int value);
void blynkWriteV2(int value);
void blynkWriteV3(int value);
void checkAndNotifyTrashStatus(bool bin1Full, bool bin2Full, bool bin3Full);
void sendTrashFullNotification(int binNumber);

// Biến theo dõi trạng thái đã thông báo
extern bool bin1NotificationSent;
extern bool bin2NotificationSent;
extern bool bin3NotificationSent;

#endif // BLYNK_CONTROL_H 