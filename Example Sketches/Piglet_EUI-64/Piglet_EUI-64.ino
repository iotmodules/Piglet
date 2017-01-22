/**
 * This Arduino sketch is a demonstration of the EUI-64 unique ID feature of the Piglet IOT development board
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
 *  <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
 * 
 * Created by Doug Mahy <doug.mahy@gmail.com>
 * Copyright (C) 2017  Doug Mahy 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <Wire.h>
#define I2CADDRESS 0x50
#define EUI64 0xf8
#define PIGLET_I2C_PULLUP_PIN 7

typedef struct {
   size_t len;
   uint8_t *bytes;
} vdata;

void setup(void)
{
  //low speed serial to allow for piglets using internal clock
  Serial.begin(9600);
  //initialise I2C
  Wire.begin();
}

void loop(void)
{
  String eui = getEUI64();
  Serial.print("This Piglet's EUI-64 is (");
  Serial.print(eui);
  Serial.print(")");
  Serial.println();
  delay(1000);
}

String getEUI64(){
    //enable pullups on piglet
    pinMode(PIGLET_I2C_PULLUP_PIN,OUTPUT);
    digitalWrite(PIGLET_I2C_PULLUP_PIN,HIGH);
    Wire.beginTransmission(I2CADDRESS);
    Wire.write(EUI64);
    Wire.endTransmission();
    Wire.requestFrom(I2CADDRESS, 8);

    byte *eui64;eui64 = (byte *)malloc(8);
    int i=0;
    while (Wire.available()){
     eui64[i] = Wire.read();i=i+1;
    }
    vdata inData; inData.len = 8; inData.bytes = eui64;
    return String(vdata_get_hex(inData));
    digitalWrite(PIGLET_I2C_PULLUP_PIN,LOW);
}

char* vdata_get_hex(const vdata data)
{
   char hex_str[]= "0123456789abcdef";

   char* out;
   out = (char *)malloc(data.len * 2 + 1);
   (out)[data.len * 2] = 0;
   
   if (!data.len) return NULL;
   
   for (size_t i = 0; i < data.len; i++) {
      (out)[i * 2 + 0] = hex_str[(data.bytes[i] >> 4) & 0x0F];
      (out)[i * 2 + 1] = hex_str[(data.bytes[i]     ) & 0x0F];
   }
   return out;
}


  
