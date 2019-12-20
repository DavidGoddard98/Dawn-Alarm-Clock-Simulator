// DawnUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>

extern String apSSID;
void setDawnColour(uint16_t col);

// handle touch on this page ////////////////////////////////////////////////
bool DawnUIElement::handleTouch(long x, long y) {
  if (x >= 80 && x <= (80 + BOXSIZE*8) && y >= 105 && y <= 265 || (x>= 430 && y  <=25)) {
    Serial.print("x: "); Serial.println(x);
    Serial.print("y: "); Serial.println(y);
    getDawnColour(x,y);
    return true;
  }
  return false;
}

// writes various things including mac address and wifi ssid ///////////////
void DawnUIElement::draw(){
  drawSwitcher(440,10);
  m_tft->setFont(&FreeSans9pt7b);
  drawDawnColour();
  m_tft->setTextColor(WHITE);
  m_tft->setTextSize(2.5);
  m_tft->setCursor(10, 60);
  m_tft->println("Select Dawn Colour");
}

void DawnUIElement::drawDawnColour() {
  int j = 0;
  for(uint8_t i = 0; i < NUM_BOXES; i++) {
    if (i < 4) {
      m_tft->fillRect(80+(i * BOXSIZE*2), 105, BOXSIZE*2, BOXSIZE*2, colour2box[i]);
    }
    else {
      m_tft->fillRect(80+(j * BOXSIZE*2), 185, BOXSIZE*2, BOXSIZE*2, colour2box[i]);
      j++;
    }
  }
}

void DawnUIElement::getDawnColour(long x, long y) {
  uint16_t colour;
  int j = 0;
  if (y < 185) {
    for (uint8_t i = 0; i < 4; i++) {
      if (x < (80+((i+1)*BOXSIZE*2))) {
        colour = colour2box[i];
        Serial.println(colour);
        setDawnColour(colour);
        break;
      }
    }
  }
    else if (y >= 185) {
      for (uint8_t j = 0; j < 4; j++) {
        if (x < (80+((j+1)*BOXSIZE*2))) {
          colour = colour2box[j+4];
          Serial.println(colour);
          setDawnColour(colour);
          break;
        }
      }
    }
}

//////////////////////////////////////////////////////////////////////////
void DawnUIElement::runEachTurn(){

}
