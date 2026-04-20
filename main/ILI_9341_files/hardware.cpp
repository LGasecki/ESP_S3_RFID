/**
# Project: ESP_S3_RFID
#
# Author: Łukasz Gąsecki
# Description: Configures the ESP32-S3 hardware peripherals, including independent SPI buses (SPI2 and SPI3) and GPIO pins for the TFT display and touch sensor.
# 
*/

#include "hardware.h"

spi_device_handle_t Hardware::spi_tft = nullptr;
spi_device_handle_t Hardware::spi_touch = nullptr;

void Hardware::init() {
    // 1. Inicjalizacja SPI2 dla Wyświetlacza
    spi_bus_config_t tft_buscfg = {};
    tft_buscfg.miso_io_num = PIN_NUM_TFT_MISO;
    tft_buscfg.mosi_io_num = PIN_NUM_TFT_MOSI;
    tft_buscfg.sclk_io_num = PIN_NUM_TFT_CLK;
    tft_buscfg.quadwp_io_num = -1;
    tft_buscfg.quadhd_io_num = -1;
    tft_buscfg.max_transfer_sz = 320 * 240 * 2 + 8;

    spi_bus_initialize(SPI2_HOST, &tft_buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t tft_cfg = {};
    tft_cfg.clock_speed_hz = 40 * 1000 * 1000; 
    tft_cfg.mode = 0;                               
    tft_cfg.spics_io_num = PIN_NUM_TFT_CS;     
    tft_cfg.queue_size = 7;                          

    spi_bus_add_device(SPI2_HOST, &tft_cfg, &spi_tft);

    // 2. Inicjalizacja SPI3 dla Dotyku
    spi_bus_config_t touch_buscfg = {};
    touch_buscfg.miso_io_num = PIN_NUM_TOUCH_MISO;
    touch_buscfg.mosi_io_num = PIN_NUM_TOUCH_MOSI;
    touch_buscfg.sclk_io_num = PIN_NUM_TOUCH_CLK;
    touch_buscfg.quadwp_io_num = -1;
    touch_buscfg.quadhd_io_num = -1;

    spi_bus_initialize(SPI3_HOST, &touch_buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t touch_cfg = {};
    touch_cfg.clock_speed_hz = 2 * 1000 * 1000; 
    touch_cfg.mode = 0;                               
    touch_cfg.spics_io_num = PIN_NUM_TOUCH_CS;   
    touch_cfg.queue_size = 3;                          

    spi_bus_add_device(SPI3_HOST, &touch_cfg, &spi_touch);

    // 3. Konfiguracja pinów
    gpio_set_direction((gpio_num_t)PIN_NUM_TFT_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_NUM_TFT_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_NUM_TFT_LED, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_NUM_TFT_LED, 1); 
    
    gpio_set_direction((gpio_num_t)PIN_NUM_TOUCH_IRQ, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)PIN_NUM_TOUCH_IRQ, GPIO_PULLUP_ONLY);
}