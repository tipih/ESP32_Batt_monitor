/*Michael Rahr 03-28-2025
Handle ADC reading and return a compensated value, it will compensate for the non liner ADC of the ESP32
*/
double analogReadAdjusted(byte pinNumber);