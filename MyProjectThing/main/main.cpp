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

#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// libraries for projects; comment out as required
#include <Adafruit_VS1053.h>      // the audio chip
#include <RCSwitch.h>             // 433 MHz remote switching
#include <DHTesp.h>               // temperature / humidity sensor
#include <GP2Y1010_DustSensor.h>  // the Sharp dust sensor
#include <Adafruit_NeoMatrix.h>   // neopixel matrix
#include <Adafruit_NeoPixel.h>    // neopixels generally
#include <Adafruit_MotorShield.h> // the hbridge motor driver
#include <Adafruit_TSL2591.h>     // light sensor

// OTA, MAC address, messaging, loop slicing//////////////////////////////////
int firmwareVersion = 1; // keep up-to-date! (used to check for updates)
char *getMAC(char *);    // read the address into buffer
char MAC_ADDRESS[13];    // MAC addresses are 12 chars, plus the NULL
void flash();            // the RGB LED
void loraMessage();      // TTN
void lcdMessage(char *); // message on screen
int loopIter = 0;        // loop slices

// SETUP: initialisation entry point /////////////////////////////////////////
void setup() {
  UNPHONE_DBG = true;
  unPhone::begin();

  // power management
  unPhone::printWakeupReason(); // what woke us up?
  unPhone::checkPowerSwitch();  // if power switch is off, shutdown

  // flash the internal RGB LED
  flash();

  // LoRaWAN example
  if(false) loraMessage();

  // buzz a bit
  for(int i = 0; i < 3; i++) {
    unPhone::vibe(true);  delay(150); unPhone::vibe(false); delay(150);
  }

  // TODO you might do joinmeManageWiFi and joinmeOTAUpdate here, and/or start
  // a webserver, and/or init any other special hardware you're using

  // say hi, store MAC, blink etc.
  Serial.printf("Hello from MyProjectThing...\n");
  getMAC(MAC_ADDRESS);          // store the MAC address
  Serial.printf("\nsetup...\nESP32 MAC = %s\n", MAC_ADDRESS);
  Serial.printf("battery voltage = %3.3f\n", unPhone::batteryVoltage());
  lcdMessage("hello from MyProjectThing!"); // say hello on screen
  flash(); // flash the internal RGB LED
}

void loop() {
  D("\nentering main loop\n")
  while(1) {
    micros(); // update overflow

    // TODO do your stuff here...

    unPhone::checkPowerSwitch(); // if power switch is off shutdown

    // allow the protocol CPU IDLE task to run periodically
    if(loopIter % 2500 == 0) {
      if(loopIter % 25000 == 0)
        D("completed loop %d, yielding 1000th time since last\n", loopIter)
      delay(100); // 100 appears min to allow IDLE task to fire
    }
    loopIter++;

    // register button presses
    if(unPhone::button1()) Serial.println("button1");
    if(unPhone::button2()) Serial.println("button2");
    if(unPhone::button3()) Serial.println("button3");
  }
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

// message on LCD
void lcdMessage(char *s) {
  unPhone::tftp->setCursor(0, 465);
  unPhone::tftp->setTextSize(2);
  unPhone::tftp->setTextColor(HX8357_CYAN, HX8357_BLACK);
  unPhone::tftp->print("                          ");
  unPhone::tftp->setCursor(0, 465);
  unPhone::tftp->print(s);
}

// send TTN message
void loraMessage() {
  /* LoRaWAN keys: copy these values from TTN
   * register a device and change it to ABP, then copy the keys in msb format
   * and define them in your private.h, along with _LORA_DEV_ADDR; they'll
   * look something like this:
   *   #define _LORA_APP_KEY  { 0xFF, 0xFF, 0xFF, ... }
   *   #define _LORA_NET_KEY  { 0xFF, 0xFF, 0xFF, ... }
   *   #define _LORA_DEV_ADDR 0x99999999
   */
  u1_t NWKSKEY[16] = _LORA_NET_KEY;
  u1_t APPSKEY[16] = _LORA_APP_KEY;

  // send a LoRaWAN message to TTN
  Serial.printf("doing LoRaWAN to TTN...\n");
  unPhone::lmic_init(_LORA_DEV_ADDR, NWKSKEY, APPSKEY);
  unPhone::lmic_do_send(&unPhone::sendjob);
  Serial.printf("...done (TTN)\n");
}

// cycle the LED
void flash() {
  unPhone::rgb(0, 0, 0); delay(300); unPhone::rgb(0, 0, 1); delay(300);
  unPhone::rgb(0, 1, 1); delay(300); unPhone::rgb(1, 0, 1); delay(300);
  unPhone::rgb(1, 1, 0); delay(300); unPhone::rgb(1, 1, 1); delay(300);
}
