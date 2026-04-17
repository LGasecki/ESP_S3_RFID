#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "ILI_9341_files/hardware.h"
#include "ILI_9341_files/tft_gfx.h"
#include "ILI_9341_files/touch_sensor.h"
#include "ILI_9341_files/keypad.h"



spi_device_handle_t spi_tft;
spi_device_handle_t spi_touch;



void app_main(void) {
    init_hardware();
    TFT_Init();
    Draw_Keypad();


    while (1) {
        uint16_t tx, ty;
        if (Touch_GetCoordinates(&tx, &ty)) {
            if(Handle_Login_Touch(tx,ty)){
                TFT_FillScreen(TFT_DARKCYAN);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}