#include "LEDSevSeg.h"

LEDSevSeg::LEDSevSeg() : SevSeg() {
}

void LEDSevSeg::refreshDisplay() {
    for (uint8_t digitNum = 0; digitNum < numDigits; digitNum++) {
        digitOn(digitNum);

        #ifdef ARDUINO
        delayMicroseconds(ledOnTime);
#else
        sleep(ledOnTime/1000000);
#endif

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
void LEDSevSeg::begin(uint8_t numDigitsIn, bool resOnSegmentsIn, 
        bool updateWithDelaysIn, bool leadingZerosIn,
        bool disableDecPoint) {
    this->numDigits = numDigitsIn;

    this->m_matrixWidth = this->numDigits * 2;
    this->m_matrixHeight = 7;

    this->numSegments = 7;
    this->resOnSegments = resOnSegmentsIn;
    this->updateWithDelays = updateWithDelaysIn;
    this->leadingZeros = leadingZerosIn;
    if (numDigits > MAXNUMDIGITS) numDigits = MAXNUMDIGITS;

    this->segmentOnVal = 1;
    this->segmentOffVal = !this->segmentOnVal;

    for (uint8_t digitNum = 0; digitNum < this->numDigits; digitNum++) {
        for (uint8_t segmentNum = 0; segmentNum < this->numSegments; segmentNum++) {
            switch (segmentNum) {
            case 0:
                digitLEDS[digitNum][segmentNum][0] = this->XY(2 * digitNum, 0);
                digitLEDS[digitNum][segmentNum][1] = this->XY(2 * digitNum + 1, 0);
                break;
            case 1:
                digitLEDS[digitNum][segmentNum][0] = this->XY(2 * digitNum + 1, 1);
                digitLEDS[digitNum][segmentNum][1] = this->XY(2 * digitNum + 1, 2);
                break;
            case 2:
                digitLEDS[digitNum][segmentNum][0] = this->XY(2 * digitNum + 1, 4);
                digitLEDS[digitNum][segmentNum][1] = this->XY(2 * digitNum + 1, 5);
                break;
            case 3:
                digitLEDS[digitNum][segmentNum][0] = this->XY(2 * digitNum, 6);
                digitLEDS[digitNum][segmentNum][1] = this->XY(2 * digitNum + 1, 6);
                break;
            case 4:
                digitLEDS[digitNum][segmentNum][0] = this->XY(2 * digitNum, 4);
                digitLEDS[digitNum][segmentNum][1] = this->XY(2 * digitNum, 5);
                break;
            case 5:
                digitLEDS[digitNum][segmentNum][0] = this->XY(2 * digitNum, 1);
                digitLEDS[digitNum][segmentNum][1] = this->XY(2 * digitNum, 2);
                break;
            case 6:
                digitLEDS[digitNum][segmentNum][0] = this->XY(2 * digitNum, 3);
                digitLEDS[digitNum][segmentNum][1] = this->XY(2 * digitNum + 1, 3);
                break;
            }
        }
    }
}

void LEDSevSeg::digitOn(uint8_t digitNum) {
    printf("Digit num: %d\n", digitNum);
    printf("Digit code: %d\n", digitCodes[digitNum]);

    for (uint8_t segmentNum = 0; segmentNum < numSegments; segmentNum++) {
        if (digitCodes[digitNum] & (1 << segmentNum)) {
            printf("%d, %d\n", digitLEDS[digitNum][segmentNum][0], digitLEDS[digitNum][segmentNum][1]);
            // TODO: ADD FastLED turn on
        }
    }
}

void LEDSevSeg::digitOff(uint8_t digitOff) {
    for (uint8_t segmentNum = 0; segmentNum < numSegments; segmentNum++) {
        // TODO: Add FastLED turn off
    }
}

const bool kMatrixSerpentineLayout = true;

uint16_t LEDSevSeg::XY( uint8_t x, uint8_t y) {
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * m_matrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (m_matrixWidth - 1) - x;
      i = (y * m_matrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * m_matrixWidth) + x;
    }
  }
  
  return i;
}