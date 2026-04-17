/**
# Project: ESP_S3_RFID
#
# Author: Łukasz Gąsecki
# Description: Main application entry point. Initializes hardware, FreeRTOS tasks, and queues, and manages the main application loop for handling touch interactions and UI updates.
# 
*/

// --- 1. Biblioteki Standardowe C ---
#include <stdio.h>

// --- 2. System Operacyjny (FreeRTOS) ---
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// --- 3. Sterowniki systemowe ESP-IDF ---
#include "driver/gpio.h"
#include "driver/spi_master.h"

// --- 4. Moduły lokalne projektu ---
#include "ILI_9341_files/hardware.h"
#include "ILI_9341_files/touch_sensor.h"
#include "ILI_9341_files/tft_gfx.h"
#include "ILI_9341_files/keypad.h"

spi_device_handle_t spi_tft;
spi_device_handle_t spi_touch;

// --- DEFINICJA STANÓW APLIKACJI ---
typedef enum {
    APP_STATE_LOGIN,
    APP_STATE_MAIN_MENU
} AppState;

QueueHandle_t touch_queue;

// --- ZADANIE 1: OBSŁUGA DOTYKU ---
void touch_task(void *pvParameters) {
    TouchEvent event; 
    
    while (1) {
        if (Touch_GetCoordinates(&event.x, &event.y)) {
            xQueueSend(touch_queue, &event, 0);
            vTaskDelay(pdMS_TO_TICKS(50)); 
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

// --- ZADANIE 2: GŁÓWNA PĘTLA ---
void app_main(void) {
    init_hardware();
    TFT_Init();
    Draw_Keypad();

    // Domyślny stan po restarcie urządzenia
    AppState current_state = APP_STATE_LOGIN;

    touch_queue = xQueueCreate(5, sizeof(TouchEvent));
    xTaskCreate(touch_task, "Touch_Task", 4096, NULL, 5, NULL);

    TouchEvent received_event;

    while (1) {
        // Czekamy na dotyk
        if (xQueueReceive(touch_queue, &received_event, pdMS_TO_TICKS(20)) == pdTRUE) {
            
            // --- MASZYNA STANÓW (Switch - Case) ---
            switch (current_state) {
                
                case APP_STATE_LOGIN:
                    // --- Ekran 1: Klawiatura ---
                    if (Handle_Login_Touch(received_event.x, received_event.y)) {
                        
                        // Przejście: PIN poprawny
                        TFT_FillScreen(TFT_BLACK);
                        TFT_Print(100, 110, "ZALOGOWANO", TFT_WHITE, TFT_DARKCYAN, 2);
                        
                        // ZMIANA STANU
                        current_state = APP_STATE_MAIN_MENU;
                    }
                    break; 

                case APP_STATE_MAIN_MENU:
                    // --- Ekran 2: Menu Główne ---

                    if (Is_Btn_Pressed(received_event.x, received_event.y, 10, 10, 100, 40)) {
                        Update_Pin_Display();
                        Draw_Keypad(); // Rysujemy klawiaturę na nowo
                        current_state = APP_STATE_LOGIN; // Wracamy do stanu logowania
                    }

                    break;

                default:
                    // Opcjonalnie: zabezpieczenie na wypadek błędu pamięci
                    break;
            }
            
        }
        
        // --- Tutaj w przyszłości zrobisz podobnego switch-case dla Kolejki RFID! ---
    }
}