/**
# Project: ESP_S3_RFID
#
# Author: Łukasz Gąsecki
# Description: Implements the ILI9341 TFT display driver over SPI. Provides fundamental graphics functions for drawing pixels, rectangles, and text using DMA transfers.
# 
*/

#include "tft_gfx.h"
#include "hardware.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "glcdfont.c"

void TFT_GFX::writeCommand(uint8_t cmd) {
    gpio_set_level((gpio_num_t)PIN_NUM_TFT_DC, 0); 
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &cmd;
    spi_device_polling_transmit(Hardware::spi_tft, &t); 
}

void TFT_GFX::writeData(uint8_t data) {
    gpio_set_level((gpio_num_t)PIN_NUM_TFT_DC, 1); 
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &data;
    spi_device_polling_transmit(Hardware::spi_tft, &t);
}

void TFT_GFX::init() {
    gpio_set_level((gpio_num_t)PIN_NUM_TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level((gpio_num_t)PIN_NUM_TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level((gpio_num_t)PIN_NUM_TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(150));

    writeCommand(0x01); // Software Reset
    vTaskDelay(pdMS_TO_TICKS(150));
    writeCommand(0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(150));
    writeCommand(0x3A); // Pixel Format
    writeData(0x55);    // 16-bit
    writeCommand(0x29); // Display ON
    vTaskDelay(pdMS_TO_TICKS(150));
    
    writeCommand(0x36); // Memory Access Control
    writeData(0x20);   
}

void TFT_GFX::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(0x2A); 
    writeData(x0 >> 8); writeData(x0 & 0xFF);
    writeData(x1 >> 8); writeData(x1 & 0xFF);

    writeCommand(0x2B); 
    writeData(y0 >> 8); writeData(y0 & 0xFF);
    writeData(y1 >> 8); writeData(y1 & 0xFF);

    writeCommand(0x2C); 
}

void TFT_GFX::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= WIDTH) || (y >= HEIGHT)) return;
    setAddressWindow(x, y, x, y);
    
    uint8_t data[2] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };
    gpio_set_level((gpio_num_t)PIN_NUM_TFT_DC, 1);
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_buffer = data;
    spi_device_polling_transmit(Hardware::spi_tft, &t);
}

void TFT_GFX::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if((x >= WIDTH) || (y >= HEIGHT)) return;
    if((x + w - 1) >= WIDTH)  w = WIDTH  - x;
    if((y + h - 1) >= HEIGHT) h = HEIGHT - y;

    setAddressWindow(x, y, x+w-1, y+h-1);

    uint32_t size = w * h;
    const uint32_t max_pixels = 1024; 
    uint16_t color_swapped = (color >> 8) | (color << 8); 
    
    uint16_t buffer[max_pixels];
    for (int i = 0; i < max_pixels; i++) {
        buffer[i] = color_swapped;
    }

    gpio_set_level((gpio_num_t)PIN_NUM_TFT_DC, 1);
    
    uint32_t pixels_sent = 0;
    while (pixels_sent < size) {
        uint32_t pixels_to_send = (size - pixels_sent > max_pixels) ? max_pixels : (size - pixels_sent);
        spi_transaction_t t = {};
        t.length = pixels_to_send * 16;
        t.tx_buffer = buffer;
        spi_device_polling_transmit(Hardware::spi_tft, &t);
        pixels_sent += pixels_to_send;
    }
}

void TFT_GFX::fillScreen(uint16_t color) {
    fillRect(0, 0, WIDTH, HEIGHT, color);
}

void TFT_GFX::drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if((x >= WIDTH) || (y >= HEIGHT)) return;
    for (int8_t i = 0; i < 5; i++) { 
        uint8_t line = font[(c * 5) + i];
        for (int8_t j = 0; j < 8; j++, line >>= 1) { 
            if (line & 1) {
                if (size == 1) drawPixel(x + i, y + j, color);
                else fillRect(x + (i * size), y + (j * size), size, size, color);
            } else if (bg != color) { 
                if (size == 1) drawPixel(x + i, y + j, bg);
                else fillRect(x + (i * size), y + (j * size), size, size, bg);
            }
        }
    }
}

void TFT_GFX::print(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        if (x + (size * 6) >= WIDTH) {
            x = 0;
            y += size * 8;
        }
        drawChar(x, y, *str, color, bg, size);
        x += size * 6; 
        str++;
    }
}