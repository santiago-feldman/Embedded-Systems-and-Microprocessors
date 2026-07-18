/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "equalizer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PEAKGAIN (12) //in dB
#define NOTCHGAIN (-12) //in dB
#define GAINSTEP (3) //in dB

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

static int8_t currentGains[BANDS];

static int8_t* presetSelected;
static int8_t rockConfig[BANDS] = {5, 4, 2, 0, -1, 1, 3, 4};
static int8_t popConfig[BANDS] = {-2, -1, 2, 4, 4, 2, -1, -2};
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
            GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void initEqualizer()
{
	initFilters();
}

void changeGain(uint8_t filterID, bool direction){

	int currGain = (int)getGain(filterID);

	if(direction == INCREASE && currGain+GAINSTEP <= PEAKGAIN){
		changeGain(filterID, currGain+GAINSTEP);
	}
	else if(direction == DECREASE && currGain-GAINSTEP >= NOTCHGAIN){
		changeGain(filterID, currGain-GAINSTEP);
	}
}

float processEqualizer(float sample){
	return computeFilters(sample);
}

void changePreset(uint8_t preset){

	switch(preset){
		case rock:
			presetSelected = rockConfig;
			break;
		case pop:
			presetSelected = popConfig;
			break;
	}

	for(int i = 0; i < BANDS; i++){
		currentGains[i] = presetSelected[i];
	}

	setGain(currentGains);
}
/*******************************************************************************
 *******************************************************************************
            LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

