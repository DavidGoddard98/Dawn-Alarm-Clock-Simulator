// Thing.ino
// COM3505 lab exercise runner sketch

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

#include "Ex01.h"
#include "Ex02.h"

int LABNUM = 1; // which lab exercise number are we doing?

// function prototypes, needed by newer Arduino IDEs (and IDF)
void setup01(); void loop01(); void setup02(); void loop02();

// initialisation entry point
void setup() {
  switch(LABNUM) {
    case  1: setup01(); break;
    case  2: setup02(); break;
    default: Serial.println("oops! invalid lab number");
  }
}

// task loop entry point
void loop() {
  switch(LABNUM) {
    case  1: loop01(); break;
    case  2: loop02(); break;
    default: Serial.println("oops! invalid lab number");
  }
}
