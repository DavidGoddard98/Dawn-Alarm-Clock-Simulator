// UIController.cpp

#include "AllUIElement.h"
extern int toggle;

// initialisation flag, not complete until parent has finished config
bool UIController::provisioned = false;

// the UI elements types (screens) /////////////////////////////////////////
char *ui_mode_names[] = {
  "Menu",
  "Home Display",
  "Set Alarm",
  "Set Dawn Color",
  "Joining Wifi",
  "AP config",
  "Boot"
};
uint8_t NUM_UI_ELEMENTS = 3;  // number of UI elements

// keep Arduino IDE compiler happy /////////////////////////////////////////
UIElement::UIElement(Adafruit_HX8357* tftp, Adafruit_STMPE610* tsp) {
  m_tft = tftp;
  m_ts = tsp;
}
void UIElement::someFuncDummy() { }

// constructure for the main class /////////////////////////////////////////
UIController::UIController(ui_modes_t start_mode) {
  m_mode = start_mode;
}

bool UIController::begin() { ////////////////////////////////////////////////
  D("UI.begin()\n")

  unPhone::tftp->setRotation(0);
  unPhone::tftp->fillScreen(HX8357_RED);
  WAIT_MS(50)
  unPhone::tftp->fillScreen(HX8357_BLACK);
  WAIT_MS(50)

  unPhone::tftp->setRotation(3);
  unPhone::tftp->fillScreen(HX8357_CYAN);
  WAIT_MS(50)
  unPhone::tftp->fillScreen(HX8357_BLACK);
  WAIT_MS(50)

  unPhone::tftp->setRotation(2);
  unPhone::tftp->fillScreen(HX8357_GREEN);
  WAIT_MS(50)
  unPhone::tftp->fillScreen(HX8357_BLACK);
  WAIT_MS(50)

  unPhone::tftp->setRotation(1);
  unPhone::tftp->fillScreen(HX8357_PURPLE);
  WAIT_MS(50)
  unPhone::tftp->fillScreen(HX8357_BLACK);

  // define the menu element and the first m_element here
  m_menu = new MenuUIElement(unPhone::tftp, unPhone::tsp);
  allocateUIElement(m_mode);

  redraw();
  return true;
}

UIElement* UIController::allocateUIElement(ui_modes_t newMode) {
  if(m_element != 0 && m_element != m_menu) delete(m_element);

  switch(newMode) {
    case ui_menu:
      m_element = m_menu;
      break;
    case ui_home:
      m_element = new HomeUIElement(unPhone::tftp, unPhone::tsp);
      break;
    case ui_alarm:
      m_element = new AlarmUIElement(unPhone::tftp, unPhone::tsp);
      break;
    case ui_dawn:
      m_element = new DawnUIElement(unPhone::tftp, unPhone::tsp);
      break;
    case ui_WiFi:
      m_element = new WiFiUIElement(unPhone::tftp, unPhone::tsp);
      break;
    case ui_config:
      m_element = new ConfigUIElement(unPhone::tftp, unPhone::tsp);
      break;
    case ui_boot:
      m_element = new BootUIElement(unPhone::tftp, unPhone::tsp);
      break;
    default:
      Serial.printf("invalid UI mode %d in allocateUIElement\n", newMode);
      m_element = m_menu;
  }

  return m_element;
}

// touch management code ////////////////////////////////////////////////////
TS_Point nowhere(-1, -1, -1);    // undefined coordinate
TS_Point firstTouch(0, 0, 0);    // the first touch defaults to 0,0,0
TS_Point p(-1, -1, -1);          // current point of interest (signal)
TS_Point prevSig(-1, -1, -1);    // the previous accepted touch signal
bool firstTimeThrough = true;    // first time through gotTouch() flag
uint16_t fromPrevSig = 0;        // distance from previous signal
unsigned long now = 0;           // millis
unsigned long prevSigMillis = 0; // previous signal acceptance time
unsigned long sincePrevSig = 0;  // time since previous signal acceptance
uint16_t DEFAULT_TIME_SENSITIVITY = 125; // min millis between touches
uint16_t TIME_SENSITIVITY = DEFAULT_TIME_SENSITIVITY;
uint16_t DEFAULT_DIST_SENSITIVITY = 200; // min distance between touches
uint16_t DIST_SENSITIVITY = DEFAULT_DIST_SENSITIVITY;
uint16_t TREAT_AS_NEW = 600;     // if no signal in this period treat as new
uint8_t MODE_CHANGE_TOUCHES = 1; // number of requests needed to switch mode
uint8_t modeChangeRequests = 0;  // number of current requests to switch mode
void setTimeSensitivity(uint16_t s = DEFAULT_TIME_SENSITIVITY) { ////////////
  TIME_SENSITIVITY = s;
}
void setDistSensitivity(uint16_t d = DEFAULT_DIST_SENSITIVITY) { ////////////
  DIST_SENSITIVITY = d;
}
uint16_t distanceBetween(TS_Point a, TS_Point b) { // coord distance ////////
  uint32_t xpart = b.x - a.x, ypart = b.y - a.y;
  xpart *= xpart; ypart *= ypart;
  return sqrt(xpart + ypart);
}
void dbgTouch() { // print current state of touch model /////////////////////
  if(touchDBG) {
    D("p(x:%04d,y:%04d,z:%03d)", p.x, p.y, p.z)
    D(", now=%05lu, sincePrevSig=%05lu, prevSig=", now, sincePrevSig)
    D("p(x:%04d,y:%04d,z:%03d)", prevSig.x, prevSig.y, prevSig.z)
    D(", prevSigMillis=%05lu, fromPrevSig=%05u", prevSigMillis, fromPrevSig)
  }
}

// accept or reject touch signals ///////////////////////////////////////////
bool UIController::gotTouch() {
  if(!unPhone::tsp->touched()) {
    return false; // no touches
  }

  // set up timings
  now = millis();
  if(firstTimeThrough) {
    sincePrevSig = TIME_SENSITIVITY + 1;
  } else {
    sincePrevSig = now - prevSigMillis;
  }

  // retrieve a point
  p = unPhone::tsp->getPoint();
  //weird readings outputted occasionaly from touch at point x~16300 , y~-12500, z~025
  //dont read these and continue - means touch is WAY more responsive
  if (p.x > 10000) {
    return false;
  }

  if(p == firstTouch && firstTimeThrough) {
    dbgTouch();
    if(touchDBG) D(", rejecting (0)\n\n")
    return false;
  }
  firstTimeThrough = false;

  // calculate distance from previous signal
  fromPrevSig = distanceBetween(p, prevSig);
  //dbgTouch();

  if(touchDBG)
    D(", sincePrevSig<TIME_SENS.: %d...  ", sincePrevSig<TIME_SENSITIVITY)
  if(sincePrevSig < TIME_SENSITIVITY) { // ignore touches too recent
    if(touchDBG) D("rejecting (2)\n")
  } else if(fromPrevSig < DIST_SENSITIVITY && sincePrevSig < TREAT_AS_NEW) {
    if(touchDBG) D("rejecting (3)\n")
  } else {
    prevSig = p;
    prevSigMillis = now;
    D("decided this is a new touch\n")
    firstTimeThrough=true;
    return true;
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
void UIController::changeMode() {
  D("changing mode from %d to...", m_mode)
  unPhone::tftp->fillScreen(HX8357_BLACK);
  setTimeSensitivity(); // set TIME_SENS to the default
  // allocate an element according to nextMode and
  toggle = 0;
  if(m_mode == ui_menu) {       // coming OUT of menu
    D("...%d\n", nextMode)
    int8_t menuSelection = ((MenuUIElement *)m_menu)->getMenuItemSelected();
    if(menuSelection != -1)     // if user selected an item use it
      nextMode = (ui_modes_t) menuSelection; // (else use current nextMode)
    m_mode =    nextMode;
    m_element = allocateUIElement(nextMode);
  } else {                      // going INTO menu
    D("...%d (menu)\n", ui_menu)
    modeCounter = ++modeCounter % NUM_UI_ELEMENTS; // calculate next mode
    if(modeCounter == 0) modeCounter++; // wrap through
    nextMode = (ui_modes_t) modeCounter;
    dbf(miscDBG, "nextMode=%d, modeCounter=%d\n", nextMode, modeCounter);
    m_mode =   ui_menu;
    m_element = m_menu;
  }
  redraw();
  return;
}

/////////////////////////////////////////////////////////////////////////////
void UIController::showUI(ui_modes_t newMode) {
  m_mode = newMode;
  m_element = allocateUIElement(newMode);
  redraw();
}

/////////////////////////////////////////////////////////////////////////////
void UIController::handleTouch() {
  // adjust mapping for landscape orientation
  int16_t nTmpX = p.x; // temp p.x so that p.y updates
  p.x =
    map(p.y, unPhone::TS_MINY, unPhone::TS_MAXY, 0, unPhone::tftp->width());
  p.y =
    map(nTmpX, unPhone::TS_MINX, unPhone::TS_MAXX, 0, unPhone::tftp->height());
  // unPhone::tftp->fillRect(p.x-1,p.y-1,2,2,HX835 7_GREEN); // DEBUG touch feedback
  if(m_element->handleTouch(p.x, p.y)) {
    if(++modeChangeRequests >= MODE_CHANGE_TOUCHES) {
      changeMode();
      modeChangeRequests = 0;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
void UIController::run() {
  m_element->runEachTurn();
}

////////////////////////////////////////////////////////////////////////////
void UIController::redraw() {
  unPhone::tftp->fillScreen(HX8357_BLACK);
  m_element->draw();
}

////////////////////////////////////////////////////////////////////////////
void UIController::message(char *s) {
  unPhone::tftp->setCursor(0, 465);
  unPhone::tftp->setTextSize(2);
  unPhone::tftp->setTextColor(HX8357_CYAN, HX8357_BLACK);
  unPhone::tftp->print("                          ");
  unPhone::tftp->setCursor(0, 465);
  unPhone::tftp->print(s);
}

////////////////////////////////////////////////////////////////////////////
void UIElement::drawSwitcher(uint16_t xOrigin, uint16_t yOrigin) {
  uint16_t leftX = xOrigin;
  if(leftX == 0)
    leftX = (SWITCHER * BOXSIZE) + 8; // default is on right hand side
    m_tft->fillRect(leftX, 15 + yOrigin, BOXSIZE - 15, HALFBOX - 10, WHITE);
    m_tft->fillTriangle(
      leftX + 15, 35 + yOrigin,
      leftX + 15,  5 + yOrigin,
      leftX + 30, 20 + yOrigin,
      WHITE
    );
}

////////////////////////////////////////////////////////////////////////////
void UIElement::showLine(char *buf, uint16_t *yCursor) {
  *yCursor += 20;
  m_tft->setCursor(0, *yCursor);
  m_tft->print(buf);
}
