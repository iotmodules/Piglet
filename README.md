# Piglet - IOT Development Module

![Piglet 1.5](https://github.com/iotmodules/Piglet/blob/master/Hardware/Images/Piglet_v_1_5_8.jpg?raw=true "Piglet v1.5")

#Features of the Piglet IOT development board
 
 * ATMega328-au fully compatible with the Arduino IDE
 * Support for Optiboot and DualOptiboot Bootloaders
 * Extremeley low power consumption when asleep typically 8uA
 * External 8Mhz crystal oscillator to allow support for the critical timig required by the LORA radios and the LMIC LoRaWAN Library https://github.com/matthijskooijman/arduino-lmic
 * Precision resistor divider to accurately measure the battery voltage before the regulator
 * Real time clock with temperature compensation to provide hi level of accuaracy for time critical functions such as scheduling of transmissions and timestamping events. 
 * Support for HopeRF HCW69HCW and RFM95 Lora radios 
 * On board EUI-64 unique ID accesible over the I2C bus
 * On board 4Mbit SPI flash for configuration data storage and code updates using DualOptiboot 
 * Precision voltage regulator with quiescent current of only 1ua allowing supply voltages between 2.8v and 6.0v
 * U.FL connector for connection of a range of antenna options

#Licence

CC-BY-SA 4.0 https://creativecommons.org/licenses/by-sa/4.0/ You are free to share and adapt. But you need to give attribution and use the same license to redistribute.
