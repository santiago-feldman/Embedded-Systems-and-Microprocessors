#ifndef _LED_Matrix_
#define _LED_Matrix_

#include <stdint.h>

/*
 * Sobre la biblioteca: Para utilizar la matriz primero debe seleccionarse un color para el LED. Por defecto
 * los LEDs comienzan sin color. Luego, debe prenderse el respectivo LED o, en su defecto, utilizar la funcion
 * turnOnAll para encender todos.
 */

#define ROWS (8)
#define COLS (8)
#define MINBRIGHTNESS (1)
#define MAXBRIGHTNESS (100)

#define RED (0xFF0000)
#define BLUE (0x0000FF)
#define GREEN (0x00FF00)
#define WHITE (0xFFFFFF)
#define ORANGE (0xFF6E00)
#define PINK (0xE60066)
#define PURPLE (0xFF99FF)
#define CYAN (0x00FFFF)
#define YELLOW (0xFFFF00)
#define BLACK (0x000000)

typedef union
{
	uint32_t hex;
	struct
	{
		uint8_t b, g, r, bright;
	};
} color_t;

/**
 * @brief Initilze led matrix
 * @return void
 */
void initLEDMatrix();

/**
 * @brief Turn on the specified led. If the led already is turned on, does nothing.
 * @param row The row where the led is
 * @param col The column where the led is
 * @return void
 */
void turnOn(uint8_t row, uint8_t col);

/**
 * @brief Turn on all leds. If the led already is turned on, does nothing.
 * @return void
 */
void turnOnAll();

/**
 * @brief Turn off the specified led. If the led already is turned off, does nothing.
 * @param row The row where the led is
 * @param col The column where the led is
 * @return void
 */
void turnOff(uint8_t row, uint8_t col);

/**
 * @brief Change leds brightness
 * @param percentage The percentage from 1 to 100 that indicates desire brightness
 * @return void
 */
void changeBrightness(uint8_t percentage);

/**
 * @brief Change the specified led color
 * @param row The row where the led is
 * @param col The column where the led is
 * @param color color to set
 * @return void
 */
void changeColor(uint8_t row, uint8_t col, color_t color);

/**
 * @brief Start led to blink with a period twice specified by inverval in ms
 * @param row The row where the led is
 * @param col The column where the led is
 * @param interval time interval between led state change
 * @return void
 */
void blink(uint8_t row, uint8_t col, float ms);

/**
 * @brief Stop led to blink
 * @param row The row where the led is
 * @param col The column where the led is
 * @return void
 */
void stopBlink(uint8_t row, uint8_t col);

#endif // _LED_Matrix_
