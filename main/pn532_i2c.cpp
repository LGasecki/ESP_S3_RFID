#include "pn532_i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define PN532_I2C_PORT I2C_NUM_0
#define PN532_I2C_ADDR 0x24 // Odpowiada 0x48 >> 1

// Konstruktor - jawne rzutowanie int na gpio_num_t naprawia błąd kompilacji
PN532_I2C::PN532_I2C(int sda_pin, int scl_pin) 
    : _sda((gpio_num_t)sda_pin), _scl((gpio_num_t)scl_pin), _dev_handle(nullptr) {}

bool PN532_I2C::begin() {
    // 1. Inicjalizacja nowej szyny (Master Bus)
    i2c_master_bus_config_t i2c_mst_config = {};
    i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_mst_config.i2c_port = PN532_I2C_PORT;
    i2c_mst_config.scl_io_num = _scl;
    i2c_mst_config.sda_io_num = _sda;
    i2c_mst_config.glitch_ignore_cnt = 7;
    i2c_mst_config.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus_handle;
    esp_err_t ret = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
    if (ret != ESP_OK) return false;

    // 2. Dodanie urządzenia PN532 do szyny
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = PN532_I2C_ADDR;
    dev_cfg.scl_speed_hz = 100000; // 100 kHz

    ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &_dev_handle);
    if (ret != ESP_OK) return false;

    // 3. Wyślij komendę SAMConfiguration (Tryb Normalny)
    uint8_t cmd[] = { 0x14, 0x01, 0x14, 0x01 };
    writeCommand(cmd, sizeof(cmd));
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t ackBuff[8];
    return readResponse(ackBuff, 8); // Zwraca true, jeśli odpowiedział ACK
}

void PN532_I2C::writeCommand(uint8_t* cmd, uint8_t cmdlen) {
    uint8_t buff[64];
    uint8_t checksum = 0;
    int i = 0;

    buff[i++] = 0x00; // Preamble
    buff[i++] = 0x00; // Start 1
    buff[i++] = 0xFF; // Start 2
    buff[i++] = cmdlen + 1; // Len
    buff[i++] = (uint8_t)(~(cmdlen + 1) + 1); // LCS
    buff[i++] = 0xD4; // Host to PN532
    checksum += 0xD4;

    for (int k = 0; k < cmdlen; k++) {
        buff[i++] = cmd[k];
        checksum += cmd[k];
    }

    buff[i++] = (uint8_t)(~checksum + 1); // DCS
    buff[i++] = 0x00; // Postamble

    // Nowa komenda zapisu z v6.0
    i2c_master_transmit(_dev_handle, buff, i, pdMS_TO_TICKS(100));
}

bool PN532_I2C::isReady() {
    uint8_t status = 0;
    // Odczyt jednego bajtu. Gdy PN532 przetworzył dane, zwraca 0x01.
    esp_err_t ret = i2c_master_receive(_dev_handle, &status, 1, pdMS_TO_TICKS(10));
    return (ret == ESP_OK && status == 0x01);
}

bool PN532_I2C::readResponse(uint8_t* buffer, uint8_t maxLen) {
    int timeout = 200;
    // Czekanie na stan gotowości
    while (!isReady()) {
        vTaskDelay(pdMS_TO_TICKS(2));
        if (--timeout == 0) return false;
    }
    
    // Odczyt właściwych danych ramki z v6.0
    esp_err_t ret = i2c_master_receive(_dev_handle, buffer, maxLen, pdMS_TO_TICKS(100));
    return (ret == ESP_OK);
}

bool PN532_I2C::readPassiveTargetID(PN532_Tag *tagWskaznik) {
    uint8_t cmd[] = { 0x4A, 0x01, 0x00 };
    writeCommand(cmd, sizeof(cmd));

    vTaskDelay(pdMS_TO_TICKS(5));
    uint8_t ackBuff[8];
    if (!readResponse(ackBuff, 8)) return false; // Brak ACK
    
    vTaskDelay(pdMS_TO_TICKS(30));
    uint8_t response[32];
    
    if (readResponse(response, 24)) {
        for (int i = 0; i < 18; i++) {
            if (response[i] == 0xD5 && response[i+1] == 0x4B) { 
                if (response[i+2] == 1) { 
                    tagWskaznik->uidLen = response[i+7];
                    if (tagWskaznik->uidLen > 7) tagWskaznik->uidLen = 7;
                    for (int k = 0; k < tagWskaznik->uidLen; k++) {
                        tagWskaznik->uid[k] = response[i+8+k];
                    }
                    return true;
                }
            }
        }
    }
    return false;
}