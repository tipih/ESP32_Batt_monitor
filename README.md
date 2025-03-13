# ESP32_Batt_monitor
ESP32 low power batteri computer
This system can be used in LIPO Bike batt with a advance BMS, where the BMS has been replaced with a new cheep one.
It will enable autolight to the build in backlight, as well as have the Batt indicator work
My current setup is using 470 + 56 kohm voltage divider to split voltage from mas 42 volt down to max 3.3 volt, that is the ESP32 ADC ref input
This can most likely be optimised even futher, in order to bring down the leak curret even futher, current setup uses approx. 80 uA

