
/***************************************************************************/ /**
   @file     equalizer.h
   @brief    Equalizer driver
   @author   Grupo 3
  ******************************************************************************/

#ifndef _EQUALIZER_H_
#define _EQUALIZER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "biquad.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DECREASE (0)
#define INCREASE (1)


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum{rock, pop};

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
 */
void initEqualizer();

void changeGain(uint8_t filterID, bool direction);

float processEqualizer(float sample);

void changePreset(uint8_t preset);

/* TODO: POR AHI FALTA HACER UNA FUNCION PARA BORRAR LOS DATOS DEL EQUALIZADOR DE LA CANCION ANTERIOR POR UN TEMA DE CAUSALIDAD*/

/*******************************************************************************
 ******************************************************************************/

#endif // _EQUALIZER_H_
