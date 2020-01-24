// MenuUIElement.cpp

#include "AllUIElement.h"
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
extern bool m_first_touch;
// handle touch on this page
// @returns true if the touch is a menu item
bool MenuUIElement::handleTouch(long x, long y) {
  Serial.print("x: "); Serial.println(x);
  Serial.print("y: "); Serial.println(y);
  // D("text mode: responding to touch @ %d/%d/%d: ", x, y,-1)
  m_tft->setTextColor(WHITE, BLACK);
  uint8_t menuItem = mapTextTouch(x, y);
  D("menuItem=%d, ", menuItem)

  if(menuItem > 0 && menuItem <= NUM_UI_ELEMENTS) {
    menuItemSelected = menuItem;
    return true;
  }


  return false;
}

// returns menu item number //////////////////////////////////////////////
uint8_t MenuUIElement::mapTextTouch(long xInput, long yInput) {
  for(int y = 35, i = 1; y < 320; y += 48, i++)
    if(yInput > y && yInput < y + 48)
      return i;
  return -1;
}

// draw a textual menu ///////////////////////////////////////////////////
void MenuUIElement::draw(){
  m_tft->setFont(&FreeMono9pt7b);
  m_tft->setTextSize(2);
  m_tft->setTextColor(RED);
  m_tft->setCursor(230, 25);
  m_tft->print("MENU");

  uint16_t yCursor = 35;
  m_tft->drawFastHLine(0, yCursor, 480, MAGENTA);
  yCursor += 33;

  for(int i = 1; i <= NUM_UI_ELEMENTS; i++) {
    m_tft->setTextColor(BLUE);
    m_tft->setCursor(0, yCursor);
    m_tft->print(ui_mode_names[i]);
    drawSwitcher(448, yCursor - 30);
    yCursor += 15;
    m_tft->drawFastHLine(0, yCursor, 480, MAGENTA);
    yCursor += 33;
  }
}

//////////////////////////////////////////////////////////////////////////
void MenuUIElement::runEachTurn(){ // text page UI, run each turn
  draw();
}
