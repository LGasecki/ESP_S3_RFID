#include "touch_sensor.h"
#include "hardware.h"
#include "esp_rom_sys.h"

#define CMD_READ_X  0xD0
#define CMD_READ_Y  0x90

static uint16_t Touch_ReadSPI(uint8_t command) {
    uint8_t tx_data[3] = {command, 0x00, 0x00};
    uint8_t rx_data[3] = {0};

    spi_transaction_t t = {
        .length = 24,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data
    };

    spi_device_polling_transmit(spi_touch, &t);

    uint16_t result = ((rx_data[1] << 8) | rx_data[2]) >> 3;
    return result;
}

void Touch_Init(void) {
    // Puste - w ESP-IDF inicjalizacja robi się sama w spi_bus_add_device
}

bool Touch_IsPressed(void) {
    return (gpio_get_level(PIN_NUM_TOUCH_IRQ) == 0);
}

bool Touch_GetRaw(uint16_t *x, uint16_t *y) {
    if (!Touch_IsPressed()) return false; 

    uint32_t sum_x = 0, sum_y = 0;
    const int samples = 4;

    for (int i = 0; i < samples; i++) {
        sum_x += Touch_ReadSPI(CMD_READ_X);
        esp_rom_delay_us(50);
        sum_y += Touch_ReadSPI(CMD_READ_Y);
        esp_rom_delay_us(50);
    }

    *x = sum_x / samples;
    *y = sum_y / samples;

    if (*x == 0 || *y == 0 || *x > 4090 || *y > 4090) return false;

    return true;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool Touch_GetCoordinates(uint16_t *x_pos, uint16_t *y_pos) {
    uint16_t raw_x, raw_y;

    if (!Touch_GetRaw(&raw_x, &raw_y)) return false;

    const uint16_t SCREEN_W = 320;
    const uint16_t SCREEN_H = 240;

    const uint16_t TS_MINX = 300;
    const uint16_t TS_MAXX = 3800;
    const uint16_t TS_MINY = 200;
    const uint16_t TS_MAXY = 3750;

    long temp_px, temp_py;

    temp_px = map(raw_y, TS_MINY, TS_MAXY, 0, SCREEN_W);
    temp_py = map(raw_x, TS_MINX, TS_MAXX, 0, SCREEN_H);
    
    temp_px = SCREEN_W - temp_px;  // Odwroc X
    temp_py = SCREEN_H - temp_py;  // Odwroc Y

    if (temp_px < 0) temp_px = 0;
    if (temp_px >= SCREEN_W) temp_px = SCREEN_W - 1;

    if (temp_py < 0) temp_py = 0;
    if (temp_py >= SCREEN_H) temp_py = SCREEN_H - 1;

    *x_pos = (uint16_t)temp_px;
    *y_pos = (uint16_t)temp_py;

    return true;
}

bool Is_Btn_Pressed(uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    return (tx >= x && tx <= (x + w) && ty >= y && ty <= (y + h));
}