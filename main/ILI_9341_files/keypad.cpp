/**
# Project: ESP_S3_RFID
#
# Module: keypad.cpp
# Author: Łukasz Gąsecki
# Description: Renders the numeric keypad interface on the TFT display and processes user touch events for PIN entry, visual feedback, and authentication logic.
# 
*/

#include "keypad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define KEY_W 50
#define KEY_H 36
#define KEY_GAP 8
#define KEY_START_X 20
#define KEY_START_Y 20
#define UPDATE_PIN_X 30
#define UPDATE_PIN_Y 2

Keypad::Keypad(TFT_GFX& display, TouchSensor& touch) 
    : tft(display), touchSensor(touch), USER_PIN("1234") {
    pinBuffer[0] = '\0';
}

void Keypad::draw() {
    tft.fillScreen(TFT_BLACK);    
    tft.fillRect(KEY_START_X, KEY_START_Y +(KEY_H + KEY_GAP)* 4 , KEY_START_X +(KEY_W + KEY_GAP)* 3, 2, TFT_WHITE); 
    
    const char* labels[12] = {"1","2","3", "4","5","6", "7","8","9", "C","0","OK"};
    
    for(int i=0; i<12; i++) {
        int row = i / 3;
        int col = i % 3;
        
        uint16_t x = KEY_START_X + col * (KEY_W + KEY_GAP);
        uint16_t y = KEY_START_Y + row * (KEY_H + KEY_GAP);
        
        uint16_t color = (i == 9) ? TFT_RED : (i == 11) ? TFT_GREEN : TFT_BLUE;

        tft.fillRect(x, y, KEY_W, KEY_H, color);
        tft.print(x + 20, y + 10, labels[i], TFT_WHITE, color, 2);
    }
}

void Keypad::updatePinDisplay() {
    char mask[5] = "    ";
    int len = strlen(pinBuffer);
    for(int i=0; i<len; i++) mask[i] = '*'; 
    
    tft.fillRect(UPDATE_PIN_X, UPDATE_PIN_Y, 100, 16, TFT_BLACK); 
    tft.print(UPDATE_PIN_X + 20, UPDATE_PIN_Y, mask, TFT_YELLOW, TFT_BLACK, 2);
}

bool Keypad::handleLoginTouch(uint16_t tx, uint16_t ty) {
    char keys[12] = {'1','2','3', '4','5','6', '7','8','9', 'C','0','K'}; 
    bool loginSuccess = false;

    for(int i=0; i<12; i++) {
        int row = i / 3;
        int col = i % 3;
        uint16_t bx = KEY_START_X + col * (KEY_W + KEY_GAP);
        uint16_t by = KEY_START_Y + row * (KEY_H + KEY_GAP);
        
        if (TouchSensor::isBtnPressed(tx, ty, bx, by, KEY_W, KEY_H)) {
            char key = keys[i];
            
            tft.fillRect(bx, by, KEY_W, KEY_H, TFT_WHITE);
            vTaskDelay(pdMS_TO_TICKS(50));
            
            uint16_t color = (i == 9) ? TFT_RED : (i == 11) ? TFT_GREEN : TFT_BLUE;           
            
            tft.fillRect(bx, by, KEY_W, KEY_H, color);
            char label[3] = {(char)(key == 'K' ? 'O' : key), (char)(key == 'K' ? 'K' : 0), 0};
            tft.print(bx + 20, by + 10, label, TFT_WHITE, color, 2);

            int len = strlen(pinBuffer);
            
            if (key == 'C') { 
                if (len > 0) pinBuffer[len-1] = 0;
            } 
            else if (key == 'K') { 
                if (strcmp(pinBuffer, USER_PIN) == 0) {
                    tft.fillScreen(TFT_BLACK);
                    tft.print(80, 100, "LOGOWANIE...", TFT_GREEN, TFT_BLACK, 2);
                    vTaskDelay(pdMS_TO_TICKS(800)); 
                    
                    tft.fillScreen(TFT_BLACK); 
                    pinBuffer[0] = 0; 
                    loginSuccess = true; 
                } else {
                    tft.fillRect(38, 210, 136, 20, TFT_YELLOW); 
                    tft.print(42, 214, "BLEDNY KOD!", TFT_RED, TFT_YELLOW, 2);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    tft.fillRect(38, 210, 136, 20, TFT_BLACK); 
                    pinBuffer[0] = 0; 
                }
            } 
            else if (len < 4) { 
                pinBuffer[len] = key;
                pinBuffer[len+1] = 0;
            }
            if(!loginSuccess) updatePinDisplay();
            
            while(touchSensor.isPressed()) {
                vTaskDelay(pdMS_TO_TICKS(120));
            }
            return loginSuccess; 
        }
    }
    return false;
}