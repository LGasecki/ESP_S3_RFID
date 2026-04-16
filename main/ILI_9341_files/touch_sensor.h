#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

// Struktura przechowujaca wspolrzedne
typedef struct {
    uint16_t x;
    uint16_t y;
} Point;

// Inicjalizacja
void Touch_Init(void);

// Sprawdza, czy ekran jest nacisniety (na podstawie pinu IRQ)
bool Touch_IsPressed(void);

// Pobiera surowe dane z przetwornika (0-4095)
bool Touch_GetRaw(uint16_t *x, uint16_t *y);

// Pobiera wspolrzedne przeliczone na piksele ekranu
bool Touch_GetCoordinates(uint16_t *x_pos, uint16_t *y_pos);

// Sprawdz czy jest klikniety przycisk
bool Is_Btn_Pressed(uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

#endif