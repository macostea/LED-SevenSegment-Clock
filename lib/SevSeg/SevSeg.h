/* SevSeg Library
 *
 * Copyright 2017 Dean Reading
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * This library allows an Arduino to easily display numbers and letters on a
 * 7-segment display without a separate 7-segment display controller.
 *
 * Direct any questions or suggestions to deanreading@hotmail.com
 * See the included readme for instructions.
 * https://github.com/DeanIsMe/SevSeg
 */

#ifndef MAXNUMDIGITS
#define MAXNUMDIGITS 8 // Can be increased, but the max number is 2^31
#endif

#ifndef SevSeg_h
#define SevSeg_h

#ifdef ARDUINO
#include "Arduino.h"
#else
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

long map(long x, long in_min, long in_max, long out_min, long out_max);

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#endif

// Use defines to link the hardware configurations to the correct numbers
#define COMMON_CATHODE 0
#define COMMON_ANODE 1
#define N_TRANSISTORS 2
#define P_TRANSISTORS 3
#define NP_COMMON_CATHODE 1
#define NP_COMMON_ANODE 0


class SevSeg
{
public:
  SevSeg();

  virtual void refreshDisplay();
  void begin(uint8_t hardwareConfig, uint8_t numDigitsIn, uint8_t digitPinsIn[],
          uint8_t segmentPinsIn[], bool resOnSegmentsIn=0, 
          bool updateWithDelaysIn=0, bool leadingZerosIn=0,
		  bool disableDecPoint=0);
  void setBrightness(int brightnessIn); // A number from 0..100

  void setNumber(long numToShow, char decPlaces=-1, bool hex=0);
  void setNumber(unsigned long numToShow, char decPlaces=-1, bool hex=0);
  void setNumber(int numToShow, char decPlaces=-1, bool hex=0);
  void setNumber(unsigned int numToShow, char decPlaces=-1, bool hex=0);
  void setNumber(char numToShow, char decPlaces=-1, bool hex=0);
  void setNumber(uint8_t numToShow, char decPlaces=-1, bool hex=0);
  void setNumber(float numToShow, char decPlaces=-1, bool hex=0);

  void setSegments(uint8_t segs[]);
  void setChars(char str[]);
  void blank(void);

protected:
  void setNewNum(long numToShow, char decPlaces, bool hex=0);
  void findDigits(long numToShow, char decPlaces, bool hex, uint8_t digits[]);
  void setDigitCodes(uint8_t nums[], char decPlaces);
  virtual void segmentOn(uint8_t segmentNum);
  virtual void segmentOff(uint8_t segmentNum);
  virtual void digitOn(uint8_t digitNum);
  virtual void digitOff(uint8_t digitNum);

  bool digitOnVal,digitOffVal,segmentOnVal,segmentOffVal;
  bool resOnSegments, updateWithDelays, leadingZeros;
  uint8_t digitPins[MAXNUMDIGITS];
  uint8_t segmentPins[8];
  uint8_t numDigits;
  uint8_t numSegments;
  uint8_t prevUpdateIdx; // The previously updated segment or digit
  uint8_t digitCodes[MAXNUMDIGITS]; // The active setting of each segment of each digit
  unsigned long prevUpdateTime; // The time (millis()) when the display was last updated
  int ledOnTime; // The time (us) to wait with LEDs on
  int waitOffTime; // The time (us) to wait with LEDs off
  bool waitOffActive; // Whether  the program is waiting with LEDs off
};

#endif //SevSeg_h
/// END ///
