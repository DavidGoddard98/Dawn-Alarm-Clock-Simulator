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


#include "NTPClient.h"
#include <WiFiUdp.h>
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;

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
// void loraMessage();      // TTN
void lcdMessage(char *); // message on screen
void pixelsOff();
bool powerOn();
void powerMode();
void screenOff();
void setAlarmTime();

// the UI controller /////////////////////////////////////////////////////////
UIController *uiCont;


std::vector< int >  getTime();
const byte BM_I2Cadd   = 0x6b; // the chip lives here on I²C
const byte BM_Status   = 0x08; // system status register

int loopIter = 0;        // loop slices
float fadeMax = 255.0;
int bright = 255;
int col;
int fadeVal;
unsigned long timeNow = 0;
unsigned long timeLast = 0;

#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  7        //Time ESP32 will go to sleep (in seconds)


//internet bits
// we're not in arduino land any more, so need to declare function protos ///
byte hextoi(char c);
void initWebServer();
void hndlRoot(AsyncWebServerRequest *);
void hndlNotFound(AsyncWebServerRequest *);
void hndlWifichz(AsyncWebServerRequest *);
void hndlStatus(AsyncWebServerRequest *);
void hndlWifi(AsyncWebServerRequest *);
void apListForm(String&);
void printIPs();
// globals for a wifi access point and webserver ////////////////////////////
String apSSID = String("Pro+UpdThing-"); // SSID of the AP
String apPassword = _DEFAULT_AP_KEY;     // passkey for the AP
AsyncWebServer* webServer;               // async web server
String ip2str(IPAddress);                // helper for printing IP addresses


// web server utils /////////////////////////////////////////////////////////
// the replacement_t type definition allows specification of a subset of the
// "boilerplate" strings, so we can e.g. replace only the title, or etc.
typedef struct { int position; const char *replacement; } replacement_t;
void getHtml(String& html, const char *[], int, replacement_t [], int);
#define ALEN(a) ((int) (sizeof(a) / sizeof(a[0]))) // only in definition scope!
#define GET_HTML(strout, boiler, repls) \
  getHtml(strout, boiler, ALEN(boiler), repls, ALEN(repls));


//Time start Settings:
const char* ntpServer = "0.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int startingHour = 12; // set your starting hour here, not below at int hour. This ensures accurate daily correction of time
int seconds = 0;
int minutes = 0;
int hours = startingHour;
int days = 0;
//Accuracy settings
int dailyErrorFast = 0; // set the average number of milliseconds your microcontroller's time is fast on a daily basis
int dailyErrorBehind = 0; // set the average number of milliseconds your microcontroller's time is behind on a daily basis
int correctedToday = 1; // do not change this variable, one means that the time has already been corrected today for the error in your boards crystal. This is true for the first day because you just set the time when you uploaded the sketch.
std::vector< int >  current_time;

void dawnAlarm();
void timePassed();

//NEOPIXEL LEDS SHIT
#define PIN A7

#define NUM_PIXELS 32

#define BRIGHTNESS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);


char date_string[50]; //50 chars should be enough
char time_string[50]; //50 chars should be enough
char day[2];


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.println(&timeinfo);
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
  Serial.printf("\nsetup...\nESP32 MAC = %s\n", MAC_ADDRESS);
  // power management
  unPhone::printWakeupReason(); // what woke us up?
  unPhone::checkPowerSwitch();  // if power switch is off, shutdown
  apSSID.concat(MAC_ADDRESS);   // add the MAC to the AP SSID
  // flash the internal RGB LED
  flash();
  pixels.setBrightness(1);
  pixels.begin();
  pixels.show(); // This sends the updated pixel color to the hardware.
  fadeVal = 0;
  // // LoRaWAN example
  // if(false) loraMessage();

  // buzz a bit
  for(int i = 0; i < 3; i++) {
    unPhone::vibe(true);  delay(150); unPhone::vibe(false); delay(150);
  }

  // TODO you might do joinmeManageWiFi and joinmeOTAUpdate here, and/or start
  // a webserver, and/or init any other special hardware you're using
  // say hi, store MAC, blink etc.
  Serial.printf("doing wifi manager\n");
  joinmeManageWiFi(apSSID.c_str(), apPassword.c_str()); // get net connection
  Serial.printf("wifi manager done\n\n");

  //GET current time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // //ORRR
  // timeClient.begin();
  // timeClient.setTimeOffset(3600);

  // //disconnect WiFi as it's no longer needed
  // WiFi.disconnect(true);
  // WiFi.mode(WIFI_OFF);

  uiCont = new UIController(ui_alarm);
  if(!uiCont->begin()) {
    E("WARNING: ui.begin failed!\n")
  }

  printLocalTime();





  flash(); // flash the internal RGB LED


}


void loop() {
  D("\nentering main loop\n")
  while(1) {
    micros(); // update overflow

    if (powerOn()) {
      dawnAlarm();
    } else if (!powerOn()){
      powerMode();
    }
    printLocalTime();





    current_time = getTime();
    Serial.print("secs");
    Serial.print(current_time[0]);
    Serial.print("mins");
    Serial.print(current_time[1]);



    if(unPhone::button2()) {
      //set esp to sleep for 5 seconds or if it gets woken by button
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
      " Seconds");
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);
      IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
      esp_deep_sleep_start();
    }



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

std::vector< int >  getTime() {
  timeNow = millis()/1000; // the number of seconds that have passed since boot
  seconds = timeNow - timeLast;//the number of seconds that have passed since the last time 60 seconds was reached.

  if (seconds == 60) {
    timeLast = timeNow;
    minutes = minutes + 1;
  }

  //if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.

  if (minutes == 60){
    minutes = 0;
    hours = hours + 1;
  }

  // if one hour has passed, start counting minutes from zero and add one hour to the clock

  if (hours == 24){
    hours = 0;
    days = days + 1;
    }

    //if 24 hours have passed , add one day

  if (hours ==(24 - startingHour) && correctedToday == 0){
    delay(dailyErrorFast*1000);
    seconds = seconds + dailyErrorBehind;
    correctedToday = 1;
  }

  //every time 24 hours have passed since the initial starting time and it has not been reset this day before, add milliseconds or delay the progran with some milliseconds.
  //Change these varialbes according to the error of your board.
  // The only way to find out how far off your boards internal clock is, is by uploading this sketch at exactly the same time as the real time, letting it run for a few days
  // and then determine how many seconds slow/fast your boards internal clock is on a daily average. (24 hours).

  if (hours == 24 - startingHour + 2) {
    correctedToday = 0;
  }



  //concert to strings
  // std::string seconds_s = std::to_string(seconds);
  // std::string minutes_s = std::to_string(minutes);
  // std::string hour_s = std::to_string(hours);
  // std::string days_s = std::to_string(days);
  // //append strings]
  std::vector< int > the_time;
  the_time.push_back(seconds);
  the_time.push_back(minutes);
  the_time.push_back(hours);
  the_time.push_back(days);

  return the_time;
}

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

void pixelsOff() {
  for(int i=0; i<NUM_PIXELS; i++) {

    // pixels.Color takes RGB values, from 0,00 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    //if(i > 0) pixels.setPixelColor(i - 1, pixels.Color(0,0,0));

    //delay(delayval); // Delay for a period of time (in milliseconds).
  }
  pixels.show(); // This sends the updated pixel color to the hardware.

}

void screenOff() {
  esp_sleep_enable_timer_wakeup(1000000); // sleep time is in uSec
  esp_deep_sleep_start();
}

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















void printIPs() {
  dbg(startupDBG, "AP SSID: ");
  dbg(startupDBG, apSSID);
  dbg(startupDBG, "; IP address(es): local=");
  dbg(startupDBG, WiFi.localIP());
  dbg(startupDBG, "; AP=");
  dln(startupDBG, WiFi.softAPIP());
}

// web server utils /////////////////////////////////////////////////////////
void getHtml( // turn array of strings & set of replacements into a String
  String& html, const char *boiler[], int boilerLen,
  replacement_t repls[], int replsLen
) {
  for(int i = 0, j = 0; i < boilerLen; i++) {
    if(j < replsLen && repls[j].position == i)
      html.concat(repls[j++].replacement);
    else
      html.concat(boiler[i]);
  }
}
const char *templatePage[] = {    // we'll use Ex07 templating to build pages
  "<html><head><title>",                                                //  0
  "default title",                                                      //  1
  "</title>\n",                                                         //  2
  "<meta charset='utf-8'>",                                             //  3
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
  "<style>body{background:#FFF; color: #000; font-family: sans-serif;", //  4
  "font-size: 150%;}</style>\n",                                        //  5
  "</head><body>\n",                                                    //  6
  "<h2>Welcome to Thing!</h2>\n",                                       //  7
  "<!-- page payload goes here... -->\n",                               //  8
  "<!-- ...and/or here... -->\n",                                       //  9
  "\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p>\n",                  // 10
  "</body></html>\n\n",                                                 // 11
};
void initWebServer() { // changed naming conventions to avoid clash with Ex06
  // register callbacks to handle different paths
  webServer->on("/", hndlRoot);              // slash
  webServer->onNotFound(hndlNotFound);       // 404s...
  webServer->on("/generate_204", hndlRoot);  // Android captive portal support
  webServer->on("/L0", hndlRoot);            // erm, is this...
  webServer->on("/L2", hndlRoot);            // ...IoS captive portal...
  webServer->on("/ALL", hndlRoot);           // ...stuff?
  webServer->on("/wifi", hndlWifi);          // page for choosing an AP
  webServer->on("/wifichz", hndlWifichz);    // landing page for AP form submit
  webServer->on("/status", hndlStatus);      // status check, e.g. IP address

  webServer->begin();
  dln(startupDBG, "HTTP server started");
}

// webserver handler callbacks
void hndlNotFound(AsyncWebServerRequest *request) {
  dbg(netDBG, "URI Not Found: ");
  dln(netDBG, request->url());
  request->send(200, "text/plain", "URI Not Found");
}
void hndlRoot(AsyncWebServerRequest *request) {
  dln(netDBG, "serving page notionally at /");
  replacement_t repls[] = { // the elements to replace in the boilerplate
    {  1, apSSID.c_str() },
    {  8, "" },
    {  9, "<p>Choose a <a href=\"wifi\">wifi access point</a>.</p>" },
    { 10, "<p>Check <a href='/status'>wifi status</a>.</p>" },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07
  request->send(200, "text/html", htmlPage);
}
void hndlWifi(AsyncWebServerRequest *request) {
  dln(netDBG, "serving page at /wifi");

  String form = ""; // a form for choosing an access point and entering key
  apListForm(form);
  replacement_t repls[] = { // the elements to replace in the boilerplate
    { 1, apSSID.c_str() },
    { 7, "<h2>Network configuration</h2>\n" },
    { 8, "" },
    { 9, form.c_str() },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07

  request->send(200, "text/html", htmlPage);
}
void hndlWifichz(AsyncWebServerRequest *request) {
  dln(netDBG, "serving page at /wifichz");

  String title = "<h2>Joining wifi network...</h2>";
  String message = "<p>Check <a href='/status'>wifi status</a>.</p>";

  String ssid = "";
  String key = "";
  for(uint8_t i = 0; i < request->args(); i++ ) {
    if(request->argName(i) == "ssid")
      ssid = request->arg(i);
    else if(request->argName(i) == "key")
      key = request->arg(i);
  }

  if(ssid == "") {
    message = "<h2>Ooops, no SSID...?</h2>\n<p>Looks like a bug :-(</p>";
  } else {
    char ssidchars[ssid.length()+1];
    char keychars[key.length()+1];
    ssid.toCharArray(ssidchars, ssid.length()+1);
    key.toCharArray(keychars, key.length()+1);
    WiFi.begin(ssidchars, keychars);
  }

  replacement_t repls[] = { // the elements to replace in the template
    { 1, apSSID.c_str() },
    { 7, title.c_str() },
    { 8, "" },
    { 9, message.c_str() },
  };
  String htmlPage = "";     // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls);

  request->send(200, "text/html", htmlPage);
}
void hndlStatus(AsyncWebServerRequest *request) { // UI to check connectivity
  dln(netDBG, "serving page at /status");

  String s = "";
  s += "<ul>\n";
  s += "\n<li>SSID: ";
  s += WiFi.SSID();
  s += "</li>";
  s += "\n<li>Status: ";
  switch(WiFi.status()) {
    case WL_IDLE_STATUS:
      s += "WL_IDLE_STATUS</li>"; break;
    case WL_NO_SSID_AVAIL:
      s += "WL_NO_SSID_AVAIL</li>"; break;
    case WL_SCAN_COMPLETED:
      s += "WL_SCAN_COMPLETED</li>"; break;
    case WL_CONNECTED:
      s += "WL_CONNECTED</li>"; break;
    case WL_CONNECT_FAILED:
      s += "WL_CONNECT_FAILED</li>"; break;
    case WL_CONNECTION_LOST:
      s += "WL_CONNECTION_LOST</li>"; break;
    case WL_DISCONNECTED:
      s += "WL_DISCONNECTED</li>"; break;
    default:
      s += "unknown</li>";
  }

  s += "\n<li>Local IP: ";     s += ip2str(WiFi.localIP());
  s += "</li>\n";
  s += "\n<li>Soft AP IP: ";   s += ip2str(WiFi.softAPIP());
  s += "</li>\n";
  s += "\n<li>AP SSID name: "; s += apSSID;
  s += "</li>\n";

  s += "</ul></p>";

  replacement_t repls[] = { // the elements to replace in the boilerplate
    { 1, apSSID.c_str() },
    { 7, "<h2>Status</h2>\n" },
    { 8, "" },
    { 9, s.c_str() },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07

  request->send(200, "text/html", htmlPage);
}
void apListForm(String& f) { // utility to create a form for choosing AP
  const char *checked = " checked";
  int n = WiFi.scanNetworks();
  dbg(netDBG, "scan done: ");

  if(n == 0) {
    dln(netDBG, "no networks found");
    f += "No wifi access points found :-( ";
    f += "<a href='/'>Back</a><br/><a href='/wifi'>Try again?</a></p>\n";
  } else {
    dbg(netDBG, n); dln(netDBG, " networks found");
    f += "<p>Wifi access points available:</p>\n"
         "<p><form method='POST' action='wifichz'> ";
    for(int i = 0; i < n; ++i) {
      f.concat("<input type='radio' name='ssid' value='");
      f.concat(WiFi.SSID(i));
      f.concat("'");
      f.concat(checked);
      f.concat(">");
      f.concat(WiFi.SSID(i));
      f.concat(" (");
      f.concat(WiFi.RSSI(i));
      f.concat(" dBm)");
      f.concat("<br/>\n");
      checked = "";
    }
    f += "<br/>Pass key: <input type='textarea' name='key'><br/><br/> ";
    f += "<input type='submit' value='Submit'></form></p>";
  }
}
String ip2str(IPAddress address) { // utility for printing IP addresses
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}
