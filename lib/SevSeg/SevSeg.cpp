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

#include "SevSeg.h"

#define BLANK_IDX 36 // Must match with 'digitCodeMap'
#define DASH_IDX 37
#define PERIOD_IDX 38
#define ASTERISK_IDX 39

static const long powersOf10[] = {
  1, // 10^0
  10,
  100,
  1000,
  10000,
  100000,
  1000000,
  10000000,
  100000000,
  1000000000
}; // 10^9

static const long powersOf16[] = {
  0x1, // 16^0
  0x10,
  0x100,
  0x1000,
  0x10000,
  0x100000,
  0x1000000,
  0x10000000
}; // 16^7

// digitCodeMap indicate which segments must be illuminated to display
// each number.
static const uint8_t digitCodeMap[] = {
  //     GFEDCBA  Segments      7-segment map:
  0b00111111, // 0   "0"          AAA
  0b00000110, // 1   "1"         F   B
  0b01011011, // 2   "2"         F   B
  0b01001111, // 3   "3"          GGG
  0b01100110, // 4   "4"         E   C
  0b01101101, // 5   "5"         E   C
  0b01111101, // 6   "6"          DDD
  0b00000111, // 7   "7"
  0b01111111, // 8   "8"
  0b01101111, // 9   "9"
  0b01110111, // 65  'A'
  0b01111100, // 66  'b'
  0b00111001, // 67  'C'
  0b01011110, // 68  'd'
  0b01111001, // 69  'E'
  0b01110001, // 70  'F'
  0b00111101, // 71  'G'
  0b01110110, // 72  'H'
  0b00000110, // 73  'I'
  0b00001110, // 74  'J'
  0b01110110, // 75  'K'  Same as 'H'
  0b00111000, // 76  'L'
  0b00000000, // 77  'M'  NO DISPLAY
  0b01010100, // 78  'n'
  0b00111111, // 79  'O'
  0b01110011, // 80  'P'
  0b01100111, // 81  'q'
  0b01010000, // 82  'r'
  0b01101101, // 83  'S'
  0b01111000, // 84  't'
  0b00111110, // 85  'U'
  0b00111110, // 86  'V'  Same as 'U'
  0b00000000, // 87  'W'  NO DISPLAY
  0b01110110, // 88  'X'  Same as 'H'
  0b01101110, // 89  'y'
  0b01011011, // 90  'Z'  Same as '2'
  0b00000000, // 32  ' '  BLANK
  0b01000000, // 45  '-'  DASH
  0b10000000, // 46  '.'  PERIOD
  0b01100011, // 42 '*'  DEGREE ..
};

// Constant pointers to constant data
const uint8_t * const numeralCodes = digitCodeMap;
const uint8_t * const alphaCodes = digitCodeMap + 10;

// SevSeg Constructor
/******************************************************************************/
SevSeg::SevSeg() {
  // Initial value
  ledOnTime = 2000; // Corresponds to a brightness of 100
  waitOffTime = 0;
  waitOffActive = false;
  numDigits = 0;
  prevUpdateIdx = 0;
  prevUpdateTime = 0;
  resOnSegments = 0;
  updateWithDelays = 0;
}


// begin
/******************************************************************************/
// Saves the input pin numbers to the class and sets up the pins to be used.
// If you use current-limiting resistors on your segment pins instead of the
// digit pins, then set resOnSegments as true.
// Set updateWithDelays to true if you want to use the 'pre-2017' update method
// In that case, the processor is occupied with delay functions while refreshing
// leadingZerosIn indicates whether leading zeros should be displayed
// disableDecPoint is true when the decimal point segment is not connected, in
// which case there are only 7 segments.
void SevSeg::begin(uint8_t hardwareConfig, uint8_t numDigitsIn, uint8_t digitPinsIn[],
                   uint8_t segmentPinsIn[], bool resOnSegmentsIn,
                   bool updateWithDelaysIn, bool leadingZerosIn, bool disableDecPoint) {

  resOnSegments = resOnSegmentsIn;
  updateWithDelays = updateWithDelaysIn;
  leadingZeros = leadingZerosIn;

  numDigits = numDigitsIn;
  numSegments = disableDecPoint ? 7 : 8; // Ternary 'if' statement
  //Limit the max number of digits to prevent overflowing
  if (numDigits > MAXNUMDIGITS) numDigits = MAXNUMDIGITS;

  switch (hardwareConfig) {

    case 0: // Common cathode
      digitOnVal = 0;
      segmentOnVal = 1;
      break;

    case 1: // Common anode
      digitOnVal = 1;
      segmentOnVal = 0;
      break;

    case 2: // With active-high, low-side switches (most commonly N-type FETs)
      digitOnVal = 1;
      segmentOnVal = 1;
      break;

    case 3: // With active low, high side switches (most commonly P-type FETs)
      digitOnVal = 0;
      segmentOnVal = 0;
      break;
  }

  digitOffVal = !digitOnVal;
  segmentOffVal = !segmentOnVal;

  // Save the input pin numbers to library variables
  for (uint8_t segmentNum = 0 ; segmentNum < numSegments ; segmentNum++) {
    segmentPins[segmentNum] = segmentPinsIn[segmentNum];
  }

  for (uint8_t digitNum = 0 ; digitNum < numDigits ; digitNum++) {
    digitPins[digitNum] = digitPinsIn[digitNum];
  }

  // Set the pins as outputs, and turn them off
  for (uint8_t digit = 0 ; digit < numDigits ; digit++) {
#ifdef ARDUINO
    pinMode(digitPins[digit], OUTPUT);
    digitalWrite(digitPins[digit], digitOffVal);
#endif
  }

  for (uint8_t segmentNum = 0 ; segmentNum < numSegments ; segmentNum++) {
#ifdef ARDUINO
    pinMode(segmentPins[segmentNum], OUTPUT);
    digitalWrite(segmentPins[segmentNum], segmentOffVal);
#endif
  }

  blank(); // Initialise the display
}


// refreshDisplay
/******************************************************************************/
// Turns on the segments specified in 'digitCodes[]'
// There are 4 versions of this function, with the choice depending on the
// location of the current-limiting resistors, and whether or not you wish to
// use 'update delays' (the standard method until 2017).
// For resistors on *digits* we will cycle through all 8 segments (7 + period),
//    turning on the *digits* as appropriate for a given segment, before moving on
//    to the next segment.
// For resistors on *segments* we will cycle through all __ # of digits,
//    turning on the *segments* as appropriate for a given digit, before moving on
//    to the next digit.
// If using update delays, refreshDisplay has a delay between each digit/segment
//    as it cycles through. It exits with all LEDs off.
// If not using updateDelays, refreshDisplay exits with a single digit/segment
//    on. It will move to the next digit/segment after being called again (if
//    enough time has passed).

void SevSeg::refreshDisplay() {

  if (!updateWithDelays) {
#ifdef ARDUINO
    unsigned long us = micros();
#else
    unsigned long us = time(NULL);
#endif

    // Exit if it's not time for the next display change
    if (waitOffActive) {
      if (us - prevUpdateTime < waitOffTime) return;
    }
    else {
      if (us - prevUpdateTime < ledOnTime) return;
    }
    prevUpdateTime = us;

    if (!resOnSegments) {
      /**********************************************/
      // RESISTORS ON DIGITS, UPDATE WITHOUT DELAYS

      if (waitOffActive) {
        waitOffActive = false;
      }
      else {
        // Turn all lights off for the previous segment
        segmentOff(prevUpdateIdx);

        if (waitOffTime) {
          // Wait a delay with all lights off
          waitOffActive = true;
          return;
        }
      }

      prevUpdateIdx++;
      if (prevUpdateIdx >= numSegments) prevUpdateIdx = 0;

      // Illuminate the required digits for the new segment
      segmentOn(prevUpdateIdx);
    }
    else {
      /**********************************************/
      // RESISTORS ON SEGMENTS, UPDATE WITHOUT DELAYS

      if (waitOffActive) {
        waitOffActive = false;
      }
      else {
        // Turn all lights off for the previous digit
        digitOff(prevUpdateIdx);

        if (waitOffTime) {
          // Wait a delay with all lights off
          waitOffActive = true;
          return;
        }
      }

      prevUpdateIdx++;
      if (prevUpdateIdx >= numDigits) prevUpdateIdx = 0;

      // Illuminate the required segments for the new digit
      digitOn(prevUpdateIdx);
    }
  }

  else {
    if (!resOnSegments) {
      /**********************************************/
      // RESISTORS ON DIGITS, UPDATE WITH DELAYS
      for (uint8_t segmentNum = 0 ; segmentNum < numSegments ; segmentNum++) {

        // Illuminate the required digits for this segment
        segmentOn(segmentNum);

        // Wait with lights on (to increase brightness)
#ifdef ARDUINO
        delayMicroseconds(ledOnTime);
#else
        sleep(ledOnTime/1000000);
#endif

        // Turn all lights off
        segmentOff(segmentNum);

        // Wait with all lights off if required
        if (waitOffTime) {
#ifdef ARDUINO
          delayMicroseconds(waitOffTime);
#else
          sleep(waitOffTime/1000000);
#endif
        }
      }
    }
    else {
      /**********************************************/
      // RESISTORS ON SEGMENTS, UPDATE WITH DELAYS
      for (uint8_t digitNum = 0 ; digitNum < numDigits ; digitNum++) {

        // Illuminate the required segments for this digit
        digitOn(digitNum);

        // Wait with lights on (to increase brightness)
#ifdef ARDUINO
        delayMicroseconds(ledOnTime);
#else
        sleep(ledOnTime/1000000);
#endif

        // Turn all lights off
        digitOff(digitNum);

        // Wait with all lights off if required
        if (waitOffTime) {
#ifdef ARDUINO
          delayMicroseconds(waitOffTime);
#else
          sleep(waitOffTime/1000000);
#endif
        }
      }
    }
  }
}

// segmentOn
/******************************************************************************/
// Turns a segment on, as well as all corresponding digit pins
// (according to digitCodes[])
void SevSeg::segmentOn(uint8_t segmentNum) {
#ifdef ARDUINO
  digitalWrite(segmentPins[segmentNum], segmentOnVal);
  for (uint8_t digitNum = 0 ; digitNum < numDigits ; digitNum++) {
    if (digitCodes[digitNum] & (1 << segmentNum)) { // Check a single bit
      digitalWrite(digitPins[digitNum], digitOnVal);
    }
  }
#endif
}

// segmentOff
/******************************************************************************/
// Turns a segment off, as well as all digit pins
void SevSeg::segmentOff(uint8_t segmentNum) {
#ifdef ARDUINO
  for (uint8_t digitNum = 0 ; digitNum < numDigits ; digitNum++) {
    digitalWrite(digitPins[digitNum], digitOffVal);
  }
  digitalWrite(segmentPins[segmentNum], segmentOffVal);
#endif
}

// digitOn
/******************************************************************************/
// Turns a digit on, as well as all corresponding segment pins
// (according to digitCodes[])
void SevSeg::digitOn(uint8_t digitNum) {
#ifdef ARDUINO
  digitalWrite(digitPins[digitNum], digitOnVal);
  for (uint8_t segmentNum = 0 ; segmentNum < numSegments ; segmentNum++) {
    if (digitCodes[digitNum] & (1 << segmentNum)) { // Check a single bit
      digitalWrite(segmentPins[segmentNum], segmentOnVal);
    }
  }
#endif
}

// digitOff
/******************************************************************************/
// Turns a digit off, as well as all segment pins
void SevSeg::digitOff(uint8_t digitNum) {
#ifdef ARDUINO
  for (uint8_t segmentNum = 0 ; segmentNum < numSegments ; segmentNum++) {
    digitalWrite(segmentPins[segmentNum], segmentOffVal);
  }
  digitalWrite(digitPins[digitNum], digitOffVal);
#endif
}

// setBrightness
/******************************************************************************/
// Sets ledOnTime according to the brightness given. Standard brightness range
// is 0 to 100. Flickering is more likely at brightness > 100, and < -100.
// A positive brightness introduces a delay while the LEDs are on, and a
// negative brightness introduces a delay while the LEDs are off.
void SevSeg::setBrightness(int brightness) {
  brightness = constrain(brightness, -200, 200);
  if (brightness > 0) {
    ledOnTime = map(brightness, 0, 100, 1, 2000);
    waitOffTime = 0;
    waitOffActive = false;
  }
  else {
    ledOnTime = 0;
    waitOffTime = map(brightness, 0, -100, 1, 2000);
  }
}


// setNumber
/******************************************************************************/
// This function only receives the input and passes it to 'setNewNum'.
// It is overloaded for all number data types, so that floats can be handled
// correctly.
void SevSeg::setNumber(long numToShow, char decPlaces, bool hex) { //long
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(unsigned long numToShow, char decPlaces, bool hex) { //unsigned long
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(int numToShow, char decPlaces, bool hex) { //int
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(unsigned int numToShow, char decPlaces, bool hex) { //unsigned int
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(char numToShow, char decPlaces, bool hex) { //char
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(uint8_t numToShow, char decPlaces, bool hex) { //uint8_t
  setNewNum(numToShow, decPlaces, hex);
}

void SevSeg::setNumber(float numToShow, char decPlaces, bool hex) { //float
  char decPlacesPos = constrain(decPlaces, 0, MAXNUMDIGITS);
  if (hex) {
    numToShow = numToShow * powersOf16[decPlacesPos];
  }
  else {
    numToShow = numToShow * powersOf10[decPlacesPos];
  }
  // Modify the number so that it is rounded to an integer correctly
  numToShow += (numToShow >= 0) ? 0.5f : -0.5f;
  setNewNum(numToShow, decPlaces, hex);
}


// setNewNum
/******************************************************************************/
// Changes the number that will be displayed.
void SevSeg::setNewNum(long numToShow, char decPlaces, bool hex) {
  uint8_t digits[numDigits];
  findDigits(numToShow, decPlaces, hex, digits);
  setDigitCodes(digits, decPlaces);
}


// setSegments
/******************************************************************************/
// Sets the 'digitCodes' that are required to display the desired segments.
// Using this function, one can display any arbitrary set of segments (like
// letters, symbols or animated cursors). See setDigitCodes() for common
// numeric examples.
//
// Bit-segment mapping:  0bHGFEDCBA
//      Visual mapping:
//                        AAAA          0000
//                       F    B        5    1
//                       F    B        5    1
//                        GGGG          6666
//                       E    C        4    2
//                       E    C        4    2        (Segment H is often called
//                        DDDD  H       3333  7      DP, for Decimal Point)
void SevSeg::setSegments(uint8_t segs[]) {
  for (uint8_t digit = 0; digit < numDigits; digit++) {
    digitCodes[digit] = segs[digit];
  }
}

// setChars
/******************************************************************************/
// Displays the string on the display, as best as possible.
// Only alphanumeric characters plus '-' and ' ' are supported
void SevSeg::setChars(char str[]) {
  for (uint8_t digit = 0; digit < numDigits; digit++) {
    digitCodes[digit] = 0;
  }

  uint8_t strIdx = 0; // Current position within str[]
  for (uint8_t digitNum = 0; digitNum < numDigits; digitNum++) {
    char ch = str[strIdx];
    if (ch == '\0') break; // NULL string terminator
    if (ch >= '0' && ch <= '9') { // Numerical
      digitCodes[digitNum] = numeralCodes[ch - '0'];
    }
    else if (ch >= 'A' && ch <= 'Z') {
      digitCodes[digitNum] = alphaCodes[ch - 'A'];
    }
    else if (ch >= 'a' && ch <= 'z') {
      digitCodes[digitNum] = alphaCodes[ch - 'a'];
    }
    else if (ch == ' ') {
      digitCodes[digitNum] = digitCodeMap[BLANK_IDX];
    }
    else if (ch == '.') {
      digitCodes[digitNum] = digitCodeMap[PERIOD_IDX];
    }
    else if (ch == '*') {
      digitCodes[digitNum] = digitCodeMap[ASTERISK_IDX];
    }
    else {
      // Every unknown character is shown as a dash
      digitCodes[digitNum] = digitCodeMap[DASH_IDX];
    }

    strIdx++;
    // Peek at next character. It it's a period, add it to this digit
    if (str[strIdx] == '.') {
      digitCodes[digitNum] |= digitCodeMap[PERIOD_IDX];
      strIdx++;
    }
  }
}

// blank
/******************************************************************************/
void SevSeg::blank(void) {
  for (uint8_t digitNum = 0 ; digitNum < numDigits ; digitNum++) {
    digitCodes[digitNum] = digitCodeMap[BLANK_IDX];
  }
  segmentOff(0);
  digitOff(0);
}

// findDigits
/******************************************************************************/
// Decides what each digit will display.
// Enforces the upper and lower limits on the number to be displayed.

void SevSeg::findDigits(long numToShow, char decPlaces, bool hex, uint8_t digits[]) {
  const long * powersOfBase = hex ? powersOf16 : powersOf10;
  const long maxNum = powersOfBase[numDigits] - 1;
  const long minNum = -(powersOfBase[numDigits - 1] - 1);

  // If the number is out of range, just display dashes
  if (numToShow > maxNum || numToShow < minNum) {
    for (uint8_t digitNum = 0 ; digitNum < numDigits ; digitNum++) {
      digits[digitNum] = DASH_IDX;
    }
  }
  else {
    uint8_t digitNum = 0;

    // Convert all number to positive values
    if (numToShow < 0) {
      digits[0] = DASH_IDX;
      digitNum = 1; // Skip the first iteration
      numToShow = -numToShow;
    }

    // Find all digits for base's representation, starting with the most
    // significant digit
    for ( ; digitNum < numDigits ; digitNum++) {
      long factor = powersOfBase[numDigits - 1 - digitNum];
      digits[digitNum] = numToShow / factor;
      numToShow -= digits[digitNum] * factor;
    }

    // Find unnnecessary leading zeros and set them to BLANK
    if (decPlaces < 0) decPlaces = 0;
    if (!leadingZeros) {
      for (digitNum = 0 ; digitNum < (numDigits - 1 - decPlaces) ; digitNum++) {
        if (digits[digitNum] == 0) {
          digits[digitNum] = BLANK_IDX;
        }
        // Exit once the first non-zero number is encountered
        else if (digits[digitNum] <= 9) {
          break;
        }
      }
    }

  }
}


// setDigitCodes
/******************************************************************************/
// Sets the 'digitCodes' that are required to display the input numbers

void SevSeg::setDigitCodes(uint8_t digits[], char decPlaces) {

  // Set the digitCode for each digit in the display
  for (uint8_t digitNum = 0 ; digitNum < numDigits ; digitNum++) {
    digitCodes[digitNum] = digitCodeMap[digits[digitNum]];
    // Set the decimal point segment
    if (decPlaces >= 0) {
      if (digitNum == numDigits - 1 - decPlaces) {
        digitCodes[digitNum] |= digitCodeMap[PERIOD_IDX];
      }
    }
  }
}

#ifndef ARDUINO
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif

/// END ///
