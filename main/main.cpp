/**
# Project: ESP_S3_RFID
#
# Author: Łukasz Gąsecki
# Description: Główny punkt wejścia aplikacji. Zarządza maszyną stanów, 
#              kolejkami FreeRTOS oraz interakcją między UI a czytnikiem RFID.
# 
*/

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ILI_9341_files/hardware.h"
#include "ILI_9341_files/tft_gfx.h"
#include "ILI_9341_files/touch_sensor.h"
#include "ILI_9341_files/keypad.h"
#include "ESP-PN532-UART-main/src/PN532_UART.h"

static const char *TAG = "RFID_APP";

// Definicja stanów aplikacji
typedef enum {
    APP_STATE_LOGIN,
    APP_STATE_WAIT_RFID,
    APP_STATE_SHOW_ID
} AppState;

// Struktura zdarzenia RFID przesyłana w kolejce
struct RFIDEvent {
    char uid_hex[32];
};

// Kolejki komunikacyjne
QueueHandle_t rfid_queue;
QueueHandle_t touch_queue;

// Obiekty globalne
TFT_GFX tft;
TouchSensor touch;

/**
 * Zadanie obsługujące panel dotykowy.
 * Pobiera współrzędne i wysyła je do kolejki zdarzeń dotyku.
 */
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

/**
 * Zadanie obsługujące czytnik RFID PN532.
 * Inicjalizuje moduł i w pętli skanuje w poszukiwaniu tagów.
 */
void rfid_task(void *pvParameters) {
    PN532_UART rfid(PN532_UART_PORT, PIN_PN532_TX, PIN_PN532_RX);
    rfid.begin();
    
    // Inicjalizacja modułu
    if (rfid.setNormalMode()) {
        ESP_LOGI(TAG, "Moduł PN532 zainicjalizowany pomyślnie.");
    } else {
        ESP_LOGE(TAG, "Błąd inicjalizacji PN532.");
    }

    RFIDEvent event;
    while (1) {
        TagTechnology::Iso14aTagInfo tag = rfid.hf14aScan();
        
        if (tag.uidSize > 0) {
            strncpy(event.uid_hex, tag.uid_hex.c_str(), sizeof(event.uid_hex) - 1);
            event.uid_hex[sizeof(event.uid_hex) - 1] = '\0';
            
            ESP_LOGI(TAG, "Wykryto tag RFID: %s", event.uid_hex);
            xQueueSend(rfid_queue, &event, 0);
            vTaskDelay(pdMS_TO_TICKS(1500)); // Zapobiega wielokrotnemu odczytowi tej samej karty
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

extern "C" void app_main(void) {
    // Inicjalizacja sprzętu
    Hardware::init();
    tft.init();
    
    ESP_LOGI(TAG, "System ESP_S3_RFID uruchomiony.");

    Keypad keypad(tft, touch);
    keypad.draw();

    // Utworzenie kolejek
    rfid_queue = xQueueCreate(5, sizeof(RFIDEvent));
    touch_queue = xQueueCreate(5, sizeof(TouchEvent));
    
    // Uruchomienie zadań
    xTaskCreate(rfid_task, "RFID_Task", 4096, NULL, 5, NULL);
    xTaskCreate(touch_task, "Touch_Task", 4096, &touch, 5, NULL);

    AppState current_state = APP_STATE_LOGIN;
    RFIDEvent rfid_data;
    TouchEvent touch_data;

    while (1) {
        // Maszyna stanów aplikacji oparta na switch-case
        switch (current_state) {
            case APP_STATE_LOGIN:
                if (xQueueReceive(touch_queue, &touch_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                    if (keypad.handleLoginTouch(touch_data.x, touch_data.y)) {
                        tft.fillScreen(TFT_BLACK);
                        tft.print(40, 100, "ZALOGOWANO!", TFT_GREEN, TFT_BLACK, 2);
                        tft.print(20, 140, "ZBLIZ KARTE RFID...", TFT_WHITE, TFT_BLACK, 1);
                        current_state = APP_STATE_WAIT_RFID;
                    }
                }
                break;

            case APP_STATE_WAIT_RFID:
                if (xQueueReceive(rfid_queue, &rfid_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                    tft.fillScreen(TFT_BLACK);
                    tft.print(40, 50, "KARTA WYKRYTA:", TFT_YELLOW, TFT_BLACK, 2);
                    tft.print(40, 100, rfid_data.uid_hex, TFT_DARKCYAN, TFT_BLACK, 2);
                    tft.print(40, 200, "[ POWROT ]", TFT_RED, TFT_BLACK, 2);
                    current_state = APP_STATE_SHOW_ID;
                }
                break;

            case APP_STATE_SHOW_ID:
                if (xQueueReceive(touch_queue, &touch_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                    keypad.draw();
                    current_state = APP_STATE_LOGIN;
                }
                break;
        }
    }
}