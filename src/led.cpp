/*Michael Rahr 03-28-2025
Led handling
*/

#include <adcread.h>
#include <hw.h>
#include <led.h>

// function to turn on one LED, input 1-5
void setLedOn(byte led) {
  Serial.print("Led on ");
  Serial.println(led);
  switch (led) {
  case 1:
    digitalWrite(LED1, LOW);
    break;
  case 2:
    digitalWrite(LED2, LOW);
    break;
  case 3:
    digitalWrite(LED3, LOW);
    break;
  case 4:
    digitalWrite(LED4, LOW);
    break;
  case 5:
    digitalWrite(LED5, LOW);
    break;
  default:
    break;
  }
}

// function to turn off one LED, input 1-5
void setLedOff(byte led) {

  Serial.print("Led off ");
  Serial.println(led);
  switch (led) {
  case 1:
    digitalWrite(LED1, HIGH);
    break;
  case 2:
    digitalWrite(LED2, HIGH);
    break;
  case 3:
    digitalWrite(LED3, HIGH);
    break;
  case 4:
    digitalWrite(LED4, HIGH);
    break;
  case 5:
    digitalWrite(LED5, HIGH);
    break;
  default:
    break;
  }
}

// Blink onboard Led to indicate alive
void blinkLed() {
#ifdef ARDUINO_ESP32S3_DEV
  neopixelWrite(BLINK_LED, 0, 0, RGB_BRIGHTNESS); // Red
  delay(100);
  neopixelWrite(BLINK_LED, 0, 0, 0); // Red
#else
  digitalWrite(BLINK_LED, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);                    // wait for a second
  digitalWrite(BLINK_LED, LOW);  // turn the LED off by making the voltage LOW
#endif
}

// Set all LED to output
void setLedPinMode() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);

  // TODO fix names

  pinMode(BLINK_LED, OUTPUT);
}

// Set led level based on ADC readout
// This will also preform a light sequence
// This will also turn on the backlight
// Function will be invoked by the checkADC button triggered in the interrupt
void setLedLevel() {

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);

  // for (int i = 0; i < 10; i++) {
  for (int a = 1; a < 6; a++) {
    setLedOn(a);
    delay(100);
  }

  for (int a = 1; a < 6; a++) {
    setLedOff(6 - a);
    delay(100);
  }

  delay(200);
  for (int a = 1; a < 6; a++) {
    setLedOn(a);
  }
  delay(200);
  for (int a = 1; a < 6; a++) {
    setLedOff(a);
  }
  delay(500);
  float adjustedInputValue = analogReadAdjusted(ADCPIN);
  setBattLevel(adjustedInputValue);

  delay(5000);
  for (int a = 1; a < 6; a++) {
    setLedOff(a);
  }
  delay(500);

  turnOnBackLight();
}

// Set LED indicators based on ADC reading
// TODO input error handling
void setBattLevel(double adcValue) {
  Serial.print("ADC reading ");
  Serial.println(adcValue);
  if (adcValue > battLevel1) {
    digitalWrite(LED1, LOW);
  } else {
    digitalWrite(LED1, HIGH);
  }
  if (adcValue > battLevel2) {
    digitalWrite(LED2, LOW);
  } else {
    digitalWrite(LED2, HIGH);
  }
  if (adcValue > battLevel3) {
    digitalWrite(LED3, LOW);
  } else {
    digitalWrite(LED3, HIGH);
  }
  if (adcValue > battLevel4) {
    digitalWrite(LED4, LOW);
  } else {
    digitalWrite(LED4, HIGH);
  }
  if (adcValue > battLevel5) {
    digitalWrite(LED5, LOW);
  } else {
    digitalWrite(LED5, HIGH);
  }
}

void turnOnBackLight() {
  Serial.println("Turn on Backlight");
  // setLedOn(1);
  digitalWrite(Back_light, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(300);
}

void turnOffBackLight() {
  Serial.println("Turn off Backlight");
  digitalWrite(Back_light, LOW); // turn the LED on (HIGH is the voltage level)
  delay(300);
}
