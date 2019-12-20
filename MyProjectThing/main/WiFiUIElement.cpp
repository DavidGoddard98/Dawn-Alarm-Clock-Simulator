// WiFiUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>

extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool WiFiUIElement::handleTouch(long x, long y) {
  return true;
}

// writes various things including mac address and wifi ssid ///////////////
void WiFiUIElement::draw(){
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(3);
  m_tft->setCursor(5, 5);
  m_tft->println("Attmepting to connect " );
  m_tft->println("to last known WIFI:");
  m_tft->println(apSSID);

}

//////////////////////////////////////////////////////////////////////////
void WiFiUIElement::runEachTurn(){

}
