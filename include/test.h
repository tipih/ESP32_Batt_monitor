
#define LED1                     GPIO_NUM_5
#define LED2                     GPIO_NUM_12
#define LED3                     GPIO_NUM_13
#define LED4                     GPIO_NUM_14
#define LED5                     GPIO_NUM_15

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


void setPin(int pinnumber);
void setLed(float adjustedInputVoltage);
void setLedPinMode();
double analogReadAdjusted(byte pinNumber);