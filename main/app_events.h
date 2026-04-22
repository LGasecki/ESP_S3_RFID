#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "ILI_9341_files/touch_sensor.h"

// Struktura zdarzenia RFID
struct RFIDEvent {
    char uid_hex[32];
};

// Globalne uchwyty do kolejek przesyłania wiadomości (zdefiniowane w main.cpp)
extern QueueHandle_t rfid_queue;
extern QueueHandle_t touch_queue;