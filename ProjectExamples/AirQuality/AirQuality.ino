#include <SPI.h>                 // the SPI bus
#include <Adafruit_GFX.h>        // core graphics library
#include "Adafruit_HX8357.h"     // tft display local hacked version
#include <Wire.h>                // I²C comms on the Arduino
#include <IOExpander.h>          // unPhone's IOExpander (controlled via I²C)
#include <GP2Y1010_DustSensor.h> // Library for the Sharp dust sensor
#include <DHTesp.h>              // Library for temperature / humidity sensor
DHTesp dht;

const int GasPin = A2;
const int DustLEDPin = 32;
const int DustPin = 15;
const int dhtPin = 13;

// the LCD
#define TFT_DC   21      // this isn't on the IO expander
Adafruit_HX8357 tft = Adafruit_HX8357(IOExpander::LCD_CS, TFT_DC, IOExpander::LCD_RESET);

GP2Y1010_DustSensor dustsensor;

void setup() {
  Serial.begin(115200);
  Serial.println("Gas and Dust sensor readings");
  Wire.setClock(100000);
  Wire.begin();
  IOExpander::begin();
  delay(10);
  tft.begin(HX8357D); // start the LCD driver chip
  IOExpander::digitalWrite(IOExpander::BACKLIGHT,HIGH); // backlight on
  tft.fillScreen(HX8357_BLACK);
  tft.setTextSize(2);

  analogSetPinAttenuation(GasPin, ADC_2_5db);   // this sets ADC to 0-3.3V range
  analogSetPinAttenuation(DustPin, ADC_2_5db);
  dustsensor.begin(DustLEDPin, DustPin);
  dustsensor.setInputVolts(5);
  dustsensor.setADCbit(12);
  
  dht.setup(dhtPin, DHTesp::AM2302);
  Serial.println("DHT initiated");
}

void loop() {
  tft.fillRect(0,50,320,200, HX8357_BLACK); // clear screen ready for next display
  tft.setCursor(0,50);

  tft.println("Dust Density:");
  float dust = dustsensor.getDustDensity();
  tft.println(dust);
  Serial.println(dust);

  tft.println("Gas Reading:");
  long valr = analogRead(GasPin);
  // the 1.5V range is divided into 4096 steps, giving 0.0003662V per step
  // and the original voltage is divided by a 10K/3.3K resistor divider (3.03)
  // so the 4096 steps corespond to 0.00111V per step of original 5V
  float volts = valr*0.00111;
  tft.print(volts);
  tft.println("V");
  Serial.print(volts);
  Serial.println("V");
  
  TempAndHumidity newValues = dht.getTempAndHumidity();
  tft.println(" T:" + String(newValues.temperature) + "C H:" + String(newValues.humidity) + "%RH");

  delay(1000);
}
