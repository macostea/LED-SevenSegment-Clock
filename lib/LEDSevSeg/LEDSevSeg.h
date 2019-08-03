#include "SevSeg.h"

typedef void (*CallBack)(int);

class LEDSevSeg: public SevSeg 
{
public:
    LEDSevSeg(CallBack ledOn, CallBack ledOff);
    // LEDSevSeg();
    void refreshDisplay();
    void begin(uint8_t numDigitsIn, bool resOnSegmentsIn=0, 
            bool updateWithDelaysIn=0, bool leadingZerosIn=0,
            bool disableDecPoint=0);
protected:
    void digitOn(uint8_t digitNum);
    void digitOff(uint8_t digitOff);
private:
    uint8_t m_matrixWidth;
    uint8_t m_matrixHeight;
    uint16_t digitLEDS[MAXNUMDIGITS][7][2];

    CallBack ledOn;
    CallBack ledOff;

    uint16_t XY(uint8_t x, uint8_t y);
};