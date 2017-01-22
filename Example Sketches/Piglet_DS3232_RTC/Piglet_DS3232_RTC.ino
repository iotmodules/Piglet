/**
 * This Arduino sketch is a demonstration of the DS3232RTC real time clock of the Piglet IOT development board
 * It sends the alarm time the time set in setup and the temperature to the serial port showing how each of these 
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
 *  <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
 * 
 * Created by Doug Mahy <doug.mahy@gmail.com>
 * Copyright (C) 2017  Doug Mahy 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
#include <TimeLib.h>      //http://playground.arduino.cc/Code/Time
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <math.h>

#define INTERRUPT_PIN 3 // interrupt for RTC
#define LED_PIN 9 //onboard LED
#define RTC_CLOCK_PIN 7 //power for the RTC
#define WAKEINTERVAL 10 //period between the alarm interrupts

typedef struct { //this structure is to hold the results of the conversion from seconds to hours mins and seconds
   int tHours;
   int tMinutes;
   int tSeconds;
} t_hms;

bool rtcTrigger = true; //setup trigger initially to get the alarm going

void setup() {
  Serial.begin(38400);
  initAlarm(); //initialise the alarm
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), rtcInterrupt, FALLING); //interrupt to fire when alarm time reached
  writeTime(1485094870); //set the time seconds since Jan 01 1970. (UTC)
}

void loop() { 
  if (rtcTrigger){
    Serial.print("Current time set on piglet : ");Serial.println(String(getTime())); //print time set to serial
    t_hms ts = setAlarm(WAKEINTERVAL); //set alarm to trigger again in x seconds time
    Serial.print("Next alarm interrupt at    : ");Serial.print(ts.tHours);Serial.print(":");Serial.print(ts.tMinutes);Serial.print(":");Serial.println(ts.tSeconds);
    Serial.print("Temperature                : ");Serial.println(String(getTemp()));
    Serial.println();
  }
}

void rtcInterrupt(){
  rtcTrigger = true; //get it back in the loop
}

t_hms setAlarm(long inSecondsTime){
  rtcTrigger = false; //reset the interrupt trigger
  digitalWrite(RTC_CLOCK_PIN, HIGH); //power up the clock
  RTC.alarm(ALARM_1); // reset the alarm
  time_t t = RTC.get();  // get the time
  long totalSeconds = (long(hour(t)) * 60 * 60)+ (long(minute(t)) * 60) + long(second(t)) + inSecondsTime;  //calculate second to wake up on
  t_hms ts = convertHMS(totalSeconds); // calculate wakeup time in hours minutes and seconds
  RTC.setAlarm(ALM1_MATCH_HOURS , ts.tSeconds, ts.tMinutes, ts.tHours, 0); //set the alarm
  digitalWrite(RTC_CLOCK_PIN, LOW); //sleep the rt clock to save power
  return ts;  
}

void initAlarm(){
  
  pinMode(RTC_CLOCK_PIN, OUTPUT);
  digitalWrite(RTC_CLOCK_PIN, HIGH);          // powers up rtc clock
  RTC.squareWave(SQWAVE_NONE);                // no square wave
  RTC.alarmInterrupt(ALARM_2, false);         // disables alarm 2
  RTC.alarmInterrupt(ALARM_1, true);          // This alarm has resolution in seconds
  RTC.setAlarm(ALM1_MATCH_HOURS, 0, 0, 0, 0); // match alarm when hours mins and seconds match so longest sleep duration is 24 hours
  digitalWrite(RTC_CLOCK_PIN, LOW);           // puts the RTC into sleep mode and saves power
  
}

t_hms convertHMS(long tSec){
   if (tSec >= 86400) tSec = tSec - 86400;
   int remainder = tSec % (60*60);
   int tHours = (tSec - (remainder))/(60*60);
   int tSeconds = remainder % 60;
   int tMinutes = (remainder - tSeconds)/60;
   t_hms out;
   out.tHours = tHours;
   out.tMinutes = tMinutes;
   out.tSeconds = tSeconds;
   return out;
}

char* getTime(){

  char timeBuff[30];
  digitalWrite(RTC_CLOCK_PIN, HIGH);  //turn on real time clock dont forget to turn on before using
  time_t t = RTC.get(); //get the time
  timeFormat(t).toCharArray(timeBuff, 30);  //format into a readable time and date
  digitalWrite(RTC_CLOCK_PIN, LOW); // puts the RTC into sleep mode and saves power
  return timeBuff;
  
}

float getTemp(){

  digitalWrite(RTC_CLOCK_PIN, HIGH); 
  int tp = RTC.temperature(); //get the temp
  float celsius = tp / 4.0; //do conversion to degrees centigrade
  digitalWrite(RTC_CLOCK_PIN, HIGH); 
  return celsius;
  
}

void writeTime(long timeInSeconds){
  digitalWrite(RTC_CLOCK_PIN, HIGH);  
  RTC.set(timeInSeconds);
  digitalWrite(RTC_CLOCK_PIN, LOW);  
}

String timeFormat(time_t t) {
  String timeString = String(hour(t)) + printDigits(minute(t)) + printDigits(second(t)) + " " + String(day(t)) + " " + String(month(t)) + " " + String(year(t));
  return timeString;
}

String printDigits(int digits) {
  String x = ":";
  if (digits < 10) {
    x = x + "0";
  }
  x = x + String(digits);
  return x;
}
