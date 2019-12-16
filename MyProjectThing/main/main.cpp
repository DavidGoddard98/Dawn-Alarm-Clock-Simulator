// main.cpp / sketch.ino

// a library or two... ///////////////////////////////////////////////////////
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include "private.h" // stuff not for checking in
#include "unphone.h"
#include "IOExpander.h"
#include "UIController.h"

// a library or two... //////////////////////////////////////////////////////
#include <ESPAsyncWebServer.h>
#include "joinme.h"

//time
#include "time.h"

//power consumption
#include "driver/rtc_io.h"

#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// libraries for projects; comment out as required
#include <Adafruit_VS1053.h>      // the audio chip
#include <Adafruit_NeoMatrix.h>   // neopixel matrix
#include <Adafruit_NeoPixel.h>    // neopixels generally
#include <Adafruit_TSL2591.h>     // light sensor

// define methods//
char *getMAC(char *);    // read the address into buffer
char MAC_ADDRESS[13];    // MAC addresses are 12 chars, plus the NULL
void lcdMessage(char *); // message on screen
void pixelsOff();
bool powerOn();
void powerMode();
void setAlarmTime();
void dawnAlarm();

//TIME FUNCTIONS
#include <soc/rtc.h>
extern "C" {
  #include <esp_clk.h>
}

// the UI controller /////////////////////////////////////////////////////////
UIController *uiCont;

//instantiate some instance variables
int loopIter = 0;        // loop slices
float fadeMax = 255.0;
int bright = 255;
int col;
int fadeVal;
const byte BM_I2Cadd   = 0x6b; // the chip lives here on I²C
const byte BM_Status   = 0x08; // system status register
bool alarmSet = false;
bool snooze = false;
int dawnTime = 240000000;
int time_check_hour =0;
double seconds;
unsigned long timer = micros();


#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  7        //Time ESP32 will go to sleep (in seconds)
#define sleepPeriod 10000000ul  // 10 seconds sleep


// globals for a wifi access point and webserver ////////////////////////////
String apSSID = String("Pro+UpdThing-"); // SSID of the AP
String apPassword = _DEFAULT_AP_KEY;     // passkey for the AP

//Time start Settings:
const char* ntpServer = "0.pool.ntp.org";

//offsets
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

//NEOPIXEL LEDS SHIT
#define PIN A7
#define NUM_PIXELS 32
#define BRIGHTNESS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);


//TODO SAVE DATA OVER BOOT IN RCU MEMORY
RTC_DATA_ATTR byte bootCountt = 0;
RTC_DATA_ATTR time_t time_now;
RTC_DATA_ATTR struct tm * timeinfo;
RTC_DATA_ATTR int old_milis;
RTC_DATA_ATTR uint64_t sleepTime;
RTC_DATA_ATTR uint64_t offset_time;
RTC_DATA_ATTR struct tm alarmTime;


uint64_t timeDiff, timeNow;

//gets and print local time, returns failed if no time found.
//also fill date_string and time_string with relevant info
void printLocalTime()
{
  time(&time_now);
  timeinfo = localtime (&time_now);
  Serial.printf ("%s\n", asctime(timeinfo));
}

void setAlarmTime() {
  time(&time_now);
  alarmTime = *localtime(&time_now);
  alarmTime.tm_sec = 0;
  alarmTime.tm_mon = 0;
  alarmTime.tm_mon  = 11;
  alarmTime.tm_hour = 4;
  alarmTime.tm_min = 16;
  alarmTime.tm_mday = 16;
  alarmTime.tm_year = 119;
}

void updateTime(uint64_t time_diff) {
  //#secs
  int seconds = floor((time_diff / 1000000));
  //#milis
  int milis = floor(time_diff % 1000000);

  if (milis+old_milis > 1000) {
    //if milis more than 1000 add second to time
    seconds += 1;
    //make up the difference to save the rest of the milis which didnt make the 1 sec
    old_milis = (old_milis + milis) -1000 ;
  } else {
    old_milis += milis;
  }
  time_now = time_t(time_now) + seconds;
}

void hourCheck(uint64_t time_since_start) {
  time_check_hour = time_since_start/3600000000;
}

double time2Alarm() {
  seconds = difftime(mktime(&alarmTime),time_now);
  return seconds;
}

void vibrate() {
  unPhone::vibe(true);  delay(150);
  unPhone::vibe(false); delay(150);
}

// SETUP: initialisation entry point /////////////////////////////////////////
void setup() {
  UNPHONE_DBG = true;
  unPhone::begin();
  getMAC(MAC_ADDRESS);          // store the MAC address
  apSSID.concat(MAC_ADDRESS);   // add the MAC to the AP SSID


  //....
  unPhone::printWakeupReason(); // what woke us up?
  unPhone::checkPowerSwitch();  // if power switch is off, shutdown

  //Reset pixels
  pixels.setBrightness(1);
  pixels.begin();
  pixels.show(); // This sends the updated pixel color to the hardware.
  fadeVal = 0;

  //first boot so connect to wifi to get time
  if (bootCountt == 0 ) {
    //Connect to save wifi, if none start AP
    Serial.printf("doing wifi manager\n");
    joinmeManageWiFi(apSSID.c_str(), apPassword.c_str()); // get net connection
    Serial.printf("wifi manager done\n\n");
    //GET current time and print to serial line
    int year;
    while( year < 118) {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      delay(1000);
      time(&time_now);
      struct tm yearCheck;
      yearCheck = *localtime(&time_now);
      year = yearCheck.tm_year;
      // WiFi_Disconnect();
      // delay(200); // do not remove, need a delay for disconnect to cha nge status()

    }
    bootCountt++;
    // wm.disconnect();

    offset_time = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
  }


  timeNow = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;
  timeDiff = timeNow - sleepTime;
  updateTime(timeDiff);
  timeinfo = localtime (&time_now);

  hourCheck(timeNow);
  if(time_check_hour >= 2) {
    bootCountt =0;
    ESP.restart();
  }

  Serial.printf ("%s\n", asctime(timeinfo));


  //Show Alarm User interface
  uiCont = new UIController(ui_home);
  if(!uiCont->begin()) {
    E("WARNING: ui.begin failed!\n")
  }
}

void stopAlarm() {
  pixelsOff();
  alarmTime.tm_year = 130;
}

void snoozeAlarm() {
  alarmTime.tm_sec += 300 ;
  pixelsOff();
}

bool alarm_on = false;

void loop() {
  D("\nentering main loop\n")
  unsigned long int timer_2 = micros();
  while(1) {

    uiCont->run();

    if (!alarmSet) {
      setAlarmTime();
      alarmSet = true;
    }

    //Start alarm
    if (time2Alarm() <= dawnTime/1000000 ) {
      if (micros() - timer >= (dawnTime/255)) {
        Serial.print("here");
        dawnAlarm();
        timer = micros();
      }
      if (time2Alarm() <= 0 ) {
        alarm_on = true;
        vibrate();
      }
    }
    if (time2Alarm() > 800 && (micros() - timer_2 >= 30000000)) {
      //wake up with button one
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);
      //Set timer to 5 seconds
      IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
      // digitalWrite(GPIO_NUM_36, LOW);
      pixelsOff();
    	esp_sleep_enable_timer_wakeup((time2Alarm() / 2) * uS_TO_S_FACTOR);
    	Serial.println("Setup ESP32 to sleep for every " + String((time2Alarm() / 2)) +" Seconds");
      esp_deep_sleep_start();

    }

    //else check if usb connected and then do such and such...
    if (!powerOn()){
      powerMode();
    }
    if(unPhone::button3() && alarm_on) {
      alarm_on = false;
      snoozeAlarm();
    }
    if(unPhone::button1() && alarm_on) {
      alarm_on = false;
      stopAlarm();
    }

    //If button 2 pressed deep_sleep
    if(unPhone::button2()) {

      //wake up with button one
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);

      //on sleep = power options
      IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
      // digitalWrite(GPIO_NUM_36, LOW);
      pixelsOff();
      //start deep sleep
      sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;
      esp_deep_sleep_start();
    }
    printLocalTime();
    printf("%.f seconds from alarm.\n", seconds);
    Serial.print(loopIter);
    // allow the protocol CPU IDLE task to run periodically
    if(loopIter % 2500 == 0) {
      printLocalTime();

      if(loopIter % 25000 == 0) {
        D("completed loop %d, yielding 1000th time since last\n", loopIter);
        printf("%.f seconds from alarm.\n", seconds);
      }

      delay(100); // 100 appears min to allow IDLE task to fire
    }
    loopIter++;

  }
}

//check power switch - rewrote method to get switch state as a return
bool powerOn() {
  uint8_t inputPwrSw = IOExpander::digitalRead(IOExpander::POWER_SWITCH);
  bool power;
  if (inputPwrSw) {
    power = true;
  } else {
    power = false;
  }
  return power;
}

//other part of power switch. - Checks usb connection and then does the same as
//the original method
void powerMode(){
  //set bootcountt to 0 so that time is fetched next time device turned on

  bool usbConnected = bitRead(unPhone::getRegister(BM_I2Cadd, BM_Status), 2);
  if (!usbConnected) {
    pixelsOff();
    bootCountt = 0;
    sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;

    unPhone::setShipping(true);
  } else {
    pixelsOff();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0); // 1 = High, 0 = Low

    // cludge: LCD (and other peripherals) will still be powered when we're
    // on USB; the next call turns the LCD backlight off, but would be
    // preferable if we could cut the 5V to all but the BM (which needs it
    // for charging)...?
    IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
    // deep sleep, wait for wakeup on GPIO
    sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;
    esp_deep_sleep_start();
  }
}

//used to turn pixels off when esp32 is turned off or sleep mode
void pixelsOff() {
  for(int i=0; i<NUM_PIXELS; i++) {

    // pixels.Color takes RGB values, from 0,00 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show(); // This sends the updated pixel color to the hardware.

}

//fade neopixel in
void dawnAlarm() {
  col = bright * float(fadeVal/fadeMax);
  for(uint16_t i=0; i< NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    pixels.setBrightness(col);
  }

  Serial.println("col");
  Serial.println(col);
  Serial.println("pixel brighness");
  Serial.println(pixels.getBrightness());
  //First loop, fade in!
  //increase brightness slowly to fade
  if(fadeVal < fadeMax) {
    fadeVal++;
  }
  pixels.show();

}

// message on LCD
void lcdMessage(char *s) {
  unPhone::tftp->setCursor(0, 465);
  unPhone::tftp->setTextSize(2);
  unPhone::tftp->setTextColor(HX8357_CYAN, HX8357_BLACK);
  unPhone::tftp->print("                          ");
  unPhone::tftp->setCursor(0, 465);
  unPhone::tftp->print(s);
}

// misc utilities ////////////////////////////////////////////////////////////
// get the ESP's MAC address
char *getMAC(char *buf) { // the MAC is 6 bytes; needs careful conversion...
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
  return buf;
}
// cycle the LED
void flash() {
  unPhone::rgb(0, 0, 0); delay(300); unPhone::rgb(0, 0, 1); delay(300);
  unPhone::rgb(0, 1, 1); delay(300); unPhone::rgb(1, 0, 1); delay(300);
  unPhone::rgb(1, 1, 0); delay(300); unPhone::rgb(1, 1, 1); delay(300);
}
