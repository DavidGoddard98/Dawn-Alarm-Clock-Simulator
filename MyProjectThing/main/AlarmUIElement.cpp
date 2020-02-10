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
extern time_t dawn_time;
extern time_t time_now;

void pixelsOff();

//Vectors to store positions of UP and RIGHT arrows.. (NOTE THIS IS V.WEIRD IMPLEMENTATION...)
//Right arrows dynamically change and move around depending on length of day...
//There are also two up arrows so these are also stored in vector.
//x_u_r Vector look like -> [X,Y,Z]
//X -> x position of arrow above hours
//Y -> x posotion of arrow above mins
//Z -> x position of arrow right of days

std::vector<double> x_u_r; //x stands for x coord, u for up arrow, r for right arrow..
std::vector<double> y_u_r;

//Vectors to store positions of DOWN and RIGHT arrows..
std::vector<double> x_d_l;
std::vector<double> y_d_l;

//variables used to display time and day...
String days [7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
String the_day;
int dayAsNum;
int hours = 0;
int mins = 0;
int toggle = 0;
char date1_str_day[10];
boolean testingCalc = false;

// handle touch on this page ////////////////////////////////////////////////
bool AlarmUIElement::handleTouch(long x, long y) {
  //Increment the displayed time if UP or Right arrow pressed
  changeTimeUR(x, y);
  //Decrement the displayed time if DOWN or LEFT arrow pressed
  changeTimeDL(x, y);
  //confirm button sets alarm time based on what user has toggled
  if (alarmAction(x,y) || x>= 430 && y  <=25) {
    return true;
  }
  return false;
}

// Draws the page..
//Arrows are dynamically drawn, i.e right and left arrows are changing depending on
//number of characters in string.. i.e Monday < Wednesday
void AlarmUIElement::draw(){
  //autofills the time and day to the time and day right now..
  if (toggle == 0) {
    hours = timeinfo->tm_hour;
    mins = timeinfo->tm_min;
    (strftime(date1_str_day, sizeof(date1_str_day), "%A", timeinfo));
    the_day = convertToString(date1_str_day);
    toggle = 1;
    dayAsNum = convertDayToNum(the_day); //used for string array of days
    m_tft->fillRoundRect(390, 215, 80, 42, 15, ORANGE);
    m_tft->fillRoundRect(390, 270, 80, 42, 15, RED);
  }
  //clear vectors
  x_u_r.clear();
  y_u_r.clear();
  x_d_l.clear();
  y_d_l.clear();

  //setup font, text size for the "Set alarm time"
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextSize(3.5);
  m_tft->setTextColor(RED);
  m_tft->setCursor(5, 42);
  m_tft->println("Set Alarm Here");

  //set time font and size
  m_tft->setFont(&FreeMonoBold9pt7b);
  m_tft->setTextColor(RED);
  m_tft->setTextSize(7);
  m_tft->setCursor(130, 175);

  //return to menu switcher
  drawSwitcher(440, 10);

  //Begin time
  m_tft->setCursor(70, 175);
  m_tft->setTextSize(6);

  //HOURS///////////////////////////////////////////////////////////////////////
  if (hours < 10) {
    m_tft->print(0); // ensures always 2 digits shown
  }
  m_tft->print(hours); //print the hour that has been toggled
  int16_t xCor = m_tft->getCursorX(); //finds x position of hour placed - used to draw arrow
  int16_t yCor = m_tft->getCursorY();

  //hour switchers
  drawUpArrow(70+ floor((xCor-70)/2), 77);
  drawDownArrow(70+ floor((xCor-70)/2), 207);

  m_tft->print(":");
  int16_t firXCor = m_tft->getCursorX(); //used to draw switchers
  int16_t firYCor = m_tft->getCursorY();

  //MINS////////////////////////////////////////////////////////////////////////
  if (mins <10) {
    m_tft->print(0); // ensures always 2 digits shown
  }
  m_tft->print(mins);//print the min that has been toggled
  int16_t secXCor = m_tft->getCursorX();
  int16_t secYCor = m_tft->getCursorY();

  //mins swithcers
  drawUpArrow(firXCor + floor((secXCor-firXCor)/2), 77);
  drawDownArrow(firXCor + floor((secXCor-firXCor)/2), 207);

  //DAY////////////////////////////////////////////////////////////////////////
  drawLeftArrow(20, 265);
  m_tft->setFont(&FreeSans9pt7b);
  m_tft->setTextColor(RED);
  m_tft->setTextSize(3.5);
  m_tft->setCursor(48, 280);
  m_tft->print(the_day);  //print what day has been toggled.
  xCor = m_tft->getCursorX();
  yCor = m_tft->getCursorY();
  drawRightArrow(xCor+ 25, 265); //dynamically changes depending on length of day...
  m_tft->setTextSize(2);
  m_tft->setCursor(396, 248);
  m_tft->print("SET");
  m_tft->setTextColor(YELLOW);
  m_tft->setCursor(392, 303);
  m_tft->print("CLR");
}

//Methods to clear parts of screen to show any update
void AlarmUIElement::clearMins() {
  m_tft->fillRect(  260,   100,  150,  90, HX8357_BLACK);
}

void AlarmUIElement::clearHours() {
  m_tft->fillRect(  60,   100,  150,  90, HX8357_BLACK);
}

void AlarmUIElement::clearDay() {
  m_tft->fillRect(  0,   230,  380,  80, HX8357_BLACK);
}

void AlarmUIElement::clearMessage() {
  m_tft->fillRect(  0,   0,  450,  60, HX8357_BLACK);

}

//When confirm button pressed set/clear alarm to time that has been toggled by user
bool AlarmUIElement::alarmAction(long x, long y){
  if (x>=390 && y >= 215 && y <= 257) { //position of confirm box
    Serial.print("confirm alarm");
    //this method calculate seconds until alarm based on what user toggled
    calcTime2Alarm();
    alarm_exist = true;
    pixelsOff();
    return true;
  }
  else if (x>=390 && y >= 270 && y <= 312) {
    Serial.print("alarm cleared");
    alarm_exist = false;
    pixelsOff();
    return true;
  }
  return false;
}

//Couldnt work out how to convert char* to string so wrote this method to do that
//INPUTS: - a -> an array of chars that = day of week
//RETURNS: - Same day but as a string
String AlarmUIElement::convertToString(char* a) {
    int i;
    string s = "";
    for (i = 0; i < 3; i++) {
      s = s + a[i];
    }
    string TwoLetters = s.substr(0, 2);

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

//Given a day as input returns the next day
String AlarmUIElement::getNextDay(String a_day){
    if (a_day == "Monday") return "Tuesday";
    else if (a_day == "Tuesday")  return "Wednesday";
    else if (a_day == "Wednesday") return "Thursday";
    else if (a_day == "Thursday") return "Friday";
    else if (a_day == "Friday") return "Saturday";
    else if (a_day == "Saturday") return "Sunday";
    else return "Monday";
}

int AlarmUIElement::convertDayToNum(String a_day){
  if (a_day == "Monday") return 0;
  else if (a_day == "Tuesday")  return 1;
  else if (a_day == "Wednesday") return 2;
  else if (a_day == "Thursday") return 3;
  else if (a_day == "Friday") return 4;
  else if (a_day == "Saturday") return 5;
  else return 6;
}

//Used in confirm() method
//Given a time that the user has toggled on the page, calculates the time to that
//alarm in seconds and sets the alarm_time, dawn_time variables.
void AlarmUIElement::calcTime2Alarm() {
  //currentDay stores what day it is at time of setting alarm
  (strftime(date1_str_day, sizeof(date1_str_day), "%A", timeinfo));
  String currentDay = convertToString(date1_str_day);
  int counter = 0;
  //stores what the time is right now in hours and minutes
  int tmHour = timeinfo->tm_hour;
  int tmMin = timeinfo->tm_min;
  int addedSeconds; //this will eventually store total time to alarm in seconds

  //the_day is day toggled by user
  //hours is hours toggled by user, mins is mins toggled by user..
  if (the_day == currentDay && hours >= tmHour && mins >= tmMin) { //if alarm today and both hours and mins are > timenow ...
    Serial.print("1");
    addedSeconds = (((hours-tmHour)*60*60) + ((mins-tmMin))*60);

  } else if (the_day == currentDay && hours >= tmHour && mins < tmMin) {
    addedSeconds = ((hours-tmHour-1)*60*60) + ((60-tmMin+mins)*60);

  } else { //if alarm is for today and time < time now, then must be set for 7 days from now
    if (the_day == currentDay) {
      counter = 7;
    }
    while(the_day!=currentDay) { //if alarm not for today then iterate until we find day
      currentDay = getNextDay(currentDay);
      counter ++; //amount of days away...
    }

    if (hours >= tmHour && mins >= tmMin) { // hours and mins toggled are more than current hours and mins .. ect.ect
      Serial.print("3");
      //the actual calculation to determine time in seconds
      addedSeconds = (counter*24*60*60 ) +  ((hours-tmHour)*60*60) + ((mins-tmMin)*60);

    } else if (hours >= tmHour && mins < tmMin) {
      Serial.print("4");

      addedSeconds = (counter*24*60*60) + ((hours-tmHour-1)*60*60) + (((60-(tmMin-mins)))*60);

    } else if (hours < tmHour && mins >= tmMin) {
      Serial.print("5");
      addedSeconds = ((counter-1)*24*60*60) + ((24-(tmHour-hours))*60*60) + ((mins-tmMin) *60);

    } else  { //(hours < tmHour && mins < tmMin )
      Serial.print("6");
      addedSeconds = (counter-1)*24*60*60 + ((23-(tmHour-hours))*60*60) + (60-(tmMin-mins)) *60;
    }
  }

  if (testingCalc) {
    testTimeCalc();
  }

  alarm_time = time_t(time_now) + addedSeconds; //set global alarm_time variable
  dawn_time = alarm_time - 240; //dawn time is 4 minutes from alarm time - set global
  alarmTime = localtime(&alarm_time);
}

//Increments days, hours and mins
void AlarmUIElement::changeTimeUR(long x, long y) {
  for (int i=0; i<3; i++ ){ // cycles through vector for each arrow (two up, one right)
    //i = 0 -> arrow above hours, i = 1 -> arrow above mins, i = 2 -> arrow right of days
    if ((x >= x_u_r[i]) && (x <=  x_u_r[i] + 50)) { //is touch within arrow?
      if ((y <= y_u_r[i]) && (y >=  y_u_r[i] - 45)) {//....^
        //register touch
        if (i ==0) { //if touch registerd and i == 0 then this is arrow above hours
          clearHours();
          if (hours == 23) hours = 0; //start at begginning
          else hours++;
        }
        if (i ==1)  { //if 1 then arrow above mins
          clearMins();
          if (mins == 59) mins = 0;
          else mins++;
        }
        if (i ==2) { // if 2 then right arrow (days)
          clearDay();
          if (dayAsNum ==6) {
            dayAsNum = 0;
            the_day = days[dayAsNum];
          }
          else {
            dayAsNum++;
            the_day = days[dayAsNum];
          }
        }
      }
    }
  }
}

//Decrements days, mins, hours
void AlarmUIElement::changeTimeDL(long x, long y) {
  for (int i=0; i<3; i++ ){ //cycles through vector for each arrow (two down, one left)
    //i = 0 -> arrow below hours, i = 1 -> arrow below mins, i = 2 -> arrow left of days
    if ((x <= x_d_l[i]) && (x >=  x_d_l[i] - 50)) { //is touch within arrow?
      if ((y >= y_d_l[i] - 10) && (y <=  y_d_l[i] + 25)) { //...^
        //register touch
        if (i ==0) { //if touch registerd and i == 0 then this is arrow below hours..
          clearHours(); //so decrement hour
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
          if (dayAsNum ==0) {
            dayAsNum = 6;
            the_day = days[dayAsNum];
          }
          else {
            dayAsNum--;
            the_day = days[dayAsNum];
          }
        }

      }
    }

  }
}

//Draws the actual up arrow
void AlarmUIElement::drawUpArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_u_r.push_back(xOrigin - 25); //x_u_r[X,X,_] - fills vector with x positions of 2 up arrows
  y_u_r.push_back(yOrigin + 12.5); // .... ^ but y positions

  m_tft->fillTriangle(
    xOrigin - 25,    yOrigin + 12.5,
    xOrigin,         yOrigin - 12.5,
    xOrigin + 25,    yOrigin   + 12.5,
    ORANGE
  );
}

void AlarmUIElement::drawDownArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_d_l.push_back(xOrigin + 25); //x_d_l[X,X,_] -  fills vectors with x positions of 2 down arrows
  y_d_l.push_back(yOrigin - 12.5); // ...^ but y positions

  m_tft->fillTriangle(
    xOrigin + 25,    yOrigin - 12.5,
    xOrigin,         yOrigin + 12.5,
    xOrigin - 25,    yOrigin - 12.5,
    ORANGE
  );
}

void AlarmUIElement::drawRightArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_u_r.push_back(xOrigin - 12.5); // x_u_r[_,_,X] - changes x to x position of right arrow
  y_u_r.push_back(yOrigin + 25);// ...^ but with y position

  m_tft->fillTriangle(
    xOrigin - 12.5,   yOrigin + 25,
    xOrigin + 12.5,   yOrigin,
    xOrigin - 12.5,   yOrigin - 25,
    ORANGE
  );
}

void AlarmUIElement::drawLeftArrow(uint16_t xOrigin, uint16_t yOrigin) {
  x_d_l.push_back(xOrigin + 12.5); //x_d_l[_,_,X] -changes x to x position of left arrow
  y_d_l.push_back(yOrigin - 25); //...^ but with y position

  m_tft->fillTriangle(
    xOrigin + 12.5,   yOrigin - 25,
    xOrigin - 12.5,   yOrigin,
    xOrigin + 12.5,   yOrigin + 25,
    ORANGE
  );
}

//TESTING///////////////////////////////////////////////////////////////////////
void AlarmUIElement::testTimeCalc(){
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("The test time now is: Wednesday, 5:20");

  //SAME DAY
  Serial.println("Test 1");
  Serial.println("Logical case 1: Same day, hours >, mins >");
  Serial.println("Alarm time set for: Wednesday, 6:21");
  testing("Wednesday", 6, 21);
  Serial.println("");
  Serial.println("");

  Serial.println("Test 2");
  Serial.println("Logical case 2: Same day, hours >, mins <");
  Serial.println("Alarm time set for: Wednesday, 6:19");
  testing("Wednesday", 6, 19);
  Serial.println("");
  Serial.println("");

  Serial.println("Test 3");
  Serial.println("Logical case 3: Same day, hours <, mins >");
  Serial.println("Alarm time set for: Wednesday, 4:21");
  testing("Wednesday", 4, 21);
  Serial.println("");
  Serial.println("");

  Serial.println("Test 4");
  Serial.println("Logical case 4: Same day, hours <, mins <");
  Serial.println("Alarm time set for: Wednesday, 4:19");
  testing("Wednesday", 4, 19);
  Serial.println("");
  Serial.println("");

  //DIFFERENT DAY
  Serial.println("Test 5");
  Serial.println("Logical case 1: Different day, hours >, mins >");
  Serial.println("Alarm time set for: Friday, 6:21");
  testing("Friday", 6, 21);
  Serial.println("");
  Serial.println("");

  Serial.println("Test 6");
  Serial.println("Logical case 2: Different day, hours >, mins <");
  Serial.println("Alarm time set for: Saturday, 6:19");
  testing("Saturday", 6 , 19);
  Serial.println("");
  Serial.println("");

  Serial.println("Test 7");
  Serial.println("Logical case 3: Different day, hours <, mins >");
  Serial.println("Alarm time set for: Monday, 4:21");
  testing("Monday", 4, 21);
  Serial.println("");
  Serial.println("");

  Serial.println("Test 8");
  Serial.println("Logical case 4: Different day, hours < , mins <");
  Serial.println("Alarm time set for: Tuesday, 4:19");
  testing("Tuesday", 4, 19);
  Serial.println("");
  Serial.println("");

  //SET FOR NOW
  Serial.println("Test 9");
  Serial.println("Logical case 1: Right now");
  Serial.println("Alarm time set for: Wednesday, 5:20");
  testing("Wednesday", 5, 20);
  Serial.println("");
  Serial.println("");

  delay(1000);
}

void AlarmUIElement::testing(String test_day, int test_hours, int test_mins){
  //Manually set time
  //Time below is - Wednesday 16th January 05:20:00 2019


  int testCounter = 0;
  //stores what the time is right now in hours and minutes
  String the_day_test = "Wednesday";
  int testTmHour = 5;
  int testTmMin = 20;
  int testAddedSeconds; //this will eventually store total time to alarm in seconds

  //the_day is day toggled by user
  //test_hours is hours toggled by user, test_test_mins is mins toggled by user..
  if (the_day_test == test_day && test_hours >= testTmHour && test_mins >= testTmMin) { //if alarm today and both hours and mins are > timenow ...
    testAddedSeconds = (((test_hours-testTmHour)*60*60) + ((test_mins-testTmMin))*60);

  } else if (the_day_test == test_day && test_hours >= testTmHour && test_mins < testTmMin) {
    testAddedSeconds = ((test_hours-testTmHour-1)*60*60) + ((60-testTmMin+test_mins)*60);

  } else { //if alarm is for today and time < time now, then must be set for 7 days from now
    if (the_day_test == test_day) {
      testCounter = 7;
    }
    while(the_day_test!=test_day) { //if alarm not for today then iterate until we find day
      test_day = getNextDay(test_day);
      testCounter ++; //amount of days away...
    }

    if (test_hours >= testTmHour && test_mins >= testTmMin) { // hours and mins toggled are more than current hours and mins .. ect.ect
      //the actual calculation to determine time in seconds
      testAddedSeconds = (testCounter*24*60*60 ) +  ((test_hours-testTmHour)*60*60) + ((test_mins-testTmMin)*60);

    } else if (test_hours >= testTmHour && test_mins < testTmMin) {

      testAddedSeconds = (testCounter*24*60*60) + ((test_hours-testTmHour-1)*60*60) + (((60-(testTmMin-test_mins)))*60);

    } else if (test_hours < testTmHour && test_mins >= testTmMin) {
      testAddedSeconds = ((testCounter-1)*24*60*60) + ((24-(testTmHour-test_hours))*60*60) + ((test_mins-testTmMin) *60);

    } else  { //(hours < tmHour && mins < tmMin )
      testAddedSeconds = (testCounter-1)*24*60*60 + ((23-(testTmHour-test_hours))*60*60) + (60-(testTmMin-test_mins)) *60;
    }
  }

  Serial.println("Seconds to Alarm:");
  Serial.println(testAddedSeconds);
}

//////////////////////////////////////////////////////////////////////////
void AlarmUIElement::runEachTurn(){
  draw();
}
