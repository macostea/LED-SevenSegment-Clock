#ifdef ARDUINO
#include <Arduino.h>
#include <FastLED.h>
#include <TimeLib.h>
#include <Wire.h>
#include <RtcDS3231.h>
#endif
#include <LEDSevSeg.h>

#define DATA_PIN 5
#define LED_TYPE WS2812B
#define COLOR_ORDER RGB
#define NUM_LEDS 64

#define BRIGHTNESS 96
#define FRAMES_PER_SECOND 120

CRGB leds[NUM_LEDS];
uint8_t gHue = 0;

RtcDS3231<TwoWire> Rtc(Wire);

void ledOn(int pos) {
  leds[pos+8] = CHSV(gHue, 255, 192);
}

void ledOff(int pos) {
  leds[pos+8] = CRGB::Black;
}

LEDSevSeg ledSevSeg(&ledOn, &ledOff);


time_t getTime() {
  RtcDateTime now = Rtc.GetDateTime();
  return now.Epoch32Time();
}

void setup() {
  Serial.begin(9600);
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    Rtc.SetDateTime(compiled);
  }

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  setSyncProvider(getTime);

  ledSevSeg.begin(4);

  delay(3000);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  randomSeed(analogRead(0));
}

void loop() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  int time = 0;
  time = hour() * 100;
  time += minute();

  ledSevSeg.setNumber(time);

  ledSevSeg.refreshDisplay();
  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND);

  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}

