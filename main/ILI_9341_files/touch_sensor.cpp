#include "touch_sensor.h"
#include "hardware.h"
#include "esp_rom_sys.h"

#define CMD_READ_X  0xD0
#define CMD_READ_Y  0x90

void TouchSensor::init() { } // Puste - w ESP-IDF robi to bus_add_device

uint16_t TouchSensor::readSPI(uint8_t command) {
    uint8_t tx_data[3] = {command, 0x00, 0x00};
    uint8_t rx_data[3] = {0};

    spi_transaction_t t = {};
    t.length = 24;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;

    spi_device_polling_transmit(Hardware::spi_touch, &t);
    return ((rx_data[1] << 8) | rx_data[2]) >> 3;
}

bool TouchSensor::isPressed() {
    return (gpio_get_level((gpio_num_t)PIN_NUM_TOUCH_IRQ) == 0);
}

bool TouchSensor::getRaw(uint16_t &x, uint16_t &y) {
    if (!isPressed()) return false; 

    uint32_t sum_x = 0, sum_y = 0;
    const int samples = 4;

    for (int i = 0; i < samples; i++) {
        sum_x += readSPI(CMD_READ_X);
        esp_rom_delay_us(50);
        sum_y += readSPI(CMD_READ_Y);
        esp_rom_delay_us(50);
    }

    x = sum_x / samples;
    y = sum_y / samples;

    if (x == 0 || y == 0 || x > 4090 || y > 4090) return false;
    return true;
}

long TouchSensor::mapVal(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool TouchSensor::getCoordinates(uint16_t &x_pos, uint16_t &y_pos) {
    uint16_t raw_x, raw_y;
    if (!getRaw(raw_x, raw_y)) return false;

    const uint16_t SCREEN_W = 320;
    const uint16_t SCREEN_H = 240;

    long temp_px = mapVal(raw_y, 200, 3750, 0, SCREEN_W);
    long temp_py = mapVal(raw_x, 300, 3800, 0, SCREEN_H);
    
    temp_px = SCREEN_W - temp_px;  // Odwroc X
    temp_py = SCREEN_H - temp_py;  // Odwroc Y

    if (temp_px < 0) temp_px = 0;
    if (temp_px >= SCREEN_W) temp_px = SCREEN_W - 1;

    if (temp_py < 0) temp_py = 0;
    if (temp_py >= SCREEN_H) temp_py = SCREEN_H - 1;

    x_pos = (uint16_t)temp_px;
    y_pos = (uint16_t)temp_py;

    return true;
}

bool TouchSensor::isBtnPressed(uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    return (tx >= x && tx <= (x + w) && ty >= y && ty <= (y + h));
}