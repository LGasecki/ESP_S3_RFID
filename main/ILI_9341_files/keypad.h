#pragma once

#include <stdint.h>
#include "tft_gfx.h"
#include "touch_sensor.h"

class Keypad {
public:
    Keypad(TFT_GFX& display, TouchSensor& touch);
    
    void draw();
    void updatePinDisplay();
    bool handleLoginTouch(uint16_t tx, uint16_t ty);

private:
    TFT_GFX& tft;
    TouchSensor& touchSensor;
    
    char pinBuffer[5];
    const char* USER_PIN;
};