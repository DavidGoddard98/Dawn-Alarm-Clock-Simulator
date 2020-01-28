// WiFiUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>

extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool WiFiUIElement::handleTouch(long x, long y) {
  return true;
}

// display autoconnect message //////////////////////////////////////////////
void WiFiUIElement::draw(){
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(2);
  m_tft->setCursor(0, 5);
  m_tft->println("Attmepting to connect");
  m_tft->println("to last known WIFI...");
}

//////////////////////////////////////////////////////////////////////////
void WiFiUIElement::runEachTurn(){

}
