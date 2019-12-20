// AlarmUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>
#include <string>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <string>

using std::string;



extern tm * alarmTime;
extern tm * timeinfo;
extern bool alarm_exist;
extern bool alarmNotSet;
extern time_t alarm_time;
extern time_t time_now;
std::vector<double> x_u_r;
std::vector<double> y_u_r;

std::vector<double> x_d_l;
std::vector<double> y_d_l;

String days [7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
int hours = 0;
int mins = 0;
String the_day = "Monday";

// handle touch on this page ////////////////////////////////////////////////
bool AlarmUIElement::handleTouch(long x, long y) {



  changeTimeUR(x, y);
  changeTimeDL(x, y);
  if (confirm(x,y)) {
    return true;
  }
  return false;
}



// writes various things including mac address and wifi ssid ///////////////
void AlarmUIElement::draw(){
  //clear vectors
  x_u_r.clear();
  y_u_r.clear();
  x_d_l.clear();
  y_d_l.clear();
  //m_tft->setRotation(3);
  m_tft->setTextColor(CYAN);
  m_tft->setTextSize(3);
  m_tft->setCursor(5, 50);
  m_tft->println("Set alarm here");
  //set time
  m_tft->setFont(&FreeMonoBold9pt7b);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(7);
  m_tft->setCursor(130, 175);


  //return to menu
  drawSwitcher(440, 10);

  //Begin time
  m_tft->setCursor(70, 175);
  m_tft->setTextSize(6);

  //hours///////////////////////////////////////
  if (hours < 10) {
    m_tft->print(0);
  }
  m_tft->print(hours);
  int16_t xCor = m_tft->getCursorX();
  int16_t yCor = m_tft->getCursorY();


  //hour switchers
  drawUpArrow(70+ floor((xCor-70)/2), 77);
  drawDownArrow(70+ floor((xCor-70)/2), 207);

  m_tft->print(":");
  int16_t firXCor = m_tft->getCursorX();
  int16_t firYCor = m_tft->getCursorY();

  //mins//////////////////////////////////////
  if (mins <10) {
    m_tft->print(0);
  }
  m_tft->print(mins);
  int16_t secXCor = m_tft->getCursorX();
  int16_t secYCor = m_tft->getCursorY();

  //mins swithcers
  drawUpArrow(firXCor + floor((secXCor-firXCor)/2), 77);
  drawDownArrow(firXCor + floor((secXCor-firXCor)/2), 207);


  //set days
  //day switchers

  //DAY///////////////////////////////////////////////
  drawLeftArrow(20, 265);
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextColor(MAGENTA);
  m_tft->setTextSize(3.5);
  m_tft->setCursor(48, 280);
  m_tft->print(the_day);
  xCor = m_tft->getCursorX();
  yCor = m_tft->getCursorY();
  drawRightArrow(xCor+ 25, 265);

  // Serial.println("Up hours x:" + String(x_u_r[0]) + "Up mins x" + String(x_u_r[1]) + "Up days x" + String(x_u_r[2]));
  // Serial.println("Up hours y:" + String(y_u_r[0]) + "Up mins y" + String(y_u_r[1]) + "Up days y" + String(y_u_r[2]));
  // Serial.println("");
  // Serial.print("Down hours x:" + String(x_d_l[0]) + "Down mins x" + String(x_d_l[1]) + "Down days x" + String(x_d_l[2]));
  // Serial.print("Down hours y:" + String(y_d_l[0]) + "Down mins y" + String(y_d_l[1]) + "Down days y" + String(y_d_l[2]));
  // delay(30000);
  m_tft->fillRect(390, 245, 65, 40, WHITE);




}

void AlarmUIElement::clearMins() {
  m_tft->fillRect(  260,   100,  150,  90, HX8357_BLACK);
}

void AlarmUIElement::clearHours() {
  m_tft->fillRect(  60,   100,  150,  90, HX8357_BLACK);
}

void AlarmUIElement::clearDay() {
  m_tft->fillRect(  0,   230,  400,  80, HX8357_BLACK);
}

int test = 0;

bool AlarmUIElement::confirm(long x, long y){

  if (x>=390 && y >= 245 && y <= 285) {
    Serial.print("confirm");
    calcTime2Alarm();
    alarm_exist = true;
    alarmNotSet = true;
    return true;
  }
  return false;
}





char date1_str_day[50];

String AlarmUIElement::convertToString(char* a, int size) {
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
      s = s + a[i];
    }
    string TwoLetters = s.substr(0, 2);

    Serial.println(TwoLetters.length( ));
    Serial.println(sizeof(TwoLetters));

    Serial.println(sizeof(TwoLetters));



    if (TwoLetters == "Mo"){
      return "Monday";
    }else if (TwoLetters == "Tu") {
      return "Tuesday";
    } else if (TwoLetters == "We") {
      return "Wednesday";
    } else if (TwoLetters == "Th") {
      return "Thursday";
    } else if (TwoLetters == "Fr"){
      return "Friday";
    } else if (TwoLetters == "Sa"){
      return "Saturday";
    } else {
      return "Sunday";
    }
}

String AlarmUIElement::getNextDay(String a_day){
    if (a_day == "Monday") return "Tuesday";
    else if (a_day == "Tuesday")  return "Wednesday";
    else if (a_day == "Wednesday") return "Thursday";
    else if (a_day == "Thursday") return "Friday";
    else if (a_day == "Friday") return "Saturday";
    else if (a_day == "Saturday") return "Sunday";
    else return "Monday";


}

void AlarmUIElement::calcTime2Alarm() {
  (strftime(date1_str_day, sizeof(date1_str_day), "%A", timeinfo));
  int a_size = sizeof(date1_str_day) / sizeof(char);
  String currentDay = convertToString(date1_str_day, a_size);
  Serial.print(currentDay);
  int counter = 0;
  int tmHour = timeinfo->tm_hour;
  int tmMin = timeinfo->tm_min;
  int addedSeconds;
  if (the_day == currentDay) {
    if (hours >= tmHour && mins >= tmMin) {
      Serial.print("1");
      addedSeconds += (((hours-tmHour)*60*60) + ((mins-tmMin))*60);
    } else if (hours >= tmHour && mins < tmMin) {
      Serial.print("2");

      addedSeconds +=  ((hours-tmHour-1)*60*60) + (((60-(tmMin-mins)))*60);
    }
  } else {
    Serial.println("the_day");

    Serial.println(the_day);

    Serial.println("currentday");
    Serial.println(currentDay);
    while(the_day!=currentDay) {
      currentDay = getNextDay(currentDay);
      counter ++;
    }
    Serial.println("counter");

    Serial.println(counter);


    if (hours >= tmHour && mins >= tmMin) {
      Serial.print("3");
      addedSeconds = (counter*24*60*60 ) +  ((hours-tmHour)*60*60) + ((mins-tmMin)*60);
    } else if (hours >= tmHour && mins < tmMin) {
      Serial.print("4");

      addedSeconds = (counter*24*60*60) + ((hours-tmHour-1)*60*60) + (((60-(tmMin-mins)))*60);
    } else if (hours < tmHour && mins >= tmMin) {
      Serial.print("5");

      addedSeconds = ((counter-1)*24*60*60) + ((24-(tmHour-hours))*60*60) + ((mins-tmMin) *60);
    } else  { //(hours < tmHour && mins < tmMin )
      Serial.print("6");

      addedSeconds = (counter-1)*24*60*60 + ((23-(tmHour-hours))*60*60) + (59-(tmMin-mins)) *60;
    }
    Serial.print("seconds added on");
    Serial.print(addedSeconds );
    alarm_time = time_t(time_now) + addedSeconds;
    // alarmTime = localtime(&alarm_time);
  }
}
// Tuesday
// counter
// 3
// 6seconds added on205740Alarm is set for: 1577007900

//up and right
void AlarmUIElement::changeTimeUR(long x, long y) {
  for (int i=0; i<3; i++ ){
    if ((x >= x_u_r[i]) && (x <=  x_u_r[i] + 50)) {
      if ((y <= y_u_r[i]) && (y >=  y_u_r[i] - 45)) {
        //register touch
        if (i ==0) {
          clearHours();
          if (hours == 23) hours = 0;
          else hours++;
        }
        if (i ==1)  {
          clearMins();
          if (mins == 59) mins = 0;
          else mins++;
        }
        if (i ==2) {
          clearDay();
          if (test ==6) test = 0;
          else {
            test++;
            the_day = days[test];
          }
        }

      }
    }

  }
}

//down and left
void AlarmUIElement::changeTimeDL(long x, long y) {
  for (int i=0; i<3; i++ ){
    if ((x <= x_d_l[i]) && (x >=  x_d_l[i] - 50)) {
      if ((y >= y_d_l[i] - 10) && (y <=  y_d_l[i] + 25)) {
        //register touch
        if (i ==0) {
          clearHours();
          if (hours == 0) hours = 23;
          else hours--;
        }
        if (i ==1)  {
          clearMins();
          if (mins == 0) mins = 59;
          else mins--;
        }
        if (i ==2) {
          clearDay();
          if (test ==0) test = 6;
          else {
            test--;
            the_day = days[test];
          }
        }

      }
    }

  }
}

void AlarmUIElement::drawUpArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_u_r.push_back(xOrigin - 25);
  y_u_r.push_back(yOrigin + 12.5);

  m_tft->fillTriangle(
    xOrigin - 25,    yOrigin + 12.5,
    xOrigin,         yOrigin - 12.5,
    xOrigin + 25,    yOrigin   + 12.5,
    WHITE
  );
}

//Up hours x:78.00Up mins x210.00Up days x252.50
//Up hours y:89.50Up mins y89.50Up days y290.00

//Down hours x:128.00Down mins x260.00Down days x32.50
//Down hours y:194.50Down mins y194.50Down days y240.00
// x:288y:198
// down mins
// down days

void AlarmUIElement::drawDownArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_d_l.push_back(xOrigin + 25);
  y_d_l.push_back(yOrigin - 12.5);

  m_tft->fillTriangle(
    xOrigin + 25,    yOrigin - 12.5,
    xOrigin,         yOrigin + 12.5,
    xOrigin - 25,    yOrigin - 12.5,
    WHITE
  );
}

void AlarmUIElement::drawRightArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_u_r.push_back(xOrigin - 12.5);
  y_u_r.push_back(yOrigin + 25);

  m_tft->fillTriangle(
    xOrigin - 12.5,   yOrigin + 25,
    xOrigin + 12.5,   yOrigin,
    xOrigin - 12.5,   yOrigin - 25,
    WHITE
  );
}

void AlarmUIElement::drawLeftArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_d_l.push_back(xOrigin + 12.5);
  y_d_l.push_back(yOrigin - 25);

  m_tft->fillTriangle(
    xOrigin + 12.5,   yOrigin - 25,
    xOrigin - 12.5,   yOrigin,
    xOrigin + 12.5,   yOrigin + 25,
    WHITE
  );
}

//////////////////////////////////////////////////////////////////////////
void AlarmUIElement::runEachTurn(){
  draw();
}
