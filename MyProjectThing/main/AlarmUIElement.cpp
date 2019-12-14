// AlarmUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>

extern int firmwareVersion;
extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool AlarmUIElement::handleTouch(long x, long y) {
  return y < BOXSIZE && x > (BOXSIZE * SWITCHER);
}

// writes various things including mac address and wifi ssid ///////////////
void AlarmUIElement::draw(){
  // say hello
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);
  uint16_t yCursor = 0;
  m_tft->setCursor(0, yCursor);
  m_tft->print("Hey, the date today is:");
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

//////////////////////////////////////////////////////////////////////////
void AlarmUIElement::runEachTurn(){

}
