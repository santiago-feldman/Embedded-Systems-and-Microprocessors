/***************************************************************************//**
  @file     led.c
  @brief    Led driver
  @author   Santiago Feldman
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "led.h"
#include "gpio.h"
#include "config.h"
#include "timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define CALL_TIME 5

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static state_t led1_state = OFF;
static state_t led2_state = OFF;
static state_t led3_state = OFF;
static uint8_t timed_led = 0;

static uint32_t callback_timer_id = 0;
static uint32_t timed_led_timer_id = 0;

/**
 * @brief Callback function to turn on the respective leds
 * @param void
 * @return void
 */
static void turn_led(void);

/**
 * @brief Callback function to turn off a led after a certain time
 * @param void
 * @return void
 */
static void shut_off(void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void led_init(state_t led1, state_t led2, state_t led3)
{
	// Initialize encoder
	gpioMode(STATUS0, OUTPUT);
	gpioMode(STATUS1, OUTPUT);

	// Initialize leds´ states
	led1_state = led1;
	led2_state = led2;
	led3_state = led3;

	// Initialize timer for turning on leds
	callback_timer_id = createTimer(CALL_TIME, &turn_led, PERIODIC);
	startTimer(callback_timer_id);

	// Initialize timer for timed leds
	timed_led_timer_id = createTimer(0, &shut_off, ONESHOT);
}

void set_leds(state_t led1, state_t led2, state_t led3)
{
	led1_state = led1;
	led2_state = led2;
	led3_state = led3;
}

void set_single_led(uint8_t led_num, state_t state)
{
	switch(led_num)
	{
		case 1:
			led1_state = state;
			break;
		case 2:
			led2_state = state;
			break;
		case 3:
			led3_state = state;
			break;
		default:
			break;
	}
}

void toggle_single_led(uint8_t led_num)
{
	switch(led_num)
	{
		case 1:
			led1_state = !led1_state;
			break;
		case 2:
			led2_state = !led2_state;
			break;
		case 3:
			led3_state = !led3_state;
			break;
		default:
			break;
	}
}


void turn_off_leds(void)
{
	led1_state = OFF;
	led2_state = OFF;
	led3_state = OFF;
}

void turn_on_leds(void)
{
	led1_state = ON;
	led2_state = ON;
	led3_state = ON;
}

void set_timed_led(uint8_t led_num, uint32_t time)
{

	timed_led = led_num;

	switch(led_num)
	{
		case 1:
			led1_state = ON;
			break;
		case 2:
			led2_state = ON;
			break;
		case 3:
			led3_state = ON;
			break;
		default:
			break;
	}

	// Turn on the corresponding led
	gpioWrite(STATUS0, (led_num & 0b10) >> 1);
	gpioWrite(STATUS1, led_num & 0b01);

	// Starts timer
	configTimerTime(timed_led_timer_id, time);
	startTimer(timed_led_timer_id);
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void turn_led(void)
{
	static uint8_t led = 1;
	static state_t state = OFF;

	bool status0 = (led & 0b10) >> 1;	// Logic value of encoder´s MSB
	bool status1 = led & 0b01;			// Logic value of encoder´s LSB

	switch(led)
	{
		case 1:
			state = led1_state;
			break;
		case 2:
			state = led2_state;
			break;
		case 3:
			state = led3_state;
			break;
		default:
			state = OFF;
			break;
	}

	// Turns on led
	if(state == ON)
	{
		gpioWrite(STATUS0, status0);
		gpioWrite(STATUS1, status1);
	}
	else
	{
		gpioWrite(STATUS0, 0);
		gpioWrite(STATUS1, 0);
	}

	// Updates the led to check
	led = (led == 3) ? 1 : led+1;
}

static void shut_off(void)
{
	set_single_led(timed_led, OFF);
}

/*******************************************************************************
 ******************************************************************************/
