// WiFiUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>
#include "WiFiManager.h"

extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool WiFiUIElement::handleTouch(long x, long y) {
  return true;
}

// display autoconnect message //////////////////////////////////////////////
void WiFiUIElement::draw(){
  m_tft->setTextColor(YELLOW);
  m_tft->setTextSize(3);
  m_tft->setCursor(0, 10);

}

//////////////////////////////////////////////////////////////////////////
void WiFiUIElement::runEachTurn(){
}
