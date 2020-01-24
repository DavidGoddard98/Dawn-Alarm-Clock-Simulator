// UIController.h Created on: 22-06-2018, Author: @HarryEH

#ifndef UICONTROLLER_H_
#define UICONTROLLER_H_

#include "unphone.h"            // specifics of the unPhone


class UIElement { ///////////////////////////////////////////////////////////
  protected:
    Adafruit_HX8357* m_tft;
    Adafruit_STMPE610* m_ts;

    // colour definitions
    const uint16_t BLACK =   HX8357_BLACK;
    const uint16_t BLUE =    HX8357_BLUE;
    const uint16_t RED =     HX8357_RED;
    const uint16_t GREEN =   HX8357_GREEN;
    const uint16_t CYAN =    HX8357_CYAN;
    const uint16_t ORANGE =  HX8357_ORANGE; // new colour
    const uint16_t PURPLE =  HX8357_PURPLE; // new colour
    const uint16_t PINK =    HX8357_PINK;   // new colour
    const uint16_t MAGENTA = HX8357_MAGENTA;
    const uint16_t YELLOW =  HX8357_YELLOW;
    const uint16_t WHITE =   HX8357_WHITE;
    const uint8_t  BOXSIZE = 40;
    const uint8_t  HALFBOX = (BOXSIZE / 2);
    const uint8_t  QUARTBOX = (BOXSIZE / 4);
    const uint8_t  PENRADIUS = 9; // orig: 3
    static const uint8_t NUM_BOXES = 8;
    const uint16_t colour2box[NUM_BOXES] = {
      RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, PURPLE, PINK,
    };
    const uint8_t SWITCHER = 7; // index of the switcher
    void drawSwitcher(uint16_t xOrigin = 0, uint16_t yOrigin = 0);

  public:
    UIElement(Adafruit_HX8357* tft, Adafruit_STMPE610* ts);
    virtual bool handleTouch(long x, long y) = 0;
    virtual void draw() = 0;
    virtual void runEachTurn() = 0;
    void someFuncDummy();
    void showLine(char *buf, uint16_t *yCursor);
};

// the UI elements types (screens) /////////////////////////////////////////
enum ui_modes_t {
  ui_menu = 0,
  ui_home,
  ui_alarm,
  ui_dawn,
  ui_WiFi,
  ui_config,
  ui_boot
};
extern char *ui_mode_names[];
extern uint8_t NUM_UI_ELEMENTS;  // number of UI elements (screens)

class UIController { ////////////////////////////////////////////////////////
  private:
    UIElement* m_element = 0;
    UIElement* m_menu;
    void changeMode();
    ui_modes_t m_mode;
    ui_modes_t nextMode = ui_home; // starting mode
    uint8_t modeCounter = 1; // number of mode changes % # of ui elements
  public:
    UIController(ui_modes_t);
    bool begin();
    void showUI(ui_modes_t newMode) ;
    UIElement* allocateUIElement(ui_modes_t);
    void run();
    void redraw();
    bool gotTouch();
    void handleTouch();
    void message(char *s);
    static bool provisioned;
};

#endif
