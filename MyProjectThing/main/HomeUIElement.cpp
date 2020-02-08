// HomeUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>
#include "main.cpp"
#include <Adafruit_GFX.h>    // Core graphics library

// fonts ////////////////////////////////////////////////////////////////////
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>

#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>

#include<bits/stdc++.h>
using namespace std;

extern int firmwareVersion;
extern String apSSID;
extern tm * timeinfo;
extern char arrayToStore[50];
extern float temperature;
extern float speed;
extern float humidity;
int f_sec;
int sc;
int mn;
int hr;
int fir;
int snd;
String tmp;
char time_str[50];
char date_str[50];
char date_str_day[50];
char date_str_month[50];
char date_str_year[50];
bool display = true;
String day_;
String greeting();
long int dot_timer = 0;
std::pair<int, int> timeUntilDawn(double secs);
bool cleared = false;

int normaliseF(float x);
int normaliseS(String x);

// handle touch on this page ////////////////////////////////////////////////
bool HomeUIElement::handleTouch(long x, long y) {
  // touching anywhere will display menu
  return true;
}

// draw all home display elements ///////////////////////////////////////////
void HomeUIElement::draw() {
  drawWeather();
  drawGreeting();
  drawTime();
  drawDate();
  if (alarm_exist) {                  // draw time to alarm if alarm exists
    if (time2Alarm() >= 0.00)
      drawAlarmTime();
      cleared = false;
  } else {                            // otherwise draw 'No Alarm Set'
    drawNoAlarm();
  }
  if (millis() - dot_timer >= 1000) { // flash digital clock dots every second
    flashDots();
    dot_timer = millis();
  }
}

// clear methods for updating home display elements /////////////////////////
// displays black rectangle over specific areas before being updated
void HomeUIElement::clearDate() {
  m_tft->fillRect(    5, 210,  475,  100, BLACK);
}

void HomeUIElement::clearSec() {
  m_tft->fillRect(  360,   90,  100,  75, BLACK);
}

void HomeUIElement::clearMin() {
  m_tft->fillRect(  195,   90,  100,  75, BLACK);
}

void HomeUIElement::clearHour() {
  m_tft->fillRect(  30,   90,  100,  75, BLACK);
}

void HomeUIElement::clearAlarm(int x) {
  m_tft->fillRect(  x,   170,  380,  50, BLACK);
}

void HomeUIElement::flashDots() {
  m_tft->fillRect(  315,   90,  15,  75, BLACK);
  m_tft->fillRect(  150,   90,  15,  75, BLACK);
}

void HomeUIElement::clearGreeting() {
  m_tft->fillRect(  0, 0,  480,  75, BLACK);
}

void HomeUIElement::drawGreeting() {
  if (tmp != greeting()) // clear greeting if changed
    clearGreeting();
  m_tft->setFont(&FreeMonoBoldOblique9pt7b);
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(2);
  m_tft->setCursor(5,25);
  m_tft->println(greeting());
  tmp = greeting();
}

int HomeUIElement::normaliseF(float x) {
  m_tft->setCursor(0,-100); m_tft->print(x);
  return m_tft->getCursorX();
}

int HomeUIElement::normaliseS(String x) {
  m_tft->setCursor(0,-100); m_tft->print(x);
  return m_tft->getCursorX();
}

void HomeUIElement::drawWeather() {
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextSize(2);

  if (temperature <= 0) { m_tft->setTextColor(WHITE);}
  else if (temperature > 0 && temperature <= 10)  { m_tft->setTextColor(CYAN); }
  else if (temperature > 10 && temperature <= 20) { m_tft->setTextColor(GREEN); }
  else if (temperature > 20 && temperature <= 30) { m_tft->setTextColor(ORANGE); }
  else { m_tft->setTextColor(RED); }

  m_tft->setCursor(490-normaliseF(temperature)-normaliseS("*C"),35);
  m_tft->printf("%.1f",temperature);m_tft->print("*C");

  m_tft->setTextColor(WHITE);
  m_tft->setCursor(490-normaliseF(speed)-normaliseS("m/s"),255);
  m_tft->printf("%.1f",speed);m_tft->print("m/s");

  m_tft->setCursor(516-normaliseF(humidity)-normaliseS("%"),295);
  m_tft->printf("%.f",humidity);m_tft->print("%");

  String description = EEPROM.get(0, arrayToStore);
  m_tft->setCursor(470-normaliseS(description),72);m_tft->print(description);
}

// draw time HH:MM:SS
void HomeUIElement::drawTime() {
  if (sc != timeinfo->tm_sec) // clear sec if changed
    clearSec();
  if (mn != timeinfo->tm_min) // clear mins if changed
    clearMin();
  if (hr != timeinfo->tm_hour) // clear hour if changed
    clearHour();
  m_tft->setFont(&FreeMonoBold9pt7b);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(5);
  m_tft->setCursor(25, 150);
  strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo); // format time to hh:mm:ss
  sc = timeinfo->tm_sec;
  mn = timeinfo->tm_min;
  hr = timeinfo->tm_hour;
  m_tft->print(time_str);
}

// draw time until alarm
void HomeUIElement::drawAlarmTime() {
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextColor(YELLOW);
  m_tft->setTextSize(2);
  m_tft->setCursor(150, 200);
  m_tft->print("Alarm in: ");
  if (time2Alarm() < 60) { // display time to alarm in seconds if <1m to alarm
    if (f_sec != time2Alarm())
      clearAlarm(300);
    m_tft->print((int)time2Alarm());
    m_tft->print("s");
    f_sec = time2Alarm();
  }
  else {                  // otherwise display time to alarm in h:m
    pair<int, int> p = timeUntilDawn(time2Alarm());
    if (fir != p.first || snd != p.second)
    clearAlarm(300);      // clear time to alarm if changed
    if (p.second == 59 && p.first == 0) {
      m_tft->print(1);m_tft->print("h "); // so time2 alarm goes 1h 0 min rather than
      m_tft->print(0);m_tft->print("m"); // 60 mins...
    } else if (p.second == 59 ){
      m_tft->print(p.first + 1);m_tft->print("h "); // so time2 alarm goes 1h 0 min rather than
      m_tft->print(0);m_tft->print("m"); // 60 mins...
    } else {
      m_tft->print(p.first);m_tft->print("h ");
      m_tft->print(p.second+1);m_tft->print("m");
    }
    fir = p.first;
    snd = p.second;
  }
}

// draw when no active alarm
void HomeUIElement::drawNoAlarm() {
  if (!cleared) {
    clearAlarm(100);
    cleared = true;
  }
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextColor(YELLOW);
  m_tft->setTextSize(2);
  m_tft->setCursor(150, 200);
  m_tft->print("No Alarm set ");
}

// return time until dawn as h:m pair given seconds
std::pair<int, int> timeUntilDawn(double secs) {
    int hours = floor(secs/3600);
    int mins = floor((secs - (hours*3600))/60);
    return std::make_pair(hours, mins);
}

// draw day & date
void HomeUIElement::drawDate() {
  strftime(date_str_day, sizeof(date_str_day), "%A", timeinfo);
  if (day_ != date_str_day) // clear date if changed
    clearDate();
  strftime(date_str_month, sizeof(date_str_month), "%B", timeinfo);
  strftime(date_str_year, sizeof(date_str_year), "%Y", timeinfo);
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextColor(MAGENTA);
  m_tft->setTextSize(2);
  m_tft->setCursor(5, 250);
  m_tft->print(date_str_day);m_tft->print(",");    // draw day of the week
  m_tft->setCursor(5,300);
  m_tft->print(timeinfo->tm_mday);m_tft->print(" ");
  m_tft->print(date_str_month);m_tft->print(" ");  // followed by date
  m_tft->print(date_str_year);
  day_ = date_str_day;
}

// draw greeting depending on
String greeting() {
  String greet;
  int hour = timeinfo->tm_hour;
  if (hour >= 6 && hour < 12) {         // morning 6am to 11.59am
    greet = "Good morning";
  }
  else if (hour >= 12 && hour < 17) {   // noon 12pm to 4.59pm
    greet = "Good afternoon";
  }
  else if (hour >= 17 && hour < 20) {   // evening 5pm to 7:59pm
    greet = "Good evening";
  }
  else {                                // night 8pm to 5:59am
    greet = "Good night";
  }
  return greet;
}

//////////////////////////////////////////////////////////////////////////
void HomeUIElement::runEachTurn(){
  draw();
}
