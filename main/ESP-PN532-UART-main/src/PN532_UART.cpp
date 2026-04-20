#include "PN532_UART.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "PN532_UART";

PN532_UART::PN532_UART(uart_port_t uart_num, int tx_pin, int rx_pin) 
    : _uart_num(uart_num), _tx_pin(tx_pin), _rx_pin(rx_pin) {}

PN532_UART::~PN532_UART() { close(); }

void PN532_UART::begin() {
    uart_config_t uart_config = {};
    uart_config.baud_rate = 115200;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;

    // 1. Najpierw konfiguracja parametrów
    uart_param_config(_uart_num, &uart_config);
    
    // 2. Potem ustawienie pinów
    uart_set_pin(_uart_num, _tx_pin, _rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // 3. Na końcu instalacja sterownika z buforem RX = 1024 oraz TX = 1024 (tutaj był błąd - brak bufora TX!)
    uart_driver_install(_uart_num, 1024, 1024, 0, NULL, 0);
}

uint8_t PN532_UART::dcs(uint8_t *data, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) { checksum += data[i]; }
    return (0x00 - checksum) & 0xFF;
}

std::vector<uint8_t> PN532_UART::writeCommand(uint8_t cmd, const std::vector<uint8_t> &datas, int time_out) {
    uint8_t *data = const_cast<uint8_t *>(datas.data());
    size_t length = datas.size();
    std::vector<uint8_t> commands;
    commands.push_back(PN532Cmd::Data_Tif_Send);
    commands.push_back(cmd);
    commands.insert(commands.end(), data, data + length);

    std::vector<uint8_t> frame;
    frame.push_back(PN532Cmd::Data_Preamble);
    frame.push_back(PN532Cmd::Data_Start_Code_0);
    frame.push_back(PN532Cmd::Data_Start_Code_1);

    uint8_t len = commands.size();
    uint8_t length_check_sum = (0x00 - len) & 0xFF;
    frame.push_back(len);
    frame.push_back(length_check_sum);
    frame.insert(frame.end(), commands.begin(), commands.end());

    uint8_t dcs_value = dcs(commands.data(), commands.size());
    frame.push_back(dcs_value);
    frame.push_back(PN532Cmd::Data_Postamble);

    // Wysyłanie przez ESP-IDF UART
    uart_write_bytes(_uart_num, frame.data(), frame.size());

    uint8_t response[256];
    TickType_t startTick = xTaskGetTickCount();
    int resLength = 0;

    // Odczyt z timeoutem
    while ((xTaskGetTickCount() - startTick) * portTICK_PERIOD_MS < time_out) {
        int bytes_read = uart_read_bytes(_uart_num, response + resLength, sizeof(response) - resLength, pdMS_TO_TICKS(10));
        if (bytes_read > 0) {
            resLength += bytes_read;
        }
        if (resLength >= 6) { break; } 
    }

    if (resLength < 6) {
        // ESP_LOGW(TAG, "Timeout or invalid response length"); // Opcjonalny log błędów
        return {};
    }

    // Ekstrakcja danych
    uint8_t res_len = response[9];
    uint8_t res_length_check_sum = response[10];
    if (((res_len + res_length_check_sum) & 0xFF) != 0) {
        ESP_LOGE(TAG, "Length checksum error");
        return {};
    }
    uint8_t res_dcs_value = response[11 + res_len];
    if (dcs(response + 11, res_len) != res_dcs_value) {
        ESP_LOGE(TAG, "Data checksum error");
        return {};
    }

    std::vector<uint8_t> payload(response + 13, response + 13 + res_len);
    return payload;
}

std::vector<uint8_t> PN532_UART::writeRawCommand(const std::vector<uint8_t> data, int time_out) {
    uart_write_bytes(_uart_num, data.data(), data.size());
    uint8_t response[256];
    int resLength = uart_read_bytes(_uart_num, response, sizeof(response), pdMS_TO_TICKS(time_out));
    std::vector<uint8_t> responseData(response, response + resLength);
    return responseData;
}

bool PN532_UART::setNormalMode() {
    std::vector<uint8_t> wakeUpCommand = {
        0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uart_write_bytes(_uart_num, wakeUpCommand.data(), wakeUpCommand.size());
    std::vector<uint8_t> response = writeCommand(PN532Cmd::SAMConfiguration, {0x01});
    vTaskDelay(pdMS_TO_TICKS(10));
    return response.size() > 0;
}

std::vector<uint8_t> PN532_UART::getVersion() { return writeCommand(PN532Cmd::GetFirmwareVersion); }

TagTechnology::Iso14aTagInfo PN532_UART::hf14aScan() {
    std::vector<uint8_t> result = writeCommand(PN532Cmd::InListPassiveTarget, {0x01, 0x00}, 1000);
    if (result.size() < 6) { return TagTechnology::Iso14aTagInfo(); }
    return tagTechnology.parseIso14aTag(result.data(), result.size());
}

void PN532_UART::close() { 
    uart_driver_delete(_uart_num); 
}

std::vector<uint8_t> PN532_UART::getData() { return writeCommand(PN532Cmd::TgGetData); }

std::vector<uint8_t> PN532_UART::setData(const std::vector<uint8_t> &data) {
    return writeCommand(PN532Cmd::TgSetData, data);
}

bool PN532_UART::inRelease() {
    std::vector<uint8_t> response = writeCommand(PN532Cmd::InRelease, {0x00});
    return response.size() > 0;
}

std::vector<uint8_t> PN532_UART::tgInitAsTarget(const std::vector<uint8_t> &data) {
    return writeCommand(PN532Cmd::TgInitAsTarget, data);
}