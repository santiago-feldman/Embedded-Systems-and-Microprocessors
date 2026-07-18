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

#define MAXTIMERS_SYS (20)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum TimerMode_SYS{
	SYS_ONESHOT,
	SYS_PERIODIC
} timerModesSYS_t;

enum states_SYS{
	SYS_FREE,
	SYS_IDLE,
	SYS_RUNNING
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
bool initTimer_SYS();

/**
 * @brief Initialize a new timer
 * @param time in ms when callback will be called
 * @param funcallback callback function when timer's count ends
 * @param mode ONESHOT when function must be called once or PERIODIC
 * @return id of timer where new timer was allocated. Return MAXTIMERS if allocation failed
 */
uint8_t createTimer_SYS(uint32_t time, void (*funcallback)(void), timerModesSYS_t mode);

/**
 * @brief Config callback of timer of ID id
 * @param id timer's id which will be configured
 * @param callback to be replaced
 * @return void
 */

void configTimerCallback_SYS(uint8_t id, void (*funcallback)(void));

/**
 * @brief Config time of timer of ID id
 * @param id timer's id which will be configured
 * @param time to be modified
 * @return void
 */
void configTimerTime_SYS(uint8_t id, uint32_t time);

/**
 * @brief Config mode of timer of ID id
 * @param id timer's id which will be configured
 * @param new mode for timer
 * @return void
 */
void configTimerMode_SYS(uint8_t id, uint8_t mode);

/**
 * @brief Free timer id to allocate a new timer instead
 * @param id timer's id which will be detroyed
 * @return void
 */
void destroyTimer_SYS(uint8_t id);

/**
 * @brief reset timer
 * @param id timer's id which will be reseted
 * @return void
 */
void resetTimer_SYS(uint8_t id);

/**
 * @brief start timer
 * @param id timer's id which will be started
 * @return void
 */
void startTimer_SYS(uint8_t id);

/**
 * @brief stop timer
 * @param id timer's id which will be stopped
 * @return void
 */
void stopTimer_SYS(uint8_t id);

/**
 * @brief get current timer count
 * @param id timer's id which count will be returned
 * @return timer count in ms
 */
uint32_t getTime_SYS(uint8_t id);

/**
 * @brief get current timer state
 * @param id timer's id which state will be returned
 * @return state of the timer
 */
uint8_t getTimerState_SYS(uint8_t id);

/*******************************************************************************
 ******************************************************************************/

#endif // _TIMER_H_
