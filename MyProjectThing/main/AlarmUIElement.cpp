// AlarmUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>

extern int firmwareVersion;
extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool AlarmUIElement::handleTouch(long x, long y) {
  Serial.print("x: "); Serial.println(x);
  Serial.print("y: "); Serial.println(y);
  //
  return true;
}

// writes various things including mac address and wifi ssid ///////////////
void AlarmUIElement::draw(){
  //m_tft->setRotation(3);
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(3);
  m_tft->setCursor(5, 50);
  m_tft->println("Set alarm here");
}

//////////////////////////////////////////////////////////////////////////
void AlarmUIElement::runEachTurn(){

}
