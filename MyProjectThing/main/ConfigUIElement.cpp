// ConfigUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>
extern String apSSID;
extern int bootCountt;

// handle touch on this page ////////////////////////////////////////////////
bool ConfigUIElement::handleTouch(long x, long y) {
  return true;
}

// display provisioning message ////////////////////////////////////////////
void ConfigUIElement::draw(){
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);
  m_tft->setCursor(0, 5);
  m_tft->println("Please wait a few seconds for the device");
  m_tft->println("to connect to the previously saved WiFi.");
  m_tft->println();
  m_tft->setTextColor(YELLOW);
  m_tft->println("Device will also check for any updates...");
  m_tft->println();
  m_tft->println();
  m_tft->setTextColor(CYAN);
  m_tft->println("If no connection is found, an access ");
  m_tft->println("point will be set up for 4 minutes, ");
  m_tft->println("afterwhich the device will restart: ");
  m_tft->println();
  m_tft->setTextColor(PINK);
  m_tft->println(apSSID);



}

//////////////////////////////////////////////////////////////////////////
void ConfigUIElement::runEachTurn(){

}
