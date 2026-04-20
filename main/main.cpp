/**
# Project: ESP_S3_RFID
#
# Author: Łukasz Gąsecki
# Description: Main application entry point. Initializes hardware, FreeRTOS tasks, and queues, and manages the main application loop for handling touch interactions and UI updates.
# 
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Dołączamy nagłówki naszych nowych klas
#include "ILI_9341_files/hardware.h"
#include "ILI_9341_files/tft_gfx.h"
#include "ILI_9341_files/touch_sensor.h"
#include "ILI_9341_files/keypad.h"

typedef enum {
    APP_STATE_LOGIN,
    APP_STATE_MAIN_MENU
} AppState;

QueueHandle_t touch_queue;

// --- Globalne Obiekty (lub mogą być wewnątrz app_main) ---
TFT_GFX tft;
TouchSensor touch;

// ZADANIE 1: OBSŁUGA DOTYKU
// Parametr "pvParameters" pozwala nam wstrzyknąć obiekt TouchSensor
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

// Główny punkt wejścia. Wymaga extern "C" ze względu na ESP-IDF
extern "C" void app_main(void) {
    // 1. Inicjalizacja sprzętu i klas
    Hardware::init();
    tft.init();
    touch.init();
    
    Keypad keypad(tft, touch);
    keypad.draw();

    AppState current_state = APP_STATE_LOGIN;

    touch_queue = xQueueCreate(5, sizeof(TouchEvent));
    
    // Przekazujemy adres obiektu touch do zadania FreeRTOS
    xTaskCreate(touch_task, "Touch_Task", 4096, &touch, 5, NULL);

    TouchEvent received_event;

    while (1) {
        if (xQueueReceive(touch_queue, &received_event, pdMS_TO_TICKS(20)) == pdTRUE) {
            
            switch (current_state) {
                case APP_STATE_LOGIN:
                    if (keypad.handleLoginTouch(received_event.x, received_event.y)) {
                        tft.fillScreen(TFT_BLACK);
                        tft.print(100, 110, "ZALOGOWANO", TFT_WHITE, TFT_DARKCYAN, 2);
                        current_state = APP_STATE_MAIN_MENU;
                    }
                    break; 

                case APP_STATE_MAIN_MENU:
                    // Przykładowy przycisk wylogowania w Main Menu
                    if (TouchSensor::isBtnPressed(received_event.x, received_event.y, 10, 10, 100, 40)) {
                        keypad.updatePinDisplay();
                        keypad.draw(); 
                        current_state = APP_STATE_LOGIN; 
                    }
                    break;

                default:
                    break;
            }
        }
    }
}