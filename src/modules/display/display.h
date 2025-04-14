#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

// Function declarations
void initDisplay();
void screenTransition();
void drawButton(int x, int y, bool state, const char *label);
void displayStatus();
void updateButton(int buttonIndex, bool state);
void updateConnectionStatus();
void refreshScreen();
void showConnectingMessage();

#endif // DISPLAY_H 