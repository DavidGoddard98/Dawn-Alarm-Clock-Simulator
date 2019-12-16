//  AllUIElement.h

#ifndef ALLUIELEMENT_H_
#define ALLUIELEMENT_H_

#include "UIController.h"

class MenuUIElement: public UIElement { /////////////////////////////////////
  private:
    void drawTextBoxes();
    uint8_t mapTextTouch(long, long);
    int8_t menuItemSelected = -1;
  public:
    MenuUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
    : UIElement(tft, ts) {
      // nothing to initialise
    };
    bool handleTouch(long x, long y);
    void draw();
    void runEachTurn();
    int8_t getMenuItemSelected() { return menuItemSelected; }
};

class HomeUIElement: public UIElement { ///////////////////////////////////
private:
  long m_timer;
public:
  HomeUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
  : UIElement(tft, ts) { m_timer = millis(); };
  bool handleTouch(long x, long y);
  void draw();
  void runEachTurn();
};

class AlarmUIElement: public UIElement { ///////////////////////////////////
  // private:
  //   long m_timer;
  public:
    AlarmUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

class DawnUIElement: public UIElement { ///////////////////////////////////
  private:
    // long m_timer;
  public:
    DawnUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

#endif
