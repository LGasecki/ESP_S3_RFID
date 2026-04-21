#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hal/gpio_types.h"
#include "driver/i2c_master.h"

struct PN532_Tag {
    uint8_t uid[7];
    uint8_t uidLen;
};

class PN532_I2C {
public:
    // Zostawiamy 'int' w konstruktorze dla wygody, zrzutujemy go w środku
    PN532_I2C(int sda_pin, int scl_pin);
    bool begin();
    bool readPassiveTargetID(PN532_Tag *tag);

private:
    gpio_num_t _sda;
    gpio_num_t _scl;
    i2c_master_dev_handle_t _dev_handle; // Uchwyt do urządzenia na nowej szynie v6.0

    void writeCommand(uint8_t* cmd, uint8_t cmdlen);
    bool isReady();
    bool readResponse(uint8_t* buffer, uint8_t maxLen);
};