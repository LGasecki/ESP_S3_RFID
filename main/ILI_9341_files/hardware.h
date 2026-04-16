#ifndef HARDWARE_H
#define HARDWARE_H

#include "driver/spi_master.h"
#include "driver/gpio.h"

// --- Piny SPI (Dostosuj do swoich połączeń) ---
#define PIN_NUM_MISO  8
#define PIN_NUM_MOSI  10
#define PIN_NUM_CLK   9

// --- Piny Ekranu TFT ---
#define PIN_NUM_TFT_CS 13
#define PIN_NUM_TFT_DC 11
#define PIN_NUM_TFT_RST 12
#define PIN_NUM_TFT_LED 46

// --- Piny Dotyku (XPT2046) ---
#define PIN_NUM_TOUCH_CS  18
#define PIN_NUM_TOUCH_IRQ 15

// Globalne uchwyty (handle) do urządzeń SPI
extern spi_device_handle_t spi_tft;
extern spi_device_handle_t spi_touch;

#endif