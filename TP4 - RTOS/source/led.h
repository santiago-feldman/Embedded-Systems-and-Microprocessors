/***************************************************************************//**
  @file     led.h
  @brief    LEDs definitions
  @author   Santiago Feldman
 ******************************************************************************/

#ifndef _LED_H_
#define _LED_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

typedef enum{
	OFF,
	ON
} state_t;

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes led pins and timer
 * @param led1 state of initialization of LED 1 (ON/OFF)
 * @param led2 state of initialization of LED 2 (ON/OFF)
 * @param led3 state of initialization of LED 3 (ON/OFF)
 * @return void
 */
void led_init(state_t led1, state_t led2, state_t led3);

/**
 * @brief Configures the state of the leds
 * @param led1 state of LED 1 (ON/OFF)
 * @param led2 state of LED 2 (ON/OFF)
 * @param led3 state of LED 3 (ON/OFF)
 * @return void
 */
void set_leds(state_t led1, state_t led2, state_t led3);

/**
 * @brief Configures the state of a single led
 * @param led_num number of the corresponding led
 * @param state state of the led (ON/OFF)
 * @return void
 */
void set_single_led(uint8_t led_num, state_t state);


void toggle_single_led(uint8_t led_num);
/**
 * @brief Turns off all the leds
 * @param void
 * @return void
 */
void turn_off_leds(void);

/**
 * @brief Turns on all the leds
 * @param void
 * @return void
 */
void turn_on_leds(void);

/**
 * @brief Turns on all the leds
 * @param void
 * @return void
 */
void set_timed_led(uint8_t led_num, uint32_t time);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/

#endif // _LED_H_
