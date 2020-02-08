// BootUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>

extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool BootUIElement::handleTouch(long x, long y) {
  return true;
}

// display boot message /////////////////////////////////////////////////////
void BootUIElement::draw(){
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(3);
  m_tft->setCursor(0, 10);
  m_tft->print("BOOTING");
  for (int i = 0; i < 5; i++) {
    WAIT_MS(250)
    m_tft->print(".");
  }
}

//////////////////////////////////////////////////////////////////////////
void BootUIElement::runEachTurn(){
  draw();
}
