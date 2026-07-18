/***************************************************************************/ /**
   @file     FTM.h
   @brief    Simple FTM services
   @author   Grupo 3
  ******************************************************************************/

#ifndef _FTM_H_
#define _FTM_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"
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
 * @brief Configures the specified pin to behave either as an input or an output
 * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
 * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 * @return Registration succeed
 */
void PWM_Init(void);

void PWM_SetDC(float DC);
float PWM_GetDC();

void PWM_SetTickPerPeriod(uint16_t);
uint16_t PWM_GetTickPerPeriod();

void PWM_GenWaveform(uint16_t *waveform, uint32_t wave_length, uint32_t waveTable_offset);
void PWM_SetWaveformOffset(uint32_t waveTable_offset);
uint32_t PWM_GetWaveformOffset();
/*******************************************************************************
 ******************************************************************************/

#endif // _FTM_H_
