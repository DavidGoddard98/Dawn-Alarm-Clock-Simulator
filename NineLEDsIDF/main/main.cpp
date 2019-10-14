// NineLEDsIDF main.cpp

#include "Arduino.h" // common definitions

/////////////////////////////////////////////////////////////////////////////
// constants etc.

// Serial speed /////////////////////////////////////////////////////////////
#define SERIAL_SPEED 115200

// MAC address //////////////////////////////////////////////////////////////
char *getMAC(char *buf); // get the MAC address
extern char MAC_ADDRESS[13]; // MACs are 12 chars, plus the NULL terminator

// OTA support //////////////////////////////////////////////////////////////
const char *myName = "NineLEDsIDF";

// LEDs, ordered accoding to the 9 LEDs board layout
uint8_t ledPins[] = {
  GPIO_NUM_26,
  GPIO_NUM_25,
  GPIO_NUM_21,
  GPIO_NUM_4,
  GPIO_NUM_27,
  GPIO_NUM_33,
  GPIO_NUM_15,
  GPIO_NUM_32,
  GPIO_NUM_14,
};
uint8_t numLedPins = sizeof(ledPins) / sizeof(uint8_t);

// blink internal LED
void blinkFeatherLED(uint8_t times);

/////////////////////////////////////////////////////////////////////////////
// setup ////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.printf("Serial initialised at %d\n", SERIAL_SPEED);
  esp_log_level_set("*", ESP_LOG_ERROR); // IDF logging

  pinMode(BUILTIN_LED, OUTPUT);          // turn built-in LED on

  for(int i = 0; i < numLedPins; i++) {
    pinMode(ledPins[i], OUTPUT);         // set up the external LED pins
    digitalWrite(ledPins[i], LOW);       // off to begin
  }

  delay(300);
  blinkFeatherLED(5);                    // signal we've finished config
  delay(500);
}

/////////////////////////////////////////////////////////////////////////////
// looooooooooooooooooooop //////////////////////////////////////////////////
uint32_t loopIndex = 0;
void loop() {
  Serial.printf("starting loop %2d\n", loopIndex++);

  for(int i = 0; i < numLedPins; i++) {
    digitalWrite(ledPins[i], HIGH);
    delay(300);
    digitalWrite(ledPins[i], LOW);
    delay(300);
  }
}

/////////////////////////////////////////////////////////////////////////////
// misc utils ///////////////////////////////////////////////////////////////

void blinkFeatherLED(uint8_t times) {
  for(int i=0; i<times; i++) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(300);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(300);
  }
}

char MAC_ADDRESS[13]; // MAC addresses are 12 chars, plus the NULL terminator
char *getMAC(char *buf) { // the MAC is 6 bytes, so needs careful conversion..
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
  return buf;
}
