/*Michael Rahr 03-28-2025
Define all platform specific IO
*/
#include "Arduino.h"
#ifndef HW_H
#define HW_H

#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)  // 2 ^ GPIO_NUMBER in hex
#define USE_EXT0_WAKEUP          1               // 1 = EXT0 wakeup, 0 = EXT1 wakeup



#ifdef ARDUINO_ESP32S3_DEV
 #define WAKEUP_GPIO              GPIO_NUM_7     // Only RTC IO are allowed - ESP32 Pin example
 #define GPIO_BTN                 GPIO_NUM_8     // ADC BTN
 #define RGB_BRIGHTNESS           10             // Change white brightness (max 255)
 #define BLINK_LED                GPIO_NUM_21    // Blue LED 21
 #else
 //#define BLINK_LED                GPIO_NUM_18     // Blue LED 1 EZC
 #define BLINK_LED                GPIO_NUM_2     // Blue LED 2 firebeatle
 #define WAKEUP_GPIO              GPIO_NUM_33     // Only RTC IO are allowed - ESP32 Pin example
 #define GPIO_BTN                 GPIO_NUM_25     // ADC BTN
 #define LED_PIN 
#endif

#define LED1                     GPIO_NUM_5
#define LED2                     GPIO_NUM_12
#define LED3                     GPIO_NUM_13
#define LED4                     GPIO_NUM_14
#define LED5                     GPIO_NUM_15

#ifdef ARDUINO_ESP32S3_DEV
 const byte ADCPIN = 1; //Pin for ADC meassurement
#else

const byte ADCPIN = 36; //Pin for ADC meassurement
#endif


//#define BLINK_LED                GPIO_NUM_18     // Blue LED 1
#define Back_light               GPIO_NUM_4      // Backlight //TODO CAPITAL LETTERS

#define multiplecell //singlecell //multiplecell 

#ifdef multiplecell //Voltage using a voltage devider
#define battLevel1                2.5
#define battLevel2                2.6
#define battLevel3                2.7
#define battLevel4                2.9
#define battLevel5                3.0
#endif

#ifdef singlecell
#define battLevel1                2.9
#define battLevel2                3.1
#define battLevel3                3.5
#define battLevel4                3.9
#define battLevel5                4.1
#endif



#endif