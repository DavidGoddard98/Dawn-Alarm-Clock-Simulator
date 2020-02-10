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
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);
  m_tft->setCursor(0, 5);
  m_tft->println("Please wait a few seconds for the device");
  m_tft->println("to connect to the previously saved WiFi.");
  m_tft->println();
  delay(10000);
  if (WiFi.status() != WL_CONNECTED) {
    m_tft->fillScreen(HX8357_BLACK);
    m_tft->setCursor(0, 5);
    m_tft->setTextColor(RED);
    m_tft->println("No connection found...");
    m_tft->println();
    m_tft->setTextColor(YELLOW);
    m_tft->println("An access point will be set up");
    m_tft->println("for 4 minutes, afterwhich the device ");
    m_tft->println("will restart: ");
    m_tft->println();
    m_tft->setTextColor(CYAN);
    m_tft->println(apSSID);
  }


}

//////////////////////////////////////////////////////////////////////////
void ConfigUIElement::runEachTurn(){

}
