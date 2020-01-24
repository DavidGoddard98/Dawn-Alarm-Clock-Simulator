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
  // if touch is within the colour select rectangle or switcher...
  if (x >= 80 && x <= (80 + BOXSIZE*8) && y >= 105 && y <= 265 || (x>= 430 && y  <=25)) {
    Serial.print("x: "); Serial.println(x);
    Serial.print("y: "); Serial.println(y);
    getDawnColour(x,y);
    return true;
  }
  return false;
}

// draw switcher and page text //////////////////////////////////////////////
void DawnUIElement::draw(){
  drawSwitcher(440,10);
  m_tft->setFont(&FreeSans9pt7b);
  drawDawnColour();
  m_tft->setTextColor(WHITE);
  m_tft->setTextSize(2.5);
  m_tft->setCursor(10, 60);
  m_tft->println("Select Dawn Colour");
}

// draw dawn colour boxes ///////////////////////////////////////////////////
void DawnUIElement::drawDawnColour() {
  // colour2box[i] ~> RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, PURPLE, PINK
  int j = 0;
  for(uint8_t i = 0; i < NUM_BOXES; i++) {
    if (i < 4) { // row 1 displays RED, ORANGE, YELLOW, GREEN
      // 80x80 boxes are seamlessly laid out next to each over
      m_tft->fillRect(80+(i * BOXSIZE*2), 105, BOXSIZE*2, BOXSIZE*2, colour2box[i]);
    }
    else { // row 2 displays CYAN, BLUE, PURPLE, PINK
      m_tft->fillRect(80+(j * BOXSIZE*2), 185, BOXSIZE*2, BOXSIZE*2, colour2box[i]);
      j++;
    }
  }
}

// get selected dawn colour ////////////////////////////////////////////////
void DawnUIElement::getDawnColour(long x, long y) {
  uint16_t colour;
  int j = 0;
  if (y < 185) {
    // similar structure to drawring colour boxes for registering selected colour
    for (uint8_t i = 0; i < 4; i++) {  // row 1
      if (x < (80+((i+1)*BOXSIZE*2))) {
        colour = colour2box[i];
        Serial.println(colour);
        setDawnColour(colour);
        break; // we want first colour registered
      }
    }
  }
    else if (y >= 185) {
      for (uint8_t j = 0; j < 4; j++) { // row 2
        if (x < (80+((j+1)*BOXSIZE*2))) {
          colour = colour2box[j+4]; // 2nd row starts from CYAN
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
