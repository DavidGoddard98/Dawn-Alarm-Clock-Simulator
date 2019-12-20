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
  void clearTime();
  void drawGreeting();
  void clearSec();
  void drawTime();
  void clearMin();
  void clearHour();
  void clearAlarm();
  void clearAlarmSec();
  void drawDate();
  void flashDots();
  void drawAlarmTime();
  void clearDate();
  void drawNoAlarm();
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
    void getDawnColour(long x, long y);
    void drawDawnColour();
    void runEachTurn();
};

class WiFiUIElement: public UIElement { ///////////////////////////////////
  private:
    // long m_timer;
  public:
    WiFiUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

class ConfigUIElement: public UIElement { ///////////////////////////////////
  private:
    // long m_timer;
  public:
    ConfigUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

class BootUIElement: public UIElement { ///////////////////////////////////
  private:
    // long m_timer;
  public:
    BootUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

#endif
