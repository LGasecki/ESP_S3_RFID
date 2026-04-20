#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"

// --- Piny ---
#define PIN_NUM_TFT_MOSI 10
#define PIN_NUM_TFT_CLK  9
#define PIN_NUM_TFT_MISO -1

#define PIN_NUM_TOUCH_MISO 16
#define PIN_NUM_TOUCH_MOSI 17
#define PIN_NUM_TOUCH_CLK  8

#define PIN_NUM_TFT_CS  13
#define PIN_NUM_TFT_DC  11
#define PIN_NUM_TFT_RST 12
#define PIN_NUM_TFT_LED 46

#define PIN_NUM_TOUCH_CS  18
#define PIN_NUM_TOUCH_IRQ 15

class Hardware {
public:
    static spi_device_handle_t spi_tft;
    static spi_device_handle_t spi_touch;

    static void init();
};