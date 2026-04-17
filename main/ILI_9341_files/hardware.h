#ifndef HARDWARE_H
#define HARDWARE_H

#include "driver/spi_master.h"
#include "driver/gpio.h"

// --- SPI2: Magistrala dla EKRANU ---
#define PIN_NUM_TFT_MOSI 10
#define PIN_NUM_TFT_CLK  9
#define PIN_NUM_TFT_MISO -1  // Wyłączamy MISO ekranu, aby zwolnić pin 8

// --- SPI3: Magistrala dla DOTYKU (Twoje nowe piny) ---
#define PIN_NUM_TOUCH_MISO 16 // T_DO
#define PIN_NUM_TOUCH_MOSI 17 // T_DIN
#define PIN_NUM_TOUCH_CLK  8  // T_CLK

// --- Piny sterujące Ekranem ---
#define PIN_NUM_TFT_CS  13
#define PIN_NUM_TFT_DC  11
#define PIN_NUM_TFT_RST 12
#define PIN_NUM_TFT_LED 46

// --- Piny sterujące Dotykiem ---
#define PIN_NUM_TOUCH_CS  18
#define PIN_NUM_TOUCH_IRQ 15

extern spi_device_handle_t spi_tft;
extern spi_device_handle_t spi_touch;

void init_hardware(void);

#endif