
#define LED1                     GPIO_NUM_26
#define LED2                     GPIO_NUM_27
#define LED3                     GPIO_NUM_14
#define LED4                     GPIO_NUM_12
#define LED5                     GPIO_NUM_13

#define battLevel1                2.6
#define battLevel2                2.7
#define battLevel3                2.8
#define battLevel4                2.9
#define battLevel5                3.0



void setPin(int pinnumber);
void setLed(float adjustedInputVoltage);
void setLedPinMode();
double analogReadAdjusted(byte pinNumber);