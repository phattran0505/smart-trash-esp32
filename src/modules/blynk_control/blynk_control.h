#ifndef BLYNK_CONTROL_H
#define BLYNK_CONTROL_H

#include "../config/config.h"

// Function declarations
void initBlynk();
void updateBlynk();
void updateBlynkStatus();
void blynkWriteV1(int value);
void blynkWriteV2(int value);
void blynkWriteV3(int value);

#endif // BLYNK_CONTROL_H 