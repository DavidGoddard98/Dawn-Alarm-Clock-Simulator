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
  int normaliseF(float x);
  int normaliseS(String x);
  void drawWeather();
  void drawGreeting();
  void clearSec();
  void drawTime();
  void clearMin();
  void clearHour();
  void clearAlarm(int x);
  void clearAlarmSec();
  void drawDate();
  void clearGreeting();
  void flashDots();
  void drawAlarmTime();
  void clearDate();
  void drawNoAlarm();
};

class AlarmUIElement: public UIElement { ///////////////////////////////////
  public:
    AlarmUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
    void drawUpArrow(uint16_t xOrigin, uint16_t yOrigin);
    void drawDownArrow(uint16_t xOrigin, uint16_t yOrigin);
    void drawLeftArrow(uint16_t xOrigin, uint16_t yOrigin);
    void drawRightArrow(uint16_t xOrigin, uint16_t yOrigin);
    void changeTimeUR(long x, long y);
    void changeTimeDL (long x, long y);
    void clearMins();
    void clearHours();
    void testing(String test_day, int test_hours, int test_mins);
    void testTimeCalc();
    void clearDay();
    void calcTime2Alarm();
    String getNextDay(String a_day);
    String convertToString(char* a);
    int convertDayToNum(String a_day);
    bool alarmAction(long x, long y);
    void clearMessage();

};

class DawnUIElement: public UIElement { ///////////////////////////////////
  private:
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
  public:
    WiFiUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

class ConfigUIElement: public UIElement { ///////////////////////////////////
  private:
  public:
    ConfigUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

class BootUIElement: public UIElement { ///////////////////////////////////
  private:
  public:
    BootUIElement (Adafruit_HX8357* tft, Adafruit_STMPE610* ts)
     : UIElement(tft, ts) { };
    bool handleTouch(long, long);
    void draw();
    void runEachTurn();
};

#endif
