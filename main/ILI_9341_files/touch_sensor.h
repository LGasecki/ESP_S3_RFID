#pragma once

#include <stdint.h>
#include <stdbool.h>

struct TouchEvent {
    uint16_t x;
    uint16_t y;
};

class TouchSensor {
public:
    void init();
    bool isPressed();
    bool getCoordinates(uint16_t &x_pos, uint16_t &y_pos);
    static bool isBtnPressed(uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

private:
    uint16_t readSPI(uint8_t command);
    bool getRaw(uint16_t &x, uint16_t &y);
    long mapVal(long x, long in_min, long in_max, long out_min, long out_max);
};