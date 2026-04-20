/**
# Project: ESP_S3_RFID
# Author: Łukasz Gąsecki
# Description: Main application entry point with state transition logging.
*/

#include <stdio.h>
#include <string.h>
#include "esp_log.h" // Biblioteka do logowania w terminalu
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ILI_9341_files/hardware.h"
#include "ILI_9341_files/tft_gfx.h"
#include "ILI_9341_files/touch_sensor.h"
#include "ILI_9341_files/keypad.h"
#include "ESP-PN532-UART-main/src/PN532_UART.h"

// Definicja tagu dla logów
static const char *TAG = "RFID_APP";

typedef enum {
    APP_STATE_LOGIN,
    APP_STATE_WAIT_RFID,
    APP_STATE_SHOW_ID
} AppState;

struct RFIDEvent {
    char uid_hex[32];
};

QueueHandle_t rfid_queue;
QueueHandle_t touch_queue;

TFT_GFX tft;
TouchSensor touch;

void touch_task(void *pvParameters) {
    TouchSensor* ts = static_cast<TouchSensor*>(pvParameters);
    TouchEvent event; 
    while (1) {
        if (ts->getCoordinates(event.x, event.y)) {
            xQueueSend(touch_queue, &event, 0);
            vTaskDelay(pdMS_TO_TICKS(50)); 
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void rfid_task(void *pvParameters) {
    PN532_UART rfid(PN532_UART_PORT, PIN_PN532_TX, PIN_PN532_RX);
    rfid.begin();       
    
    ESP_LOGI(TAG, "Próba wybudzenia PN532...");
    bool woke_up = rfid.setNormalMode(); 
    
    if (woke_up) {
        ESP_LOGI(TAG, ">>> SUKCES: Moduł PN532 odpowiedział na wybudzenie! <<<");
    } else {
        ESP_LOGE(TAG, ">>> BŁĄD: PN532 milczy. Problem z kablami TX/RX lub zasilaniem! <<<");
    }

    // Odpytanie o wersję chipa (ostateczny test komunikacji)
    std::vector<uint8_t> version = rfid.getVersion();
    if (version.size() > 0) {
        ESP_LOGI(TAG, "Wersja układu: Chip PN5%x, Firmware %d.%d", version[0], version[1], version[2]);
    } else {
        ESP_LOGE(TAG, "BŁĄD: Nie można odczytać wersji. Komunikacja UART leży.");
    }

    RFIDEvent event;
    int silent_scans = 0;

    while (1) {
        TagTechnology::Iso14aTagInfo tag = rfid.hf14aScan();
        
        if (tag.uidSize > 0) {
            strncpy(event.uid_hex, tag.uid_hex.c_str(), sizeof(event.uid_hex) - 1);
            event.uid_hex[sizeof(event.uid_hex) - 1] = '\0';
            
            ESP_LOGI(TAG, "Zadanie RFID: Wykryto kartę o UID: %s", event.uid_hex);
            xQueueSend(rfid_queue, &event, 0);
            vTaskDelay(pdMS_TO_TICKS(2000)); 
            silent_scans = 0; 
        } else {
            silent_scans++;
            if(silent_scans % 5 == 0) {
               // Zmienione na ESP_LOGI, teraz będzie to widoczne w terminalu co sekundę
               ESP_LOGI(TAG, "Skanowanie... Brak karty w polu anteny (próba %d)", silent_scans); 
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

extern "C" void app_main(void) {
    Hardware::init();
    tft.init();
    
    ESP_LOGI(TAG, "--- Start systemu ESP_S3_RFID ---");
    ESP_LOGI(TAG, "Aktualny stan: LOGIN");

    Keypad keypad(tft, touch);
    keypad.draw();

    rfid_queue = xQueueCreate(5, sizeof(RFIDEvent));
    touch_queue = xQueueCreate(5, sizeof(TouchEvent));
    
    xTaskCreate(rfid_task, "RFID_Task", 4096, NULL, 5, NULL);
    xTaskCreate(touch_task, "Touch_Task", 4096, &touch, 5, NULL);

    AppState current_state = APP_STATE_LOGIN;
    RFIDEvent rfid_data;
    TouchEvent touch_data;

    while (1) {
        if (current_state == APP_STATE_LOGIN) {
            if (xQueueReceive(touch_queue, &touch_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                if (keypad.handleLoginTouch(touch_data.x, touch_data.y)) {
                    ESP_LOGI(TAG, "Logowanie OK. Przejście do stanu: WAIT_RFID");
                    
                    tft.fillScreen(TFT_BLACK);
                    tft.print(40, 100, "ZALOGOWANO!", TFT_GREEN, TFT_BLACK, 2);
                    tft.print(20, 140, "ZBLIZ KARTE RFID...", TFT_WHITE, TFT_BLACK, 1);
                    
                    current_state = APP_STATE_WAIT_RFID;
                }
            }
        }
        
        else if (current_state == APP_STATE_WAIT_RFID) {
            if (xQueueReceive(rfid_queue, &rfid_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                ESP_LOGI(TAG, "Otrzymano dane RFID z kolejki. Przejście do stanu: SHOW_ID");
                
                tft.fillScreen(TFT_BLACK);
                tft.print(40, 50, "KARTA WYKRYTA:", TFT_YELLOW, TFT_BLACK, 2);
                tft.print(40, 100, rfid_data.uid_hex, TFT_DARKCYAN, TFT_BLACK, 2);
                tft.print(40, 200, "[ POWROT ]", TFT_RED, TFT_BLACK, 2);
                
                current_state = APP_STATE_SHOW_ID;
            }
        }
        
        else if (current_state == APP_STATE_SHOW_ID) {
            if (xQueueReceive(touch_queue, &touch_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                ESP_LOGI(TAG, "Użytkownik dotknął ekranu powrotu. Przejście do stanu: LOGIN");
                
                keypad.draw();
                current_state = APP_STATE_LOGIN;
            }
        }
    }
}