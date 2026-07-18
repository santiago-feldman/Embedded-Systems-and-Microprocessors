/***************************************************************************/ /**
   @file		biquad.h
   @brief 		Generate cascade peak notch filters to be used as equalizers
   @author		Grupo 3
  ******************************************************************************/

#ifndef _BIQUAD_H_
#define _BIQUAD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PI (3.141592654f) // PI value
#define BANDS (8)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {b34Hz, b80Hz, b190Hz, b450Hz, b1K1Hz, b2K5Hz, b6KHz, b14K2Hz};

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
void initFilters();

/**
 * @brief Reset filters to avoiding causality problems. Call it every time you want to restart processing
 * @return void
 */
void resetFilters();

/**
 * @brief Compute cascade filters to current sample
 * @param x The next sample to be processed
 * @return Returns the sample processed
 */
float computeFilters(float x);

/**
 * @brief Change the dB gain/atten of filters. Recompute the filters coeffiencients.
 * @param value target value in dB of filters gain
 * @return void
 */
void setGain(int8_t value[]);

/**
 * @brief Return the current gain of the filter
 * @param filterID filter to get the gain
 * @return gain in dB of the filter
 */
float getGain(int8_t filterID);

/*******************************************************************************
 ******************************************************************************/

#endif // _BIQUAD_H_
