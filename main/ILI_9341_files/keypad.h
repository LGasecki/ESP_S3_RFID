/* * File:   keypad.h
 * Author: User
 *
 * Obs?uga ekranu logowania i klawiatury numerycznej
 */

#ifndef KEYPAD_H
#define	KEYPAD_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Rysuje ca?? klawiatur? na ekranie
void Draw_Keypad(void);

// Aktualizuje wy?wietlanie gwiazdek (****)
void Update_Pin_Display(void);

// Obs?uguje dotyk na ekranie logowania
// Zwraca: true - je?li kod poprawny (zalogowano), false - w przeciwnym razie
bool Handle_Login_Touch(uint16_t tx, uint16_t ty);

#endif	/* KEYPAD_H */