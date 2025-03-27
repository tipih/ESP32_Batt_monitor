#include <Arduino.h>
/*
  Deep Sleep with External Wake Up
  =====================================
  This code displays how to use deep sleep with
  an external trigger as a wake up source and how
  to store data in RTC memory to use it over reboots

  This code is under Public Domain License.

  Hardware Connections
  ======================
  Push Button to GPIO 33 pulled down with a 10K Ohm
  resistor

  NOTE:
  ======
  Only RTC IO can be used as a source for external wake
  source. They are pins: 0,2,4,12-15,25-27,32-39.

  Author:
  Pranav Cherukupalli <cherukupallip@gmail.com>
*/
#include "driver/rtc_io.h"


#include <test.h>
#include <ble.h>
#include <hw.h>


volatile byte NrbOfAdc = 0;                     //Variable to handle reading of ADC, set inside interrupt
volatile bool backLightOn = false;            
volatile bool backLightDetect = false;          //Variable to handle time vindue 
volatile unsigned long int currenttime = 0;     //Variable to store current time is used together with 
volatile unsigned int NrbOfWakeUp = 0;
double sensorValue = 0;  // variable to store the value coming from the sensor






#define debug
#define adc_ajustedment 13.2758


unsigned long sleepTimeout  =   5550000;


unsigned long previousMillis1 = 0;              //Variable to meassure LED blinking time
unsigned long previousMillis2 = 0;              //Variable to for BTLE broadcast and ADC reading
const long timeToBlink = 1000;                   // Interval for function1 (1 second)
const long updateLedAndBt = 2000;               // Interval for function2 (2 seconds)




unsigned long timeToSleep;
RTC_DATA_ATTR int bootCount = 0;

void getADC();
void checkADC();
void blinkLed();
void setLedLevel();
void setLedOn(byte  led);
void setLedOff(byte  led);
void turnOnBackLight();
void turnOffBackLight();





/********************************************************************************************/
/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:  {   
    Serial.println("Wakeup caused by external signal using RTC_CNTL"); 
    NrbOfWakeUp=10; //Force turn on the backlight
    timeToSleep=0;       //Reset the timeout timer
    }
    break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}
/********************************************************************************************/




/********************************************************************************************/
//External interrupt from PIN36 reset timeout
void IRAM_ATTR isr() {
  timeToSleep=0;       //Reset the timeout timer
  //Serial.print("ISR called");
  if((millis()-currenttime <100000 )&&(backLightOn==false))
  {

    NrbOfWakeUp++;
   // Serial.print("Movement detected ");
   // Serial.print(millis()-currenttime);
   // Serial.print("   ");
   // Serial.println(NrbOfWakeUp);
  }
  else if (backLightOn==false){
    NrbOfWakeUp=0;
    //Serial.println("No movement detected, ready for next session");
    backLightDetect=false;
    //Serial.println(NrbOfWakeUp);
  }



//Serial.println("ISR detected");
}
/********************************************************************************************/






/********************************************************************************************/
//ISR for BTN press
void checkADC(){
  Serial.println("Function checkADC");
  NrbOfWakeUp=10; //Force turn on the backlight
  NrbOfAdc=0;
  timeToSleep=0;       //Reset the timeout timer
 // turnOnBackLight();
}

void setLedOn(byte  led)
{
Serial.print("Led on ");
Serial.println(led);
switch (led)
{
case 1:digitalWrite(LED1, LOW);  break;
case 2:digitalWrite(LED2, LOW);  break;
case 3:digitalWrite(LED3, LOW);  break;
case 4:digitalWrite(LED4, LOW);  break;
case 5:digitalWrite(LED5, LOW);  break;
default:
  break;
}
}

void setLedOff(byte  led)
{
  Serial.print("Led off ");
  Serial.println(led);  
switch (led)
{
case 1:digitalWrite(LED1, HIGH);  break;
case 2:digitalWrite(LED2, HIGH);  break;
case 3:digitalWrite(LED3, HIGH);  break;
case 4:digitalWrite(LED4, HIGH);  break;
case 5:digitalWrite(LED5, HIGH);  break;
default:
  break;
}
}


/********************************************************************************************/
void setup() {
  Serial.begin(115200);
  delay(1000);  //Take some time to open up the Serial Monitor
  Serial.println("Step 1");
  //setup all pins
  pinMode(Back_light, OUTPUT);
  pinMode(BLINK_LED, OUTPUT);
  pinMode(WAKEUP_GPIO, INPUT);
  pinMode(GPIO_BTN, INPUT_PULLDOWN);
  //BLECharacteristicCallbacks *my_BTLE_Callback = new BLE_GetData();


Serial.println("Pin set");



  
  //Setting LED1-5 for output
  setLedPinMode();
  ble_init();


  Serial.println("Waiting for a Client to connect...");


   attachInterrupt(WAKEUP_GPIO, isr, RISING);  // Interrupt for shake sensor
   attachInterrupt(GPIO_BTN, checkADC, RISING);  // Interrupt for BTN for ADC and LED
  timeToSleep=0;                               // Clear the time to sleep flag
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  #ifdef debug
  print_wakeup_reason();
  #endif

  /*
    First we configure the wake up source
    We set our ESP32 to wake up for an external trigger.
    There are two types for ESP32, ext0 and ext1 .
    ext0 uses RTC_IO to wakeup thus requires RTC peripherals
    to be on while ext1 uses RTC Controller so does not need
    peripherals to be powered on.
    Note that using internal pullups/pulldowns also requires
    RTC peripherals to be turned on.
  */
#if USE_EXT0_WAKEUP
  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 1);  //1 = High, 0 = Low
  // Configure pullup/downs via RTCIO to tie wakeup pins to inactive level during deepsleep.
  // EXT0 resides in the same power domain (RTC_PERIPH) as the RTC IO pullup/downs.
  // No need to keep that power domain explicitly, unlike EXT1.
  rtc_gpio_pullup_dis(WAKEUP_GPIO);
  rtc_gpio_pulldown_en(WAKEUP_GPIO);
  //rtc_gpio_pulldown_dis(WAKEUP_GPIO);

//#else  // EXT1 WAKEUP
  //If you were to use ext1, you would use it like
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK(GPIO_BTN), ESP_EXT1_WAKEUP_ANY_HIGH);
  /*
    If there are no external pull-up/downs, tie wakeup pins to inactive level with internal pull-up/downs via RTC IO
         during deepsleep. However, RTC IO relies on the RTC_PERIPH power domain. Keeping this power domain on will
         increase some power comsumption. However, if we turn off the RTC_PERIPH domain or if certain chips lack the RTC_PERIPH
         domain, we will use the HOLD feature to maintain the pull-up and pull-down on the pins during sleep.
  */
  rtc_gpio_pulldown_en(GPIO_BTN);  // GPIO33 is tie to GND in order to wake up in HIGH
  rtc_gpio_pullup_dis(GPIO_BTN);   // Disable PULL_UP in order to allow it to wakeup on HIGH
#endif
  //Go to sleep now


#ifdef debug
  Serial.println("End of setup");
#endif
}
/********************************************************************************************/




void loop() {
unsigned long currentMillis = millis();
int getData = -1;
//Serial.println("Loop");
  //This is not going to be called

 
  if (currentMillis - previousMillis1 >= timeToBlink) 
  {
    Serial.println("Blick Led");
    blinkLed();
    previousMillis1 = millis();
  }  


if (timeToSleep==1){  Serial.println("Restarting timeout");}


if (NrbOfAdc==0){
  
  setLedLevel(); //TODO
 NrbOfAdc=1;

}


if (currentMillis - previousMillis2 >= updateLedAndBt) 
{
 //Call functions
 getADC();
 previousMillis2 = millis();
}

  //Prepare for movement detection to turn on backlight
  if ((backLightDetect==false)&&(backLightOn==false))
  {
    Serial.println("Ready for movement detection");
    backLightDetect=true;
  
  }

  //Window for turning on the backlight this could be ajusted, this will be depending on the sensitivity of the shake module
  if (NrbOfWakeUp>4)
  {
    Serial.print("Number of wakeup=");
    Serial.println(NrbOfWakeUp);
    turnOnBackLight();
  }

delayMicroseconds(100); //Small delay then the timeout will be sleepTimeout*100 = (550000*100 = 50 000 000) + some ADC meassurement = aprox 2 min
timeToSleep++;
//Serial.println(timeToSleep);
if (timeToSleep>sleepTimeout) {
  digitalWrite(BLINK_LED, LOW);  // turn the LED off by making the voltage LOW
  delay(1000);
  digitalWrite(BLINK_LED, HIGH);  // turn the LED off by making the voltage LOW
  #ifdef debug
 Serial.println("Going to sleep now");
#endif

#ifdef ARDUINO_ESP32S3_DEV
neopixelWrite(BLINK_LED,0,0,0); // Red
delay(100);
gpio_reset_pin(GPIO_NUM_7);
gpio_reset_pin(GPIO_NUM_8);
gpio_reset_pin(GPIO_NUM_21);
gpio_reset_pin(GPIO_NUM_4);
gpio_reset_pin(GPIO_NUM_5);
gpio_reset_pin(GPIO_NUM_12);

gpio_reset_pin(GPIO_NUM_13);
gpio_reset_pin(GPIO_NUM_14);
gpio_reset_pin(GPIO_NUM_15);
#else

 //setPin(34);
 //gpio_hold_en(GPIO_NUM_2);
 //gpio_deep_sleep_hold_en();
 #endif 
 esp_deep_sleep_start();
 }
}


/********************************************************************************************/
// Get the ADC and ajust the value
void getADC()
{
  char buffer[14];
  char buffer1[14];
  // Get the adjusted input value with the help of the 'analogReadAdjusted' function.
  float adjustedInputValue = analogReadAdjusted(pinNumber);
  // Calculate the adjusted voltage from the adjusted analog input value.
  float adjustedInputVoltage = 3.3 / 4096 * adjustedInputValue;
  float tenCellValue=(adjustedInputVoltage*adc_ajustedment)+0.6559;
  sensorValue=adjustedInputVoltage;
#ifdef debug
  Serial.print(timeToSleep);
  Serial.print(" | I new: ");
  Serial.print(adjustedInputValue, 3);
  Serial.print(" | V new: ");
  Serial.print(adjustedInputVoltage, 3);
  Serial.print(" | V 10 Cell: ");
  Serial.print(tenCellValue, 1);
  Serial.println();
#endif

  snprintf(buffer, sizeof(buffer), "%.1f", tenCellValue); // .2 specifies 2 decimal places
  snprintf(buffer1, sizeof(buffer1), "%d", timeToSleep); // 
  
  //On send to BTLE if there is a device connected
  if (isConnected)
  {
    /* Set the value */
    
    ble_update_timeout((std::string)buffer1);
    ble_update_voltage((std::string)buffer);
  }

  //setLed(adjustedInputVoltage);   //Set Status LED according voltage
}
/********************************************************************************************/


/********************************************************************************************/
//Blink Led to indicate alive
void blinkLed()
{
  #ifdef ARDUINO_ESP32S3_DEV
  neopixelWrite(BLINK_LED,0,0,RGB_BRIGHTNESS); // Red
  delay(100);
  neopixelWrite(BLINK_LED,0,0,0); // Red
  #else
   digitalWrite(BLINK_LED, HIGH); // turn the LED on (HIGH is the voltage level)
   delay(100);                       // wait for a second
   digitalWrite(BLINK_LED, LOW);  // turn the LED off by making the voltage LOW
  #endif
}
/********************************************************************************************/


void turnOnBackLight()
{
backLightOn=true;
NrbOfWakeUp=0;
Serial.println("Turn on Backlight");
//setLedOn(1);
digitalWrite(Back_light, HIGH); // turn the LED on (HIGH is the voltage level)
delay(300);
}

void turnOffBackLight()
{
  backLightOn=false;
  backLightDetect=false;
  NrbOfWakeUp=0;
  Serial.println("Turn off Backlight");
  //setLedOff(1);
  digitalWrite(Back_light, LOW); // turn the LED on (HIGH is the voltage level)
  delay(300);


}

void setLedLevel()
{

  digitalWrite(LED1,HIGH);
  digitalWrite(LED2,HIGH);
  digitalWrite(LED3,HIGH);
  digitalWrite(LED4,HIGH);
  digitalWrite(LED5,HIGH);

  //for (int i = 0; i < 10; i++) {
  for   (int a = 1; a < 6 ; a++)
  {
  setLedOn(a);
  delay(100);
  }

  for   (int a = 1; a < 6 ; a++)
  {
  setLedOff(6-a);
  delay(100);
  }
  
  delay(200);
  for   (int a = 1; a < 6 ; a++)
  {
  setLedOn(a);
  }
  delay(200);
  for   (int a = 1; a < 6 ; a++)
  {
  setLedOff(a);
  }
  delay(500);
  getADC();
 Serial.print("ADC reading ");
 Serial.println(sensorValue);
  if (sensorValue > battLevel1)   {    digitalWrite(LED1, LOW);  }  else  {    digitalWrite(LED1, HIGH);  }
  if (sensorValue > battLevel2)   {    digitalWrite(LED2, LOW);  }  else  {    digitalWrite(LED2, HIGH);  }
  if (sensorValue > battLevel3)   {    digitalWrite(LED3, LOW);  }  else  {    digitalWrite(LED3, HIGH);  }
  if (sensorValue > battLevel4)   {    digitalWrite(LED4, LOW);  }  else  {    digitalWrite(LED4, HIGH);  }
  if (sensorValue > battLevel5)   {    digitalWrite(LED5, LOW);  }  else  {    digitalWrite(LED5, HIGH);  }
  delay(5000);
  for   (int a = 1; a < 6 ; a++)
  {
  setLedOff(a);
  //setLedOn(a); //For debugging on setup do to reverse setup

  }
  delay(500);

  turnOnBackLight();
}