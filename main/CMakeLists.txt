idf_component_register(SRCS "main.cpp" 
                            "ILI_9341_files/tft_gfx.cpp" 
                            "ILI_9341_files/touch_sensor.cpp"
                            "ILI_9341_files/glcdfont.c"
                            "ILI_9341_files/keypad.cpp"
                            "ILI_9341_files/hardware.cpp"
                       INCLUDE_DIRS "." "ILI_9341_files")