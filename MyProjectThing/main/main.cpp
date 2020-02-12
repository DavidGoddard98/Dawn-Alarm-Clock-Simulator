// main.cpp


//***********************************************************************
// a library or two... ///////////////////////////////////////////////////////
#include "private.h" // stuff not for checking in
#include "unphone.h"
#include "IOExpander.h"
#include "UIController.h"
#include "AllUIElement.h"
#include <iostream>
#include <iomanip>

//TIME FUNCTIONS
#include <soc/rtc.h>
extern "C" {
  #include <esp_clk.h>
}

using namespace std;

//GENERAL CONSTANTS AND INSTANTS////////////////////////////////////////////////
int firmwareVersion = 2;    // keep up-to-date! (used to check for updates)
int addr = 0;
int loopIter = 0;        // loop slices
const byte BM_I2Cadd   = 0x6b; // the chip lives here on I²C
const byte BM_Status   = 0x08; // system status register
bool snooze = false;
int time_check_hour =0; //hours since last wifi grab
double seconds; //used for time2alarm
bool alarm_on = false;
int year; // individual year variable - used to ensure correct time is grabbed from ntp server
char MAC_ADDRESS[13];    // MAC addresses are 12 chars, plus the NULL
double dawn_seconds; //used for time2dawn
unsigned long timer = micros(); // timer to fade in pixels over set time
const char* ntpServer = "0.pool.ntp.org"; //Time start Settings:
const long  gmtOffset_sec = 0; //offsets
const int   daylightOffset_sec = 0;
UIController *uiCont;
#define uS_TO_S_FACTOR 1000000  //Convert seconds to microseconds
// globals for a wifi access point and webserver ////////////////////////////
String apSSID = String("Paz_Dave_Alarm-"); // SSID of the AP
String apPassword = _DEFAULT_AP_KEY;     // passkey for the AP
uint64_t timeDiff, timeNow; //RTC clock variables

//NEOPIXEL ESPIDF variant/////////////////////////////////////////////////////
//imports
#include	<string.h>
#include	"esp_event_loop.h"	//	for usleep
#include	"neopixel.h"
#include  "neopixel.c"
#include	<esp_log.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "EEPROM.h"

//constants and instants
#define	NEOPIXEL_PORT	15 //Pin A7 || 15
#define	NR_LED 32
#define	NEOPIXEL_RMT_CHANNEL		RMT_CHANNEL_2
pixel_settings_t px;
uint32_t	 pixels[NR_LED];
int rc;
float pixBr = 0;
char arrayToStore[50];
////////////////////////////////////////////////////////////////////////////////
// IR SENSOR stuff//////////////////////////////////////////////////////////////
#define PIR_DOUT 27   // PIR digital output on D2
#define LED_PIN  13  // LED to illuminate on motion
#define EEPROM_SIZE 64 // allocate 64 bytes of flash memory for weather description

////////////////////////////////////////////////////////////////////////////////

//SAVE DATA OVER BOOT IN RCU MEMORY/////////////////////////////////////////////
RTC_DATA_ATTR byte bootCountt = 0;
RTC_DATA_ATTR byte hourFetched;
RTC_DATA_ATTR time_t time_now;
RTC_DATA_ATTR struct tm * timeinfo;
RTC_DATA_ATTR struct tm *  alarmTime;
RTC_DATA_ATTR int old_milis;
RTC_DATA_ATTR uint64_t sleepTime;
RTC_DATA_ATTR uint64_t offset_time;
RTC_DATA_ATTR bool alarmNotSet = true;
RTC_DATA_ATTR bool alarm_exist = false;
RTC_DATA_ATTR int fade_time = 240000000;
RTC_DATA_ATTR time_t alarm_time;
RTC_DATA_ATTR time_t dawn_time;
RTC_DATA_ATTR tuple <int,int,int> rgb;
RTC_DATA_ATTR float temperature;
RTC_DATA_ATTR float humidity;
RTC_DATA_ATTR float speed;
////////////////////////////////////////////////////////////////////////////////

//DEFINE METHODS////////////////////////////////////////////////////////////////
//ordered
void fetchTime();
void pixelsOff();
void setupPixels();
void fadePixels();
void weather();
void setDawnColour(uint16_t col);
void printLocalTime();
void setTime();
void updateTime();
double time2Alarm();
double time2Dawn();
void stopAlarm();
void snoozeAlarm();
void vibrate();
bool powerOn();
void powerMode();
void inActiveSleep();
void forcedSleep();
char *getMAC(char *);    // read the address into buffer

//==============================================================================
/////////////////////////////SETUP METHOD///////////////////////////////////////
//==============================================================================
void setup() {
  UNPHONE_DBG = true;
  unPhone::begin();
  getMAC(MAC_ADDRESS);          // store the MAC address
  apSSID.concat(MAC_ADDRESS);   // add the MAC to the AP SSID

  //Configure IR pin to read values
  pinMode(PIR_DOUT, INPUT);
  // Configure the motion indicator LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Turn the LED off

  //Show Wifi UI
  uiCont = new UIController(ui_boot);
  if(!uiCont->begin()) {
    E("WARNING: ui.begin failed!\n")
  }

  //....
  unPhone::printWakeupReason(); // what woke us up?
  unPhone::checkPowerSwitch();  // if power switch is off, shutdown

  EEPROM.begin(EEPROM_SIZE); // initialize EEPROM with predefined size

  //Initialise and setup pixels. Also clears them
  setupPixels();

  //add time esp has been asleep for to current time (using RTC offset from fetchTime method)
  updateTime();

  //check if its been two hours since last timefetch (from internet)
  //if it has restart and fetch again (keep accurate)
  if(timeinfo->tm_hour - hourFetched >=2) {
    bootCountt = 0;
  }

  //first boot so connect to wifi to get time
  if (bootCountt == 0 ) {
    fetchTime(); //connects to wifi and gets time
  }

  // //add time esp has been asleep for to current time (using RTC offset from fetchTime method)
  // updateTime();
  //
  // //check if its been two hours since last timefetch (from internet)
  // //if it has restart and fetch again (keep accurate)
  // if(timeinfo->tm_hour - hourFetched >=2) {
  //   bootCountt = 0;
  // }



  //print time to console
  Serial.printf ("%s\n", asctime(timeinfo));

  //print dawn colour to console
  Serial.print(get<0>(rgb)); Serial.print(", ");
  Serial.print(get<1>(rgb)); Serial.print(", ");
  Serial.println(get<2>(rgb));

}

//==============================================================================
/////////////////////////////LOOP METHOD////////////////////////////////////////
//==============================================================================
void loop() {
  //show the basic alarm clock UI
  uiCont->showUI(ui_home);

  D("\nentering main loop\n")

  long int touchTimer = millis(); //counter to determine how long since user touched screen
  while(1) {

    setTime(); //update time each iteration

    //handles touch
    //uiCont->run() was used to handle touch but this draws the screen
    //and there is no need to draw screen every iteration (impacts responsivness)
    //so just handle touch
    if (uiCont->gotTouch()) {
      uiCont->handleTouch();
      touchTimer = millis();
    }
    //then, after 80 iterations, draw screen.
    if (loopIter % 80 == 0) {
      uiCont->run();
    }

    //If power switch not on turn unphone off (see powerMode function)
    if (!powerOn()) powerMode();

    //Functionality for actual ALARM///////////////////////////////////////////
    if (alarm_exist) {
      //start dawn simulator
      if (time2Dawn() == 0.00 ) {
        if (micros() - timer >= (fade_time/255)) { //proportion fade time in
          fadePixels();
          timer = micros();
        }
        //start alarm (vibe motor)
        if (time2Alarm() == 0.00 ) {
          alarm_on = true;
          vibrate();
          uiCont->run();
          //Any motion measured from IR snoozes for 5 mins
          if (digitalRead(PIR_DOUT) == 1 ){
            touchTimer = millis(); //keep screen on for another 30 secs
            snoozeAlarm();
          }
          //button3 snoozes for 5 mins
          if (unPhone::button3()) {
            touchTimer = millis(); //keep screen on for another 30 secs
            snoozeAlarm();
          }
          //button 1 cancels alarm
          if (unPhone::button1()) {
            touchTimer = millis(); //keep screen on for another 30 secs
            stopAlarm();
          }
        }
      }
    }

    //Auto deep sleeps esp if;
    //Time to dawn > 5 mins
    //OR screen has not be pressed in 30 seconds
    //Wakes up if;
    //Button 3 pressed
    //Time to dawn is <= 5 mins (just before LEDS begin fading)
    if ((millis() - touchTimer) >= 30000) {
      inActiveSleep();
    }

    //If button 2 pressed deep_sleep - will wake up if;
    //Button 3 pressed
    //Time to dawn is <= 5 mins (just before LEDS begin fading)
    if(unPhone::button2()) {
      forcedSleep();
     }

    // // allow the protocol CPU IDLE task to run periodically
    if(loopIter % 2500 == 0) {
      printLocalTime();

      if(loopIter % 25000 == 0) {
        D("completed loop %d, yielding 1000th time since last\n", loopIter);
        if (alarm_exist) {
          Serial.println("Time to alarm:" + String(time2Alarm()));
          Serial.println("Time to dawn:" + String(time2Dawn()));
        }
      }
    }
    loopIter++;
  }
}

//==============================================================================
//////////////////////FUNCTION IMPLEMENTATION///////////////////////////////////
//==============================================================================

//==============================================================================
//////////////////////TIME MANAGEMENT///////////////////////////////////////////
//==============================================================================
void fetchTime() {
  //Connect to saved wifi, if none start AP
  uiCont->showUI(ui_config); //show config UI page
  Serial.printf("doing wifi manager\n");
  joinmeManageWiFi(apSSID.c_str(), apPassword.c_str());
  Serial.printf("wifi manager done\n\n");
  delay(1000); //let wifi settle

  Serial.printf("Checking for firmware updates");
  joinmeOTAUpdate(firmwareVersion, _GITLAB_PROJ_ID,_GITLAB_TOKEN,"MyProjectThing%2Ffirmware%2F");


  //let wifi AP settle
  if (WiFi.status() != WL_CONNECTED) {
    while(WiFi.status() != WL_CONNECTED) {
      if (!powerOn()) powerMode(); //turn off if switch off
      if (micros() >= 300000000) { //4 mins to connect to AP
        ESP.restart();
      }
    }
  }

  //GET current time and print to serial line
  while( year < 118) { // if time returned is a year <2018 then incorrect so fetch again
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //gets current time
    delay(1000);
    time(&time_now);
    struct tm yearCheck; //store time in tm struct
    yearCheck = *localtime(&time_now);
    //stores year - (if < 2018 then time was not retrievied correctly)
    //default is 1970...
    year = yearCheck.tm_year;
    hourFetched = yearCheck.tm_hour; //record the hour last time fetched
  }

  weather(); // make API request for weather data

  //disconnect WiFi/stop AP as it's no longer needed (SAVE BATTERY)
  WiFi.disconnect(true);
  WiFi.softAPdisconnect (true);
  WiFi.mode(WIFI_OFF);

  //increment boot count so wifi isnt connected to for another 2 hours
  bootCountt = 1;

  //initialise offset  = RTC clock time right now
  //is used back in setup method to determine how long clock been sleep 4...
  offset_time = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
}

//==============================================================================
//////////////////////PIXEL FUNCTIONALITY///////////////////////////////////////
//==============================================================================
void pixelsOff() {
  //set all values to 0
  for	( int i = 0 ; i < NR_LED ; i ++ )	{
    np_set_pixel_rgbw(&px, i , 0, 0, 0, 0);
  }
  delay(1000);
  np_show(&px, NEOPIXEL_RMT_CHANNEL);

  //let pixels settle
  //re set 1st pixel (often glitches and stays on)
  np_set_pixel_rgbw(&px, 0 , 0, 0, 0, 0);
  delay(100);
  np_show(&px, NEOPIXEL_RMT_CHANNEL);
}

//Setup pixel - library obtained from
//https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo).
void setupPixels() {
  rc = neopixel_init(NEOPIXEL_PORT, NEOPIXEL_RMT_CHANNEL);
  ESP_LOGE("main", "neopixel_init rc = %d", rc);
  usleep(1000*1000);

  pixelsOff();
  delay(1000);

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
  pixBr += 0.003921569f;

  for	( int j = 0 ; j < NR_LED ; j ++ )	{
    np_set_pixel_rgbw(&px, j , get<0>(rgb)*pixBr, get<1>(rgb)*pixBr, get<2>(rgb)*pixBr, pixBr*255);
  }
  np_show(&px, NEOPIXEL_RMT_CHANNEL);
  Serial.println(pixBr);
}

void setDawnColour(uint16_t col) {
  if (col == HX8357_BLUE) {
    rgb = make_tuple(0,0,255);
  }
  else if (col == HX8357_YELLOW) {
    rgb = make_tuple(255,255,0);
  }
  else if (col == HX8357_RED) {
    rgb = make_tuple(255,0,0);
  }
  else if (col == HX8357_GREEN) {
    rgb = make_tuple(0,255,0);
  }
  else if (col == HX8357_CYAN) {
    rgb = make_tuple(0,255,255);
  }
  else if (col == HX8357_PURPLE) {
    rgb = make_tuple(128,0,128);
  }
  else if (col == HX8357_ORANGE) {
    rgb = make_tuple(255,165,0);
  }
  else if (col == HX8357_PINK) {
    rgb = make_tuple(255,20,147);
  }
}

//==============================================================================
//////////////////////LOCAL TEMPERATURE ////////////////////////////////////////
//==============================================================================

void weather() {
  HTTPClient http;
  String cityId = "3333193";
  String countryCode = "uk";
  String owKey = "94dc8e7b59b910118bbbb2277fc434f9";
  String URL = PSTR("http://api.openweathermap.org/data/2.5/weather?id=")
    + cityId + F("&APPID=") + owKey;
  String payload;

  http.setTimeout(50000);  // increase the timeout of a request
  http.setReuse(true);
  http.addHeader("User-Agent", "ESP32");

  if (!http.begin(URL)) Serial.println("get HTTP failed");
  else {
    int respCode = http.GET();

    if (respCode == HTTP_CODE_OK) {
      payload = http.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload); // read data

      if (root.success()) {
        String name = root["name"];
        JsonObject& main = root["main"];
        temperature = main["temp"];             // pull temperature value
        temperature = temperature - 273.15f;    // convert from kelvin
        humidity = main["humidity"];            // pull humidity value

        JsonObject& weather = root["weather"][0];
        String des = weather["description"];    // pull description

        JsonObject& wind = root["wind"];
        speed = wind["speed"];                  // pull wind speed value

        /* store description string as char array in flash memory **************
          ESP is unable to retain Strings or char arrays that are saved
          in RTC memory after wakeup, but does in flash memory...
        */

        des.toCharArray(arrayToStore, des.length()+1);
        EEPROM.put(0, arrayToStore);
        EEPROM.commit();
      }
      Serial.println(payload);
    }
    else { Serial.println(respCode); }
    }
}

//==============================================================================
//////////////////////ALARM & DAWN FUNCTIONALITY////////////////////////////////
//==============================================================================


void printLocalTime() { //....
  Serial.printf ("%s\n", asctime(timeinfo));
}

//Called every iteration of LOOP function - updates the time.
void setTime() {
  time(&time_now);
  timeinfo = localtime (&time_now);
}

//Checks how long ESP has been in deep sleep for and adds this to internal clock
//See documentation for indepth analysis...
void updateTime() {
  //timeNow = RTC clock now - offset ...(essentially RTC clock now)
  //offset = duration of setup() during initial boot
  timeNow = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time; //this offset is from fetchTime() method
  //timeDiff = timeESP has been asleep for
  //sleepTime is recorded just before ESP goes to sleep - see inActiveSleep()
  timeDiff = timeNow - sleepTime;

  //Must convert this into seconds to add to clock
  //#secs
  int seconds = floor((timeDiff / 1000000));

  seconds = seconds * (1- (seconds * 0.0000016));  //apply calculated offset (the longer it sleeps the larger the offset)


  //#milis
  int milis = floor(timeDiff % 1000000);

  if (milis+old_milis > 1000) {
    //if milis more than 1000 add second to time
    seconds += 1;
    //make up the difference to save the rest of the milis which didnt make the 1 sec
    old_milis = (old_milis + milis) -1000 ;
  } else {
    old_milis += milis;
  }

  //Finally....
  //add the time ESP has been asleep to clock
  time_now = time_t(time_now) + seconds;
  struct timeval newTime = {.tv_sec = time_now};
  struct timezone timeZon = {.tz_minuteswest = 0 };     /* minutes west of Greenwich */

  settimeofday(&newTime, &timeZon );

  time(&time_now);
  timeinfo = localtime (&time_now);
  Serial.printf ("%s\n", asctime(timeinfo));
}

//returns seconds 2 alarm
double time2Alarm() {
  seconds = difftime(alarm_time,time_now);
  if (seconds <= 0) {
    seconds = 0;
  }
  return seconds;
}

//returns seconds till dawn (when leds should begin fading)
double time2Dawn() {
  dawn_seconds = difftime(dawn_time, time_now);
  if (dawn_seconds <= 0) {
    dawn_seconds = 0;
  }

  return dawn_seconds;
}

//Cancels the alarm - used when button 3 is pressed during alarm
void stopAlarm() {
  alarm_exist = false;
  pixelsOff();
  delay(500);
}

//snoozes alarm for 5 minutes
void snoozeAlarm() {
  alarm_on = false;
  alarm_time += 300;
  dawn_time += 300;
  pixelsOff();
  delay(500);
}

void vibrate() {
  unPhone::vibe(true);  delay(150);
  unPhone::vibe(false); delay(150);
}

//==============================================================================
//////////////////////POWER MANAGEMENT//////////////////////////////////////////
//==============================================================================

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
  unPhone::tftp->fillScreen(HX8357_BLACK); // clear screen for wakeup
  bool usbConnected = bitRead(unPhone::getRegister(BM_I2Cadd, BM_Status), 2);
  if (!usbConnected) {
    //turn pixels off
    pixelsOff();
    delay(1000);
    bootCountt = 0; //so time is fetched next time device turned on...
    //If usb not connected then shipping can = true, disconnect battery...
    unPhone::setShipping(true);
  } else {
    //if usb connected cant disconnect battery as might want to charge device..
    //turn pixels off
    pixelsOff();
    delay(1000);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0); //wakeup if switch switched
    //Turn screen off
    IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);

    //store epoch time
    time(&time_now);
    //store RTC clock at sleep time - needed in updateTime()
    sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;
    // deep sleep, wait for wakeup on GPIO pin
    esp_deep_sleep_start();
  }
}

//Puts ESP to sleep if screen inactive and time2Dawn > 5 mins
void inActiveSleep() {
  if (alarm_exist && time2Alarm() < (fade_time/1000000) + 5) {
    return;
  } else if (alarm_exist) {
    //wake up when dawn simulator is about to start
    esp_sleep_enable_timer_wakeup((time2Dawn() - 20) * uS_TO_S_FACTOR);

    Serial.println("Setup ESP32 to sleep for:");
    Serial.println(time2Dawn()-20);

  }
  unPhone::tftp->fillScreen(HX8357_BLACK);
  //wake up with button one
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);

  //Turn off screen
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);

  //pixels off
  pixelsOff();
  delay(1000); //let them settle

  Serial.println("Device put to sleep due to inactivity");
  //store epoch time
  time(&time_now);
  //store RTC clock at sleep time - needed in updateTime()
  sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;

  //start deep sleep
  esp_deep_sleep_start();
}

void forcedSleep() {
  //wake up with button one
  unPhone::tftp->fillScreen(HX8357_BLACK);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);

  //or before dawn simulator starts
  if (alarm_exist){
    if(time2Dawn() > 0) {
      esp_sleep_enable_timer_wakeup(time2Dawn() * uS_TO_S_FACTOR);
      Serial.println("Setup ESP32 to sleep for:");
      Serial.println(time2Dawn());
    }
  }

  //Turn peripherals off to save power
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);

  pixelsOff();
  delay(1000); //let them settle

  digitalWrite(6, LOW);
  IOExpander::digitalWrite(6,LOW);

  Serial.println("User chose to sleep device with button 2");

  //store epoch time
  time(&time_now);
  //Keep track of time before sleep
  sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) - offset_time;
  //start deep sleep
  esp_deep_sleep_start();
}


// misc utilities //////////////////////////////////////////////////////////////
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
