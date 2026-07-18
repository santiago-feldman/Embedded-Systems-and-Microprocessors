/***************************************************************************//**
  @file     timer.h
  @brief    timer driver
  @author   Grupo 3
 ******************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MAXTIMERS (50)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum TimerMode{
	ONESHOT,
	PERIODIC
} timerModes_t;

enum states{
	FREE,
	IDLE,
	RUNNING
};

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize timer driver
 * @param void
 * @return true if timer driver was initialized or false if not
 */
bool initTimer();

/**
 * @brief Initialize a new timer
 * @param time in ms when callback will be called
 * @param funcallback callback function when timer's count ends
 * @param mode ONESHOT when function must be called once or PERIODIC
 * @return id of timer where new timer was allocated. Return MAXTIMERS if allocation failed
 */
uint8_t createTimer(uint32_t time, void (*funcallback)(void), timerModes_t mode);

/**
 * @brief Config callback of timer of ID id
 * @param id timer's id which will be configured
 * @param callback to be replaced
 * @return void
 */

void configTimerCallback(uint8_t id, void (*funcallback)(void));

/**
 * @brief Config time of timer of ID id
 * @param id timer's id which will be configured
 * @param time to be modified
 * @return void
 */
void configTimerTime(uint8_t id, uint32_t time);

/**
 * @brief Config mode of timer of ID id
 * @param id timer's id which will be configured
 * @param new mode for timer
 * @return void
 */
void configTimerMode(uint8_t id, uint8_t mode);

/**
 * @brief Free timer id to allocate a new timer instead
 * @param id timer's id which will be detroyed
 * @return void
 */
void destroyTimer(uint8_t id);

/**
 * @brief reset timer
 * @param id timer's id which will be reseted
 * @return void
 */
void resetTimer(uint8_t id);

/**
 * @brief start timer
 * @param id timer's id which will be started
 * @return void
 */
void startTimer(uint8_t id);

/**
 * @brief stop timer
 * @param id timer's id which will be stopped
 * @return void
 */
void stopTimer(uint8_t id);

/**
 * @brief get current timer count
 * @param id timer's id which count will be returned
 * @return timer count in ms
 */
uint32_t getTime(uint8_t id);

/**
 * @brief get current timer state
 * @param id timer's id which state will be returned
 * @return state of the timer
 */
uint8_t getTimerState(uint8_t id);



void SysTick_Handler_TIMER(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _TIMER_H_
