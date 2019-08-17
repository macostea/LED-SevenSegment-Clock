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
uint8_t colorPos = 0;

RtcDS3231<TwoWire> Rtc(Wire);
CRGB colors[4] {CRGB::Red, CRGB::Green, CRGB::Yellow, CRGB::GreenYellow};

void ledOn(int pos) {
  leds[pos+8] = colors[colorPos];
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

  ledSevSeg.begin(4, false, false, true, false);

  delay(3000);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  randomSeed(analogRead(0));
}

int lastHour = -1;
int lastMinute = -1;

void loop() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  int currentHour = hour();
  int currentMinute = minute();

  if (currentHour != lastHour || currentMinute != lastMinute) {
    colorPos = (colorPos + 1) % 4;
  }

  int time = 0;
  time = currentHour * 100;
  time += currentMinute;

  lastHour = currentHour;
  lastMinute = currentMinute;

  ledSevSeg.setNumber(time);

  ledSevSeg.refreshDisplay();
  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND);
}
