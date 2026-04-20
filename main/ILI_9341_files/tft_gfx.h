/**
# Project: ESP_S3_RFID
#
# Author: Łukasz Gąsecki
# Description: Provides the XPT2046 touch controller driver. Handles SPI communication, raw data sampling, noise filtering, and mapping physical touch inputs to screen coordinates.
# 
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Kolory (BGR565)
#define TFT_BLACK       0x0000
#define TFT_WHITE       0xFFFF
#define TFT_RED         0x001F  
#define TFT_GREEN       0x07E0
#define TFT_BLUE        0xF800  
#define TFT_YELLOW      0x07FF  
#define TFT_DARKCYAN    0x7BE0  

class TFT_GFX {
public:
    static const uint16_t WIDTH = 320;
    static const uint16_t HEIGHT = 240;

    void init();
    
    void fillScreen(uint16_t color);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void print(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);

private:
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
};