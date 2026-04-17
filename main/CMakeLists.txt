idf_component_register(SRCS "main.c" 
                            "ILI_9341_files/tft_gfx.c" 
                            "ILI_9341_files/touch_sensor.c"
                            "ILI_9341_files/glcdfont.c"
                            "ILI_9341_files/keypad.c"
                            "ILI_9341_files/hardware.c"
                       INCLUDE_DIRS "." "ILI_9341_files")