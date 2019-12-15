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


// the UI controller /////////////////////////////////////////////////////////
UIController *uiCont;

//instantiate some instance variables
int loopIter = 0;        // loop slices
float fadeMax = 255.0;
int bright = 255;
int col;
int fadeVal;
unsigned long timeNow = 0;
unsigned long timeLast = 0;
const byte BM_I2Cadd   = 0x6b; // the chip lives here on I²C
const byte BM_Status   = 0x08; // system status register

#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  7        //Time ESP32 will go to sleep (in seconds)

// globals for a wifi access point and webserver ////////////////////////////
String apSSID = String("Pro+UpdThing-"); // SSID of the AP
String apPassword = _DEFAULT_AP_KEY;     // passkey for the AP

//Time start Settings:
const char* ntpServer = "0.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

//NEOPIXEL LEDS SHIT
#define PIN A7
#define NUM_PIXELS 32
#define BRIGHTNESS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

char date_string[50]; //50 chars should be enough
char time_string[50]; //50 chars should be enough

//gets and print local time, returns failed if no time found.
//also fill date_string and time_string with relevant info
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  strftime(date_string, sizeof(date_string), "%A, %B %d %Y", &timeinfo);
  strftime(time_string, sizeof(time_string), "%H:%M:%S", &timeinfo);
  setAlarmTime();
}

void setAlarmTime() {
  time_t now;
  struct tm alarmTime;
  double seconds;
  time(&now);
  alarmTime = *localtime(&now);
  alarmTime.tm_sec = 0;
  alarmTime.tm_mon = 0;
  alarmTime.tm_mon = 11;
  alarmTime.tm_hour = 0;
  alarmTime.tm_min = 2;
  alarmTime.tm_mday = 16;
  alarmTime.tm_year = 119;
  seconds = difftime(mktime(&alarmTime),now);
  printf("%.f seconds from alarm.\n", seconds);
}

// SETUP: initialisation entry point /////////////////////////////////////////
void setup() {
  UNPHONE_DBG = true;
  unPhone::begin();
  getMAC(MAC_ADDRESS);          // store the MAC address
  apSSID.concat(MAC_ADDRESS);   // add the MAC to the AP SSID

  Serial.printf("\nsetup...\nESP32 MAC = %s\n", MAC_ADDRESS);
  // power management

  //....
  unPhone::printWakeupReason(); // what woke us up?
  unPhone::checkPowerSwitch();  // if power switch is off, shutdown


  //Reset pixels
  pixels.setBrightness(1);
  pixels.begin();
  pixels.show(); // This sends the updated pixel color to the hardware.
  fadeVal = 0;


  //Connect to save wifi, if none start AP
  Serial.printf("doing wifi manager\n");
  joinmeManageWiFi(apSSID.c_str(), apPassword.c_str()); // get net connection
  Serial.printf("wifi manager done\n\n");


  //GET current time and print to serial line
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();


  //Show Alarm User interface
  uiCont = new UIController(ui_alarm);
  if(!uiCont->begin()) {
    E("WARNING: ui.begin failed!\n")
  }

}


void loop() {
  D("\nentering main loop\n")
  while(1) {
    micros(); // update overflow

    //if power switch on -> do usual activities
    if (powerOn()) {
      dawnAlarm();
    //else check if usb connected and then do such and such...
    } else if (!powerOn()){
      powerMode();
    }

    //print time to serial line
    printLocalTime();

    //If button 2 pressed deep_sleep
    if(unPhone::button2()) {
      //set esp to sleep for x seconds or if it gets woken by button
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
      " Seconds");
      //wake up with button one
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);
      IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
      //start deep sleep
      esp_deep_sleep_start();
    }

    // allow the protocol CPU IDLE task to run periodically
    if(loopIter % 2500 == 0) {
      if(loopIter % 25000 == 0)
        D("completed loop %d, yielding 1000th time since last\n", loopIter)
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
  bool usbConnected = bitRead(unPhone::getRegister(BM_I2Cadd, BM_Status), 2);
  if (!usbConnected) {
    pixelsOff();
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
  //delay = dawntime in miliseconds / 255
  delay(1000);
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

// // send TTN message
// void loraMessage() {
//   /* LoRaWAN keys: copy these values from TTN
//    * register a device and change it to ABP, then copy the keys in msb format
//    * and define them in your private.h, along with _LORA_DEV_ADDR; they'll
//    * look something like this:
//    *   #define _LORA_APP_KEY  { 0xFF, 0xFF, 0xFF, ... }
//    *   #define _LORA_NET_KEY  { 0xFF, 0xFF, 0xFF, ... }
//    *   #define _LORA_DEV_ADDR 0x99999999
//    */
//   u1_t NWKSKEY[16] = _LORA_NET_KEY;
//   u1_t APPSKEY[16] = _LORA_APP_KEY;
//
//   // send a LoRaWAN message to TTN
//   Serial.printf("doing LoRaWAN to TTN...\n");
//   unPhone::lmic_init(_LORA_DEV_ADDR, NWKSKEY, APPSKEY);
//   unPhone::lmic_do_send(&unPhone::sendjob);
//   Serial.printf("...done (TTN)\n");
// }


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
