/*Michael Rahr 03-28-2025
Handle the led output
*/
#include <Arduino.h>

void blinkLed();
void setLedLevel();
void setLedOn(byte  led);
void setLedOff(byte  led);
void setLedPinMode();
void setBattLevel(double adcValue);
void turnOnBackLight();
void turnOffBackLight();
