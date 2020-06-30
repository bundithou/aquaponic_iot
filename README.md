# Aquaponic_iot

A collection of codes for aquaponic system project.

## Running code

[system_SD_RTC_online_control](https://github.com/bundithou/aquaponic_iot/tree/master/system/system_SD_RTC_online_control)
A system with RTC and SD Card module for data logging, and online controlling function. The code was uploaded onto an ARDUINO MEGA 2560.

[esp32tomqtt.ino](https://github.com/bundithou/aquaponic_iot/tree/master/esp32/esp32_arduino/esp32tomqtt/esp32tomqtt)
ESP32 code, actually used on an Nano32. Function as an middleman between the server and the arduino.

Consists of both monitoring and controlling over DO, pH, water level, soil moisture, and temperature.

Controlling the system using water pump, air pump, and valves. Thus able to control only the soil moisture and DO though.

## Module tester

Almost all module are test using codes in [here](https://github.com/bundithou/aquaponic_iot/tree/master/codeForEachModule).
An example from [Aqualib Library](https://github.com/bundithou/aquaponic_iot/tree/master/Aqualib) also work for module testing as well.
