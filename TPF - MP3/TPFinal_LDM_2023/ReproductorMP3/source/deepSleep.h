/***************************************************************************/ /**
   @file		deepSleep.h
   @brief 		Put MCU into low power mode and wake up
   @author		Grupo 3
  ******************************************************************************/

#ifndef _DEEPSLEEP_H_
#define _DEEPSLEEP_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Init peak notch filters
 * @return void
 */
void deepSleep_init(void);

/**
 * @brief Reset filters to avoiding causality problems. Call it every time you want to restart processing
 * @return void
 */
void deepSleep(void);
/**
 * @brief Compute cascade filters to current sample
 * @param x The next sample to be processed
 * @return Returns the sample processed
 */

/**
 * @brief Change the dB gain/atten of filters. Recompute the filters coeffiencients.
 * @param value target value in dB of filters gain
 * @return void
 */

/**
 * @brief Return the current gain of the filter
 * @param filterID filter to get the gain
 * @return gain in dB of the filter
 */

/*******************************************************************************
 ******************************************************************************/

#endif // _DEEPSLEEP_H_
