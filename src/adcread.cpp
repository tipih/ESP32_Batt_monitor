// Michael Rahr 03-28-2025
// This code is under Public Domain License.
// Handle ADC reading and return a compensated value, it will compensate for the
// non liner ADC of the ESP32

#include "adcread.h"
#include "hw.h"
#include <Arduino.h>

// Function for the linear adjustment of the ADC.
double analogReadAdjusted(byte pinNumber) {

  // Specify the adjustment factors.
  const double f1 = 1.7111361460487501e+001;
  const double f2 = 4.2319467860421662e+000;
  const double f3 = -1.9077375643188468e-002;
  const double f4 = 5.4338055402459246e-005;
  const double f5 = -8.7712931081088873e-008;
  const double f6 = 8.7526709101221588e-011;
  const double f7 = -5.6536248553232152e-014;
  const double f8 = 2.4073049082147032e-017;
  const double f9 = -6.7106284580950781e-021;
  const double f10 = 1.1781963823253708e-024;
  const double f11 = -1.1818752813719799e-028;
  const double f12 = 5.1642864552256602e-033;

  // Specify the number of loops for one measurement.
  const int loops = 20;

  // Specify the delay between the loops.
  const int loopDelay = 100;

  // Initialize the used variables.
  int counter = 1;
  int inputValue = 0;
  double totalInputValue = 0;
  double averageInputValue = 0;
  /* This function handles the server callbacks */

  // Loop to get the average of different analog values.
  for (counter = 1; counter <= loops; counter++) {

    // Read the analog value.
    inputValue = analogRead(pinNumber);

    // Add the analog value to the total.
    totalInputValue += inputValue;

    // Wait some time after each loop.
    delay(loopDelay);
  }

  // Calculate the average input value.
  averageInputValue = totalInputValue / loops;

  // Calculate and return the adjusted input value.
  return f1 + f2 * pow(averageInputValue, 1) + f3 * pow(averageInputValue, 2) +
         f4 * pow(averageInputValue, 3) + f5 * pow(averageInputValue, 4) +
         f6 * pow(averageInputValue, 5) + f7 * pow(averageInputValue, 6) +
         f8 * pow(averageInputValue, 7) + f9 * pow(averageInputValue, 8) +
         f10 * pow(averageInputValue, 9) + f11 * pow(averageInputValue, 10) +
         f12 * pow(averageInputValue, 11);
}