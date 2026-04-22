#include "rfid_task.h"
#include "app_events.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "pn532.h"
#include "pn532_driver_hsu.h"

#define HSU_HOST_RX    GPIO_NUM_4
#define HSU_HOST_TX    GPIO_NUM_5
#define HSU_UART_PORT  UART_NUM_1
#define HSU_BAUD_RATE  115200

static const char *TAG = "RFID_TASK";

void rfid_task(void *pvParameters) {
    pn532_io_t pn532_io;
    esp_err_t err;

    vTaskDelay(pdMS_TO_TICKS(2000)); 

    ESP_LOGI(TAG, "Inicjalizacja sterownika PN532 HSU...");
    err = pn532_new_driver_hsu(HSU_HOST_RX, HSU_HOST_TX, GPIO_NUM_NC, GPIO_NUM_NC, HSU_UART_PORT, HSU_BAUD_RATE, &pn532_io);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Błąd tworzenia sterownika HSU.");
        vTaskDelete(NULL);
    }

    do {
        err = pn532_init(&pn532_io);
        if (err != ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    } while(err != ESP_OK);

    uint32_t version_data = 0;
    do {
        err = pn532_get_firmware_version(&pn532_io, &version_data);
        if (ESP_OK != err) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    } while (ESP_OK != err);

    ESP_LOGI(TAG, "Znaleziono chip PN5%x", (unsigned int)(version_data >> 24) & 0xFF);

    RFIDEvent event;
    while (1) {
        uint8_t uid[7] = {0};
        uint8_t uid_length = 0;

        err = pn532_read_passive_target_id(&pn532_io, PN532_BRTY_ISO14443A_106KBPS, uid, &uid_length, 1000);

        if (ESP_OK == err && uid_length > 0) {
            char uid_hex_str[32] = {0};
            for (int i = 0; i < uid_length; i++) {
                sprintf(&uid_hex_str[i * 2], "%02X", uid[i]);
            }

            strncpy(event.uid_hex, uid_hex_str, sizeof(event.uid_hex) - 1);
            event.uid_hex[sizeof(event.uid_hex) - 1] = '\0';
            
            ESP_LOGI(TAG, "Wykryto tag RFID: %s", event.uid_hex);
            xQueueSend(rfid_queue, &event, 0);
            
            vTaskDelay(pdMS_TO_TICKS(1500)); 
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}