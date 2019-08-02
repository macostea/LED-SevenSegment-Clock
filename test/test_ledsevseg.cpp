#include <unity.h>
#include <LEDSevSeg.h>

int main() {
    LEDSevSeg ledSevSeg;

    ledSevSeg.begin(4);
    ledSevSeg.setNumber(1234);

    ledSevSeg.refreshDisplay();
    
    return 0;
}