// Ex09.h

#ifndef LAB_09_H 
#define LAB_09_H

void setup09();
void loop09();
void initWebServer();
void hndlNotFound();
void hndlRoot();
void hndlWifi();
void hndlWifichz();
void hndlStatus();
void apListForm(String& f);
String ip2str(IPAddress address);

#endif
