// HomeUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>
#include "main.cpp"
#include <Adafruit_GFX.h>    // Core graphics library

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

int sc;
int mn;
int hr;
int fir;
int snd;
char time_str[50];
char date_str[50];
char date_str_day[50];
char date_str_month[50];
char date_str_year[50];
bool display = true;
String day_;
String greeting();
tm* getTime();
long int dot_timer = 0;
std::pair<int, int> timeUntilDawn(double secs);
bool cleared = false;

// handle touch on this page ////////////////////////////////////////////////
bool HomeUIElement::handleTouch(long x, long y) {

  //
  return false;
}

// writes various things including mac address and wifi ssid ///////////////
void HomeUIElement::draw() {
  drawGreeting();
  drawTime();
  drawDate();
  if (alarm_exist) {
    if (time2Alarm() >= 0.00)
      drawAlarmTime();
      cleared = false;
  } else {
    drawNoAlarm();
  }
  if (millis() - dot_timer >= 1000) {
    flashDots();
    dot_timer = millis();
  }
}

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

void HomeUIElement::clearAlarm() {
  m_tft->fillRect(  100,   170,  380,  50, BLACK);
}

void HomeUIElement::flashDots() {
  m_tft->fillRect(  315,   90,  15,  75, BLACK);
  m_tft->fillRect(  150,   90,  15,  75, BLACK);
}

void HomeUIElement::drawGreeting() {
  m_tft->setFont(&FreeMonoBoldOblique9pt7b);
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(2);
  m_tft->setCursor(30,30);
  m_tft->println(greeting());
}

void HomeUIElement::drawTime() {
  if (sc != getTime()->tm_sec)
    clearSec();
  if (mn != getTime()->tm_min)
    clearMin();
  if (hr != getTime()->tm_hour)
    clearHour();
  m_tft->setFont(&FreeMonoBold9pt7b);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(5);
  m_tft->setCursor(90, 150);
  strftime(time_str, sizeof(time_str), "%H:%M:%S", getTime());
  sc = getTime()->tm_sec;
  mn = getTime()->tm_min;
  hr = getTime()->tm_hour;
  m_tft->print(time_str);
}

void HomeUIElement::drawAlarmTime() {
  pair<int, int> p = timeUntilDawn(time2Alarm());
  if (fir != p.first || snd != p.second)
    clearAlarm();
  m_tft->setFont(&FreeMono9pt7b);
  m_tft->setTextColor(YELLOW);
  m_tft->setTextSize(2);
  m_tft->setCursor(120, 200);
  m_tft->print("Alarm in: ");
  m_tft->print(p.first);m_tft->print("h ");
  m_tft->print(p.second);m_tft->print("m");
  fir = p.first;
  snd = p.second;
}

void HomeUIElement::drawNoAlarm() {
  if (!cleared) {
    clearAlarm();
    cleared = true;
  }
  m_tft->setFont(&FreeMono9pt7b);
  m_tft->setTextColor(YELLOW);
  m_tft->setTextSize(2);
  m_tft->setCursor(120, 200);
  m_tft->print("No Alarm set ");

}

std::pair<int, int> timeUntilDawn(double secs)
{
    int hours = floor(secs/3600);
    int mins = floor((secs - (hours*3600))/60);
    return std::make_pair(hours, mins);
}

void HomeUIElement::drawDate() {
  strftime(date_str_day, sizeof(date_str_day), "%A", getTime());
  if (day_ != date_str_day)
    clearDate();
  strftime(date_str_month, sizeof(date_str_month), "%B", getTime());
  strftime(date_str_year, sizeof(date_str_year), "%Y", getTime());
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextColor(MAGENTA);
  m_tft->setTextSize(2);
  m_tft->setCursor(5, 250);
  m_tft->print(date_str_day);m_tft->print(",");
  m_tft->setCursor(5,300);
  m_tft->print(getTime()->tm_mday);m_tft->print(" ");
  m_tft->print(date_str_month);m_tft->print(" ");
  m_tft->print(date_str_year);
  day_ = date_str_day;
}

tm* getTime() {
  time_t rawtime;
  struct tm *info;
  time( &rawtime );
  info = localtime( &rawtime );
  return info;
}

String greeting() {
  String greet;
  int hour = getTime()->tm_hour;
  if (hour >= 6 && hour < 12) { // morning 6am to 11.59am
    greet = "Good morning";
  }
  else if (hour >= 12 && hour < 17) { // noon 12pm to 4.59pm
    greet = "Good afternoon";
  }
  else if (hour >= 17 && hour < 20) { // evening 5pm to 7:59pm
    greet = "Good evening";
  }
  else { // night 8pm to 5:59am
    greet = "Good night";
  }
  return greet;
}
//////////////////////////////////////////////////////////////////////////
void HomeUIElement::runEachTurn(){
  draw();
}
