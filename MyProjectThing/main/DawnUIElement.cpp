// DawnUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>

extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool DawnUIElement::handleTouch(long x, long y) {
  return true;
}

// writes various things including mac address and wifi ssid ///////////////
void DawnUIElement::draw(){

  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(3);
  m_tft->setCursor(5, 50);
  m_tft->println("Set dawn here");
}

//////////////////////////////////////////////////////////////////////////
void DawnUIElement::runEachTurn(){

}
