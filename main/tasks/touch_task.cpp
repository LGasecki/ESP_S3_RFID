#include "touch_task.h"
#include "app_events.h"
#include "ILI_9341_files/touch_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void touch_task(void *pvParameters) {
    TouchSensor* ts = static_cast<TouchSensor*>(pvParameters);
    TouchEvent event; 
    bool is_pressed = false;
    
    while (1) {
        if (ts->getCoordinates(event.x, event.y)) {
            if (!is_pressed) {
                xQueueSend(touch_queue, &event, 0);
                is_pressed = true;
                
                vTaskDelay(pdMS_TO_TICKS(50)); 
            }
        } else {
            if (is_pressed) {
                is_pressed = false;
                vTaskDelay(pdMS_TO_TICKS(20)); 
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}