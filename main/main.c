#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "ILI_9341_files/hardware.h"
#include "ILI_9341_files/tft_gfx.h"
#include "ILI_9341_files/touch_sensor.h"



spi_device_handle_t spi_tft;
spi_device_handle_t spi_touch;

void init_hardware() {
    // 1. Inicjalizacja SPI2 dla Wyświetlacza
    spi_bus_config_t tft_buscfg = {
        .miso_io_num = PIN_NUM_TFT_MISO,
        .mosi_io_num = PIN_NUM_TFT_MOSI,
        .sclk_io_num = PIN_NUM_TFT_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2 + 8
    };
    spi_bus_initialize(SPI2_HOST, &tft_buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t tft_cfg = {
        .clock_speed_hz = 40 * 1000 * 1000, 
        .mode = 0,                               
        .spics_io_num = PIN_NUM_TFT_CS,     
        .queue_size = 7,                          
    };
    spi_bus_add_device(SPI2_HOST, &tft_cfg, &spi_tft);

    // 2. Inicjalizacja SPI3 dla Dotyku
    spi_bus_config_t touch_buscfg = {
        .miso_io_num = PIN_NUM_TOUCH_MISO,
        .mosi_io_num = PIN_NUM_TOUCH_MOSI,
        .sclk_io_num = PIN_NUM_TOUCH_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };
    spi_bus_initialize(SPI3_HOST, &touch_buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t touch_cfg = {
        .clock_speed_hz = 2 * 1000 * 1000, 
        .mode = 0,                               
        .spics_io_num = PIN_NUM_TOUCH_CS,   
        .queue_size = 3,                          
    };
    spi_bus_add_device(SPI3_HOST, &touch_cfg, &spi_touch);

    // 3. Konfiguracja pinów sterujących (DC, RST, IRQ, LED)
    gpio_set_direction(PIN_NUM_TFT_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_TFT_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_TFT_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_TFT_LED, 1); // Włącz podświetlenie
    
    gpio_set_direction(PIN_NUM_TOUCH_IRQ, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_NUM_TOUCH_IRQ, GPIO_PULLUP_ONLY);
}

void app_main(void) {
    init_hardware();
    TFT_Init();
    
    TFT_FillScreen(TFT_BLUE);
    TFT_Print(10, 10, "ESP32-S3 gotowy!", TFT_WHITE, TFT_BLUE, 2);

    while (1) {
        uint16_t tx, ty;
        if (Touch_GetCoordinates(&tx, &ty)) {
            TFT_FillRect(tx, ty, 5, 5, TFT_RED); // Rysowanie palcem
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Czas dla FreeRTOS (50 FPS)
    }
}