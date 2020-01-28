// ConfigUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>

extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool ConfigUIElement::handleTouch(long x, long y) {
  return true;
}

// display provisioning message ////////////////////////////////////////////
void ConfigUIElement::draw(){
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(2);
  m_tft->setCursor(0, 5);
  m_tft->println("Could not automatically connect to WiFi");
  m_tft->println("Access point has been set up.");
  m_tft->println("Connect to access point:");
  m_tft->println(apSSID);
}

//////////////////////////////////////////////////////////////////////////
void ConfigUIElement::runEachTurn(){

}
