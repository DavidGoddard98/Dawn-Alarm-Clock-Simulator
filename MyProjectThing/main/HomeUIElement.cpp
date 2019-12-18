// HomeUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>
#include "main.cpp"


extern int firmwareVersion;
extern String apSSID;

char time_str[50];
char date_str[50];
char date_str_day[50];
char date_str_month[50];
char date_str_year[50];
bool display = true;
String greeting();

// handle touch on this page ////////////////////////////////////////////////
bool HomeUIElement::handleTouch(long x, long y) {
  return true;
}

// writes various things including mac address and wifi ssid ///////////////
void HomeUIElement::draw(){
  //m_tft->setRotation(3);

  m_tft->setRotation(1);
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(3);
  m_tft->setCursor(5, 5);
  m_tft->println(greeting());
  time_t rawtime;
  struct tm *info;
  time( &rawtime );
  info = localtime( &rawtime );
  strftime(date_str_day, sizeof(date_str_day), "%A", info);
  strftime(date_str_month, sizeof(date_str_month), "%B", info);
  strftime(date_str_year, sizeof(date_str_year), "%Y", info);
  strftime(time_str, sizeof(time_str), "%H:%M:%S", info);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(6);
  m_tft->setCursor(15, 150);
  m_tft->print(String(time2Alarm()));
  m_tft->setCursor(15, 200);
  m_tft->print(time_str);
  delay(1000);
  m_tft->fillRect(  0,   100,  400,  250, BLACK);
  m_tft->setTextColor(MAGENTA);
  m_tft->setTextSize(4);
  m_tft->setCursor(5, 345);
  m_tft->print(date_str_day);m_tft->print(" ");m_tft->print(info->tm_mday);m_tft->print(",");
  m_tft->setCursor(5, 395);
  m_tft->print(date_str_month);m_tft->print(",");
  m_tft->setCursor(5, 445);
  m_tft->print(date_str_year);
}

String greeting() {
  String greet;
  time_t rawtime;
  struct tm *info;
  time( &rawtime );
  info = localtime( &rawtime );
  int hour = info->tm_hour;
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


//
// void AlarmUIElement::draw_time(char* date_string, char* time_string){
//   // say hello
//   m_tft->setTextColor(GREEN);
//   m_tft->setTextSize(2);
//   uint16_t yCursor = 0;
//   m_tft->setCursor(0, yCursor);
//   m_tft->print("Hey, the date today is:");
//   m_tft->print(date_string);
//   m_tft->setTextColor(BLUE);
//   m_tft->print("The time is:");
//   m_tft->print(time_string);
// }
int loopit = 0;
//////////////////////////////////////////////////////////////////////////
void HomeUIElement::runEachTurn(){
  draw();



}
