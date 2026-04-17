/**
# Project: ESP_S3_RFID
#
# Author: Łukasz Gąsecki
# Description: Implements the ILI9341 TFT display driver over SPI. Provides fundamental graphics functions for drawing pixels, rectangles, and text using DMA transfers.
# 
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h" 
#include "tft_gfx.h"
#include "hardware.h"
#include "glcdfont.c"

// --- Funkcje pomocnicze SPI (ESP-IDF) ---
void TFT_WriteCommand(uint8_t cmd) {
    gpio_set_level(PIN_NUM_TFT_DC, 0); 
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_polling_transmit(spi_tft, &t); 
}

void TFT_WriteData(uint8_t data) {
    gpio_set_level(PIN_NUM_TFT_DC, 1); 
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
    spi_device_polling_transmit(spi_tft, &t);
}

// --- Funkcje Inicjalizacji ---
void TFT_Init(void) {
    gpio_set_level(PIN_NUM_TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(PIN_NUM_TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_NUM_TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(150));

    TFT_WriteCommand(0x01); // Software Reset
    vTaskDelay(pdMS_TO_TICKS(150));
    TFT_WriteCommand(0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(150));
    TFT_WriteCommand(0x3A); // Pixel Format
    TFT_WriteData(0x55);    // 16-bit
    TFT_WriteCommand(0x29); // Display ON
    vTaskDelay(pdMS_TO_TICKS(150));
    
    TFT_WriteCommand(0x36); // Memory Access Control
    TFT_WriteData(0x20);   
}

void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    TFT_WriteCommand(0x2A); // Kolumna
    TFT_WriteData(x0 >> 8);
    TFT_WriteData(x0 & 0xFF);
    TFT_WriteData(x1 >> 8);
    TFT_WriteData(x1 & 0xFF);

    TFT_WriteCommand(0x2B); // Wiersz
    TFT_WriteData(y0 >> 8);
    TFT_WriteData(y0 & 0xFF);
    TFT_WriteData(y1 >> 8);
    TFT_WriteData(y1 & 0xFF);

    TFT_WriteCommand(0x2C); // Zapis do pamieci
}

// --- Funkcje Rysowania ---
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) return;
    TFT_SetAddressWindow(x, y, x, y);
    
    uint8_t data[2] = {color >> 8, color & 0xFF};
    gpio_set_level(PIN_NUM_TFT_DC, 1);
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = data
    };
    spi_device_polling_transmit(spi_tft, &t);
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) return;
    if((x + w - 1) >= TFT_WIDTH)  w = TFT_WIDTH  - x;
    if((y + h - 1) >= TFT_HEIGHT) h = TFT_HEIGHT - y;

    TFT_SetAddressWindow(x, y, x+w-1, y+h-1);

    uint32_t size = w * h;
    const uint32_t max_pixels = 1024; 
    uint16_t color_swapped = (color >> 8) | (color << 8); 
    
    uint16_t buffer[max_pixels];
    for (int i = 0; i < max_pixels; i++) {
        buffer[i] = color_swapped;
    }

    gpio_set_level(PIN_NUM_TFT_DC, 1);
    
    uint32_t pixels_sent = 0;
    while (pixels_sent < size) {
        uint32_t pixels_to_send = (size - pixels_sent > max_pixels) ? max_pixels : (size - pixels_sent);
        spi_transaction_t t = {
            .length = pixels_to_send * 16,
            .tx_buffer = buffer
        };
        spi_device_polling_transmit(spi_tft, &t);
        pixels_sent += pixels_to_send;
    }
}

void TFT_FillScreen(uint16_t color) {
    TFT_FillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

// --- Funkcje Tekstowe ---
void TFT_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) return;
    for (int8_t i = 0; i < 5; i++) { 
        uint8_t line = font[(c * 5) + i];
        for (int8_t j = 0; j < 8; j++, line >>= 1) { 
            if (line & 1) {
                if (size == 1) TFT_DrawPixel(x + i, y + j, color);
                else TFT_FillRect(x + (i * size), y + (j * size), size, size, color);
            } else if (bg != color) { 
                if (size == 1) TFT_DrawPixel(x + i, y + j, bg);
                else TFT_FillRect(x + (i * size), y + (j * size), size, size, bg);
            }
        }
    }
}

void TFT_Print(uint16_t x, uint16_t y, char *str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        if (x + (size * 6) >= TFT_WIDTH) {
            x = 0;
            y += size * 8;
        }
        TFT_DrawChar(x, y, *str, color, bg, size);
        x += size * 6; 
        str++;
    }
}

void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    TFT_FillRect(x, y, w, 1, color);
    TFT_FillRect(x, y + h - 1, w, 1, color);
    TFT_FillRect(x, y, 1, h, color);
    TFT_FillRect(x + w - 1, y, 1, h, color);
}