// Ex08.h
// DON'T edit these files, do your coding in MyThing!

#ifndef LAB_08_H 
#define LAB_08_H

#include "WiFiClientSecure.h"

// globals and utilities for connecting to COM3505 cloud server via uos-other
WiFiClientSecure com3505Client; // the web client library class
const char *com3505Addr    = "hc-nuc5.shef.ac.uk";
const int   com3505Port    = 9194;
const char* guestSsid      = "uos-other";
const char* guestPassword  = "shefotherkey05";
bool cloudConnect();       // initialise com3505Client; true when connected
void cloudGet(String url); // do a GET on com3505Client
String cloudRead();        // read a line of response following a request
bool cloudAvailable();     // is there more to read from the response?
void cloudStop();          // shut connection on com3505Client

#endif
