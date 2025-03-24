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

//Include BLE libs
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <test.h>


volatile byte NrbOfAdc = 0;
volatile bool backLightOn = false;
volatile bool backLightDetect = false;
volatile unsigned long int currenttime = 0;
volatile unsigned int NrbOfWakeUp = 0;
double sensorValue = 0;  // variable to store the value coming from the sensor


#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)  // 2 ^ GPIO_NUMBER in hex
#define USE_EXT0_WAKEUP          1               // 1 = EXT0 wakeup, 0 = EXT1 wakeup
#define WAKEUP_GPIO              GPIO_NUM_33     // Only RTC IO are allowed - ESP32 Pin example
#define GPIO_BTN                 GPIO_NUM_32     // ADC BTN
#define BLINK_LED                GPIO_NUM_18     // Blue LED 1
#define Back_light               GPIO_NUM_4      // Backlight //TODO CAPITAL LETTERS
#define debug
#define adc_ajustedment 13.2758


#define sleepTimeout              550000


unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
const long timeToBlink = 200; // Interval for function1 (1 second)
const long updateLedAndBt = 2000; // Interval for function2 (2 seconds)



const byte pinNumber = 36; //Pin for ADC meassurement
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
/* Define the UUID for our Custom Service */
#define serviceID BLEUUID((uint16_t)0x1700)
/********************************************************************************************/




/********************************************************************************************/
/* Define our custom characteristic along with it's properties */
BLECharacteristic customCharacteristic(
  BLEUUID((uint16_t)0x1A00), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);
BLECharacteristic customCharacteristic1(
  BLEUUID((uint16_t)0x1A01), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);
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
/* This function handles the server callbacks */
bool deviceConnected = false;
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* MyServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* MyServer) {
      deviceConnected = false;
    }
};
/********************************************************************************************/





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
  //setup all pins
  pinMode(Back_light, OUTPUT);
  pinMode(BLINK_LED, OUTPUT);
  pinMode(WAKEUP_GPIO, INPUT);
  pinMode(GPIO_BTN, INPUT);
  
  
  //Setting LED1-5 for output
  setLedPinMode();

  // Create and name the BLE Device
  BLEDevice::init("Bike_Batt");

  /* Create the BLE Server */
  BLEServer *MyServer = BLEDevice::createServer();
  MyServer->setCallbacks(new ServerCallbacks());  // Set the function that handles Server Callbacks

  /* Add a service to our server */
  BLEService *customService = MyServer->createService(BLEUUID((uint16_t)0x1700)); //  A random ID has been selected

  /* Add a characteristic to the service */
  customService->addCharacteristic(&customCharacteristic);  //customCharacteristic was defined above
  /* Add a characteristic to the service */
  customService->addCharacteristic(&customCharacteristic1);  //customCharacteristic was defined above

  /* Add Descriptors to the Characteristic*/
  customCharacteristic.addDescriptor(new BLE2902());  //Add this line only if the characteristic has the Notify property
  customCharacteristic1.addDescriptor(new BLE2902());  //Add this line only if the characteristic has the Notify property

  BLEDescriptor VariableDescriptor(BLEUUID((uint16_t)0x2901));  /*```````````````````````````````````````````````````````````````*/
  VariableDescriptor.setValue("Show Battery Voltage");          /* Use this format to add a hint for the user. This is optional. */
  customCharacteristic.addDescriptor(&VariableDescriptor);    /*```````````````````````````````````````````````````````````````*/

  BLEDescriptor VariableDescriptor1(BLEUUID((uint16_t)0x2901));  /*```````````````````````````````````````````````````````````````*/
  VariableDescriptor1.setValue("Show timeout");          /* Use this format to add a hint for the user. This is optional. */
  customCharacteristic1.addDescriptor(&VariableDescriptor1);   

  /* Configure Advertising with the Services to be advertised */
  MyServer->getAdvertising()->addServiceUUID(serviceID);

  // Start the service
  customService->start();

  // Start the Server/Advertising
  MyServer->getAdvertising()->start();

  Serial.println("Waiting for a Client to connect...");


   attachInterrupt(WAKEUP_GPIO, isr, RISING);  // Interrupt for shake sensor
   attachInterrupt(GPIO_BTN, checkADC, HIGH);  // Interrupt for BTN for ADC and LED
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
#ifdef debug
 Serial.println("Going to sleep now");
#endif

 setPin(34);
 gpio_hold_en(GPIO_NUM_2);
 gpio_deep_sleep_hold_en();
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
  if (deviceConnected)
  {
    /* Set the value */
    // customCharacteristic.setValue((float&)adjustedInputVoltage);
    customCharacteristic.setValue((std::string)buffer);   //Set the string
    customCharacteristic.notify();                        // Notify the client of a change
    customCharacteristic1.setValue((std::string)buffer1);   //Set the string
    customCharacteristic1.notify();                        // Notify the client of a change
  }

  //setLed(adjustedInputVoltage);   //Set Status LED according voltage
}
/********************************************************************************************/


/********************************************************************************************/
//Blink Led to indicate alive
void blinkLed()
{
  digitalWrite(BLINK_LED, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(BLINK_LED, LOW);  // turn the LED off by making the voltage LOW
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