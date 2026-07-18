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

#define BANDS (8)

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
void computeFilters(float *in, float *out, uint32_t blockSize);

/**
 * @brief Change the dB gain/atten of filters. Recompute the filters coeffiencients.
 * @param value target value in dB of filters gain
 * @return void
 */
void setGain(float value[]);

/**
 * @brief Return the current gain of the filter
 * @param filterID filter to get the gain
 * @return gain in dB of the filter
 */
float getGain(int8_t filterID);

/*******************************************************************************
 ******************************************************************************/

#endif // _BIQUAD_H_
