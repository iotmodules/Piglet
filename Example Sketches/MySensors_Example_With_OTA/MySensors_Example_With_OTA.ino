
/**
 * This Arduino sketch is a demonstration of the features of the Piglet IOT development board
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
 * 
 * 
 * 
 * This example sketch relies heavily on the MySensors Arduino Library created by Tomas Hozza <thozza@gmail.com>
 * 
 * These Libraries can be obtained here https://github.com/mysensors/MySensors
 * 
 * This sketch also requires the libraries indicated below
 *  <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
 *  <TimeLib.h>      //http://playground.arduino.cc/Code/Time
 *  <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
 * 
 * Created by Doug Mahy <doug.mahy@gmail.com>
 * Copyright (C) 2017  Doug Mahy 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

// Enable debug prints
#define MY_DEBUG

#define MY_BAUD_RATE 38400

#define SKETCH_NAME "Device id not set"
#define SKETCH_MAJOR_VER "0"
#define SKETCH_MINOR_VER "10"

#define MY_RADIO_RFM69
#define MY_IS_RFM69HW
#define MY_RFM69_FREQUENCY RF69_433MHZ  
#define MY_RFM69_NETWORKID     101
#define MY_RF69_IRQ_NUM 0
#define MY_RF69_IRQ_PIN 2

#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS 1000

#define MY_OTA_FIRMWARE_FEATURE        // Enables OTA (over the air) capability
#define MY_OTA_FLASH_SS 8               // Flash CS (chip select) pin used
#define MY_OTA_FLASH_JDECID 0x1F84
#define OTA_WAIT_PERIOD 200            // Period the loop is paused the loop whilst OTA transmissions are checked
#define FLASH_CONFIG_DATA_LOCATION 0xFFFF0000   

#define I2CADDRESS 0x50
#define EUI64 0xf8
#define PIGLET_I2C_PULLUP_PIN 7

#define INTERRUPT_PIN 3 // interrupt for RTC
#define RTC_CLOCK_PIN 7 //power for the RTC
#define WAKEINTERVAL 5  //interval between wake up times this sets the interval of the RTC alarm

#define LED_PIN 9 //onboard LED

#define SENSOR_DATA_ID  1 
#define SENSOR_DATA_DESCRIPTION  "Piglet Sensor Data"

#include <math.h>
#include <DS3232RTC.h>                  //http://github.com/JChristensen/DS3232RTC
#include <Wire.h>                       //http://arduino.cc/en/Reference/Wire
#include <TimeLib.h>                    //http://playground.arduino.cc/Code/Time
#include <SPI.h>
#include <MySensors.h>

// Some globals 
long lastTime;
long lastRecievedTime;

char cbuf[255];// buffer for bytes to read from memory
byte buf[255]; //buffer for bytes to writ to memory

bool fwUpdateOngoing;
bool transmission_occured = true;
bool doSleep = true;

typedef struct {
   size_t len;
   uint8_t *bytes;
} vdata;

typedef struct { //this structure is to hold the results of the conversion from seconds to hours mins and seconds
   int tHours;
   int tMinutes;
   int tSeconds;
} t_hms;

MyMessage nodeData(SENSOR_DATA_ID, V_CUSTOM);

void setup(void)
{  
  
  pinMode(SCL, INPUT);          //setting these as inputs reduces the power used by 70uA
  pinMode(SDA, INPUT);          //setting these as inputs reduces the power used by 70uA
  pinMode(LED_PIN, OUTPUT);

  // make sure the flash is sleeping to save power. Investigate if needed though
  if (!_flash.initialize()){
    sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER); 
  }
  else{
    _flash.sleep();
  }
  
  analogReference(EXTERNAL);    //uses the external 2.7v voltage reference from the texas regulator ths is very accurate
  initAlarm(); //initialise the alarm

}

void presentation(){
    // Send the Sketch Version Information to the Gateway
    sendSketchInfo(getEUI64(), SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);
    // Register all sensors to gw (they will be created as child devices)
    present(SENSOR_DATA_ID, S_CUSTOM, SENSOR_DATA_DESCRIPTION, false);// Register all sensors to gw (they will be created as single child device)
    
    requestTime();  //get the server time to update the onboard RTC
    wait(400);  // needed to wait for time to be returned
}

void receiveTime(unsigned long time) {  // This is called when a new time value was received
  lastRecievedTime = time;
}

void receive(const MyMessage &message) {

  if ((message.command_ack_payload == C_SET) && (message.type==V_TEXT)) {
    
       if (_flash.initialize()){ }
       String mystring;
       _flash.blockErase4K(FLASH_CONFIG_DATA_LOCATION);
       while(_flash.busy());
       mystring = message.getString();
      
       mystring.getBytes(buf,255);
       _flash.writeBytes(FLASH_CONFIG_DATA_LOCATION,buf,255);
       while ( _flash.busy() ) {}
       _flash.sleep();
       presentation();
     
  }
}


void loop() {
  if (fwUpdateOngoing) {
    fwUpdateOngoing = false;
    fwUpdateOngoing = wait(OTA_WAIT_PERIOD, C_STREAM, ST_FIRMWARE_RESPONSE);
  } else {
    normalFlow();
  }
}


void normalFlow(void) {

  digitalWrite(LED_PIN, HIGH);
  if (lastTime != lastRecievedTime){  //update time if required
    writeTime(lastRecievedTime);
    lastTime = lastRecievedTime;
  }
  setAlarm(WAKEINTERVAL); //set alarm to trigger again in x seconds time
  
  float voltage =  getBatteryVoltage();   //get the battery voltage convert adc value to voltage using internal reference and bridge ratio
  uint8_t batLevel = (1-((4.1-voltage)/(4.1-2.8)))*100; //do conversion to % range 4.1 full 2.8 empty
 
  String data = String(batLevel) + "," + String(voltage) +  "," +  String(getTemp())+  "," +  String(getTime());
  char buf[26];data.toCharArray(buf, 26);
  
  send(nodeData.set(buf));

  //wait(200);
  //sendHeartbeat();
  if (doSleep){
    
    //wait(100); // now done with fwUpdateOngoing lne instead
    fwUpdateOngoing = wait(OTA_WAIT_PERIOD, C_STREAM, ST_FIRMWARE_RESPONSE); // also acts as delay to wait for other internal messages to arrive such as reboot and present before sleeping
    
    digitalWrite(LED_PIN, LOW);
    sleep(digitalPinToInterrupt(INTERRUPT_PIN),FALLING,0); 
    //smartSleep(digitalPinToInterrupt(INTERRUPT_PIN),FALLING,0);
  
  }
  else {
    digitalWrite(LED_PIN, LOW);
    wait(10000);
  }
}





//====================================================================================================================================================//


float getBatteryVoltage(){

  //convert adc value to voltage using internal reference and bridge ratio
  int aValue = analogRead(A6);
  // need to check all delays and reduce to minimums  
  //delay(100);
  float ref = ((2.7/1023.0000)*(aValue));
  float correction = 4.24/4.24;
  ref = ref*correction;
  float ratio = ((1000.00+330.00)/330.00);
  float bVoltage = ref*ratio;
  return bVoltage;

}

char* getEUI64(){
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
    return vdata_get_hex(inData);
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

bool rtcTrigger = true; //setup trigger initially to get the alarm going

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

void writeTime(long timeInSeconds){
  digitalWrite(RTC_CLOCK_PIN, HIGH);  
  RTC.set(timeInSeconds);
  digitalWrite(RTC_CLOCK_PIN, LOW);  
}

float getTemp(){

  digitalWrite(RTC_CLOCK_PIN, HIGH); 
  int tp = RTC.temperature(); //get the temp
  float celsius = tp / 4.0; //do conversion to degrees centigrade
  digitalWrite(RTC_CLOCK_PIN, HIGH); 
  return celsius;
  
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

                             
