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
#include "joinme.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

//time
#include "time.h"

//power consumption
#include "driver/rtc_io.h"

//TIME FUNCTIONS
#include <soc/rtc.h>
extern "C" {
  #include <esp_clk.h>
}





//CONSTANTS AND INSTANTS////////////////////////////////////////////////
int loopIter = 0;        // loop slices
const byte BM_I2Cadd   = 0x6b; // the chip lives here on I²C
const byte BM_Status   = 0x08; // system status register
bool snooze = false;
int dawnTime = 240000000;
int time_check_hour =0;
double seconds;
unsigned long timer = micros();
UIController *uiCont;
#define uS_TO_S_FACTOR 1000000  //Convert seconds to microseconds
// globals for a wifi access point and webserver ////////////////////////////
String apSSID = String("Paz_Dave_Unphone-"); // SSID of the AP
String apPassword = _DEFAULT_AP_KEY;     // passkey for the AP
const char* ntpServer = "0.pool.ntp.org"; //Time start Settings:
//offsets
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;
uint64_t timeDiff, timeNow; //RTC clock variables
bool alarm_on = false;




//NEOPIXEL LEDS SHIT////////////////////////
//imports
#include	<string.h>
#include	"esp_event_loop.h"	//	for usleep
#include	"neopixel.h"
#include "neopixel.c"
#include	<esp_log.h>

//constants and instants
#define	NEOPIXEL_PORT	15
#define	NR_LED 32
#define	NEOPIXEL_RMT_CHANNEL		RMT_CHANNEL_2
pixel_settings_t px;
uint32_t	 pixels[NR_LED];
int rc;
int pixelBrightness = 0;
/////////////////////////////////////////

//Connect to Wifi stuff/////////////////////
#include <ESPAsyncWebServer.h>

//instants
AsyncWebServer* webServer;               // async web server
typedef struct { int position; const char *replacement; } replacement_t;
void getHtml(String& html, const char *[], int, replacement_t [], int);

//methods
void getHtml( String& html, const char *boiler[], int boilerLen,replacement_t repls[], int replsLen);
void initWebServer();
void hndlRoot(AsyncWebServerRequest *);
void hndlNotFound(AsyncWebServerRequest *);
void hndlWifichz(AsyncWebServerRequest *);
void hndlStatus(AsyncWebServerRequest *);
void hndlWifi(AsyncWebServerRequest *);
void apListForm(String&);
void printIPs();
String ip2str(IPAddress address);
void getWiFiNetworks();
#define ALEN(a) ((int) (sizeof(a) / sizeof(a[0]))) // only in definition scope!
#define GET_HTML(strout, boiler, repls) \
  getHtml(strout, boiler, ALEN(boiler), repls, ALEN(repls));



//TODO SAVE DATA OVER BOOT IN RCU MEMORY
RTC_DATA_ATTR byte bootCountt = 0;
RTC_DATA_ATTR time_t time_now;
RTC_DATA_ATTR struct tm * timeinfo;
RTC_DATA_ATTR struct tm alarmTime;
RTC_DATA_ATTR int old_milis;
RTC_DATA_ATTR uint64_t sleepTime;
RTC_DATA_ATTR uint64_t offset_time;
RTC_DATA_ATTR bool alarmNotSet = true;
RTC_DATA_ATTR int am_sec = 0;
RTC_DATA_ATTR int am_min = 36;
RTC_DATA_ATTR int am_hour = 19;
RTC_DATA_ATTR int am_day = 17;
RTC_DATA_ATTR int am_mon = 11;
RTC_DATA_ATTR int am_year = 119;



// define methods//////////////////////////////////////////////////////
char *getMAC(char *);    // read the address into buffer
char MAC_ADDRESS[13];    // MAC addresses are 12 chars, plus the NULL
void lcdMessage(char *); // message on screen
void setupPixels();
void pixelsOff();
void fadePixels(long int fade_time);
bool powerOn();
void powerMode();
void setAlarmTime();
/////////////////////////////////////////////////////////////////////////////////

void pixelsOff() {
  //set all values to 0
  for	( int i = 0 ; i < NR_LED ; i ++ )	{
    np_set_pixel_rgbw(&px, i , 0, 0, 0, 0);
  }
  delay(100);
  np_set_pixel_rgbw(&px, 0 , 0, 0, 0, 0);
  delay(100);
  np_show(&px, NEOPIXEL_RMT_CHANNEL);
}

void setupPixels() {

  rc = neopixel_init(NEOPIXEL_PORT, NEOPIXEL_RMT_CHANNEL);
  ESP_LOGE("main", "neopixel_init rc = %d", rc);
	usleep(1000*1000);

  pixelsOff();

  px.pixels = (uint8_t *)pixels;
	px.pixel_count = NR_LED;
  strcpy(px.color_order, "GRB");

	memset(&px.timings, 0, sizeof(px.timings));
	px.timings.mark.level0 = 1;
	px.timings.space.level0 = 1;
	px.timings.mark.duration0 = 12;

	px.nbits = 24;
	px.timings.mark.duration1 = 14;
	px.timings.space.duration0 = 7;
	px.timings.space.duration1 = 16;
	px.timings.reset.duration0 = 600;
	px.timings.reset.duration1 = 600;

  //brightness of 255
	px.brightness = 0xFF;

}

void fadePixels() {
  pixelBrightness++;

  for	( int j = 0 ; j < NR_LED ; j ++ )	{
    np_set_pixel_rgbw(&px, j , pixelBrightness, 0, 0, pixelBrightness);
  }
  np_show(&px, NEOPIXEL_RMT_CHANNEL);

}

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
  alarmTime.tm_sec = am_sec;
  alarmTime.tm_hour = am_hour;
  alarmTime.tm_min = am_min;
  alarmTime.tm_mon  = am_mon;
  alarmTime.tm_mday = am_day;
  alarmTime.tm_year = am_year;
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

void stopAlarm() {
  pixelsOff();
  am_year = 130;
  setAlarmTime();
}

void snoozeAlarm() {
  am_sec += 300 ;
  setAlarmTime();
  pixelsOff();
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

  setupPixels();

  //first boot so connect to wifi to get time
  if (bootCountt == 0 ) {
    //Connect to save wifi, if none start AP
    // Serial.printf("doing wifi manager\n");
    // joinmeManageWiFi(apSSID.c_str(), apPassword.c_str()); // get net connection
    // Serial.printf("wifi manager done\n\n");
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) {
    //   delay(500);
    //   Serial.print(".");
    //   if (++wifiCounter > 30) ESP.restart(); // restart after 15 s
    // }
    // Serial.println(" CONNECTED");
    WiFi.disconnect(true);

    WiFi.begin();
    while(WiFi.status() != WL_CONNECTED) {
      Serial.printf("Starting AP");
      WiFi.mode(WIFI_STA);
      //create access point and set up its credentials
      WiFi.softAP(apSSID.c_str(), apPassword.c_str());
      vTaskDelay(400);

      //Try and connect to last known network
      // init the web server, say hello
      webServer = new AsyncWebServer(80);
      initWebServer();


      if (micros() == 300000000) {
        ESP.restart();
      }
      delay(1000);
      //let wifi AP settle
      vTaskDelay(400);
    }

    delay(1000);
    //GET current time and print to serial line
    int year = 0;
    while( year < 118) {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      delay(1000);
      time(&time_now);
      struct tm yearCheck;
      yearCheck = *localtime(&time_now);
      year = yearCheck.tm_year;
      Serial.print("year");

      Serial.print(year);
      // WiFi_Disconnect();
      // delay(200); // do not remove, need a delay for disconnect to cha nge status()

    }
    bootCountt++;

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
  uiCont = new UIController(ui_dawn);
  if(!uiCont->begin()) {
    E("WARNING: ui.begin failed!\n")
  }
}

void loop() {
  D("\nentering main loop\n")
  unsigned long int timer_2 = micros();
  while(1) {
    uiCont->run();

    if (alarmNotSet) {
      setAlarmTime();
      alarmNotSet = false;
    }

    //Start alarm
    if (time2Alarm() <= dawnTime/1000000 ) {
      if (micros() - timer >= (dawnTime/255)) {
        Serial.print("here");
        fadePixels();
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
      digitalWrite(GPIO_NUM_36, LOW);

      pixelsOff();
      delay(1000); //let pixels settle

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
      digitalWrite(GPIO_NUM_36, LOW);
      pixelsOff();
      delay(1000);

      //start deep sleep
      sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;
      esp_deep_sleep_start();
    }

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
    delay(1000);
    bootCountt = 0;
    sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;

    unPhone::setShipping(true);
  } else {
    //turn pixels off
    pixelsOff();
    //let pixels settle
    delay(1000);
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








// web server utils //////////////////////////////////////////////////////////
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
void hndlStatus(AsyncWebServerRequest *request) { // UI for checking connectivity etc.
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
