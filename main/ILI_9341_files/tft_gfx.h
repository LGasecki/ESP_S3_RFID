/*
 * tft_gfx.h
 * Sterownik ILI9341 - Naglowek
 */

#ifndef TFT_GFX_H
#define TFT_GFX_H

#include <stdint.h>
#include <stdbool.h>

// Wymiary ekranu
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// Deklaracje funkcji podstawowych
void TFT_Init(void);
void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void TFT_WriteCommand(uint8_t cmd);
void TFT_WriteData(uint8_t data);


// Deklaracje funkcji graficznych
void TFT_FillScreen(uint16_t color);
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

// Deklaracje funkcji tekstowych
void TFT_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void TFT_Print(uint16_t x, uint16_t y, char *str, uint16_t color, uint16_t bg, uint8_t size);

// Kolory (BGR565) - Skorygowane dla wyswietlaczy z odwroconym R/B
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x7800  
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x7BE0  
#define TFT_MAROON      0x000F  
#define TFT_PURPLE      0x780F  
#define TFT_OLIVE       0x03EF  
#define TFT_LIGHTGREY   0xC618  
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0xF800  
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0xFFE0  
#define TFT_RED         0x001F  
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0x07FF  
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0x053F  
#define TFT_GREENYELLOW 0x2FF5  
#define TFT_PINK        0xF81F

#endif /* TFT_GFX_H */