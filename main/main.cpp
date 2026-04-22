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

// Własne nagłówki
#include "app_events.h"
#include "tasks/rfid_task.h"
#include "tasks/touch_task.h"

static const char *TAG = "MAIN_APP";

// Inicjalizacja globalnych kolejek (zadeklarowanych jako extern w app_events.h)
QueueHandle_t rfid_queue;
QueueHandle_t touch_queue;

TFT_GFX tft;
TouchSensor touch;

typedef enum {
    APP_STATE_LOGIN,
    APP_STATE_WAIT_RFID,
    APP_STATE_SHOW_ID
} AppState;

extern "C" void app_main(void) {
    Hardware::init();
    tft.init();
    
    ESP_LOGI(TAG, "System ESP_S3_RFID uruchomiony.");

    Keypad keypad(tft, touch);
    keypad.draw();

    rfid_queue = xQueueCreate(5, sizeof(RFIDEvent));
    touch_queue = xQueueCreate(5, sizeof(TouchEvent));
    
    xTaskCreate(rfid_task, "RFID_Task", 4096, NULL, 10, NULL);
    xTaskCreate(touch_task, "Touch_Task", 4096, &touch, 5, NULL);

    AppState current_state = APP_STATE_LOGIN;
    RFIDEvent rfid_data;
    TouchEvent touch_data;
    char last_uid[32] = {0}; 

    while (1) {
        switch (current_state) {
            case APP_STATE_LOGIN:
                if (xQueueReceive(touch_queue, &touch_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                    if (keypad.handleLoginTouch(touch_data.x, touch_data.y)) {
                        tft.fillScreen(TFT_BLACK);
                        tft.print(40, 100, "ZALOGOWANO!", TFT_GREEN, TFT_BLACK, 2);
                        tft.print(20, 140, "ZBLIZ KARTE RFID...", TFT_WHITE, TFT_BLACK, 1);
                        
                        last_uid[0] = '\0'; 
                        current_state = APP_STATE_WAIT_RFID;
                    }
                }
                break;

            case APP_STATE_WAIT_RFID:
                if (xQueueReceive(rfid_queue, &rfid_data, pdMS_TO_TICKS(20)) == pdTRUE) {
                    strncpy(last_uid, rfid_data.uid_hex, sizeof(last_uid)); 
                    
                    tft.fillScreen(TFT_BLACK);
                    tft.print(40, 50, "KARTA WYKRYTA:", TFT_YELLOW, TFT_BLACK, 2);
                    tft.print(40, 100, rfid_data.uid_hex, TFT_DARKCYAN, TFT_BLACK, 2);
                    tft.print(40, 200, "[ POWROT ]", TFT_RED, TFT_BLACK, 2);
                    
                    current_state = APP_STATE_SHOW_ID;
                }
                break;

            case APP_STATE_SHOW_ID:
                if (xQueueReceive(touch_queue, &touch_data, pdMS_TO_TICKS(10)) == pdTRUE) {
                    keypad.draw();
                    current_state = APP_STATE_LOGIN;
                }
                else if (xQueueReceive(rfid_queue, &rfid_data, pdMS_TO_TICKS(10)) == pdTRUE) {
                    if (strcmp(last_uid, rfid_data.uid_hex) != 0) {
                        strncpy(last_uid, rfid_data.uid_hex, sizeof(last_uid)); 
                        
                        tft.fillScreen(TFT_BLACK);
                        tft.print(40, 50, "KARTA WYKRYTA:", TFT_YELLOW, TFT_BLACK, 2);
                        tft.print(40, 100, rfid_data.uid_hex, TFT_DARKCYAN, TFT_BLACK, 2);
                        tft.print(40, 200, "[ POWROT ]", TFT_RED, TFT_BLACK, 2);
                        
                        ESP_LOGI(TAG, "Zaktualizowano ekran: %s", rfid_data.uid_hex);
                    }
                }
                break;
        }
    }
}