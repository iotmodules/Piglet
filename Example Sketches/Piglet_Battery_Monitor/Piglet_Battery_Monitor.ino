/**
 * This Arduino sketch is a demonstration of the battery monito of the Piglet IOT development board
 * It sends the battery voltage and the battery percentage to the serial port showing how each of these 
 * functions are used.
 * 
 * Other capabilities of this board are not demonstrated in this code but are as follows
 * 
 * ATMega328-au fully compatible with the Arduino IDE
 * 
 * Support for Optiboot and DualOptiboot Bootloaders
 * 
 * Extremeley low power consumption when asleep typically 8uA
 * 
 * External 8Mhz crystal oscillator to allow support for the critical timig required by the 
 * LORA radios and the LMIC LoRaWAN Library https://github.com/matthijskooijman/arduino-lmic
 * 
 * Precision resistor divider to accurately measure the battery voltage before the regulator
 * 
 * Real time clock with temperature compensation to provide hi level of accuaracy for time 
 * critical functions such as scheduling of transmissions and timestamping events.
 * 
 * Support for HopeRF HCW69HCW and RFM95 Lora radios 
 * 
 * On board EUI-64 unique ID accesible over the I2C bus
 * 
 * On board 4Mbit SPI flash for configuration data storage and code updates using DualOptiboot
 *  
 * Precision voltage regulator with quiescent current of only 1ua allowing supply voltages between 2.8v and 6.0v
 * 
 * U.FL connector for connection of a range of antenna options
 * 
 * Created by Doug Mahy <doug.mahy@gmail.com>
 * Copyright (C) 2017  Doug Mahy 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#define MAX_VOLTAGE 4.1 //max voltage battery is charged to usually 4.1v or 4.2 for a lipo
#define MIN_VOLTAGE 2.8 //min voltage battery should be discharged to usually 2.8v absolute min for lipo typically 3.0v

void setup() {
  Serial.begin(38400);
}

void loop() {
  float voltage =  getBatteryVoltage();   //get the battery voltage convert adc value to voltage using internal reference and bridge ratio
  uint8_t batLevel = (1-((MAX_VOLTAGE-voltage)/(MAX_VOLTAGE-MIN_VOLTAGE)))*100; //do conversion to % range 4.1 full 2.8 empty
  Serial.print("Battery Voltage    : ");Serial.print(String(voltage));Serial.println("v");
  Serial.print("Battery Percentage : ");Serial.print(String(batLevel));Serial.println("%");
  Serial.println();
  delay(5000);
}

float getBatteryVoltage(){
  //convert adc value to voltage using internal reference and bridge ratio
  int aValue = analogRead(A6);
  float ref = ((2.7/1023.0000)*(aValue));
  float correction = 4.24/4.24;
  ref = ref*correction;
  float ratio = ((1000.00+330.00)/330.00);
  float bVoltage = ref*ratio;
  return bVoltage;
}
