// ConfigUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>

extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool ConfigUIElement::handleTouch(long x, long y) {
  return true;
}

// writes various things including mac address and wifi ssid ///////////////
void ConfigUIElement::draw(){
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(2);
  m_tft->setCursor(0, 5);
  m_tft->println("Access point has been set up.");
  m_tft->println("Please connet to WiFi");
  m_tft->println("through phone.");
}

//////////////////////////////////////////////////////////////////////////
void ConfigUIElement::runEachTurn(){

}
