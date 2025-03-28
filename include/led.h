// Michael Rahr 03-28-2025
// This code is under Public Domain License.
// Handle all LED functions

#include <Arduino.h>

void blinkLed();    // Blinking the buildin LED for debugging
void setLedLevel(); // Setting LED indicators based on ADC reading, it will also
                    // run a LED pattern first
void setLedOn(byte led);  // Set 1 LED ON
void setLedOff(byte led); // Set 1 LED OFF
void setLedPinMode();     // Init all LED
void setBattLevel(double adcValue);    // Setting Batt level Indicators based on ADC reading
void turnOnBackLight();  // Turn on the PIN for BAcklight
void turnOffBackLight(); // Turn off the PIN for BAcklight
