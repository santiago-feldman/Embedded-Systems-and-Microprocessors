/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"
#include "board.h"
#include "equalizer.h"
#include <stdlib.h>
#include <math.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define FREQ (34)
#define PERIODS (3.0f)
#define AMPLITUDE (1.0f)
#define SAMPLERATE (44100)
#define SIZE (10000)

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delay();
static int generateWave(float* buff, float freq);

static float signal[SIZE];
static float output[SIZE];
static int reserved = 0;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	initEqualizer();

	reserved = generateWave(signal, FREQ);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{

	for(int i = 0; i < reserved; i++){
		output[i] = processEqualizer(signal[i]);
	}

	changePreset(rock);
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/

static int generateWave(float buff[], float freq){

	float duration = PERIODS / freq; // Two periods

	int num_samples = (int)(duration * SAMPLERATE);
	float time_step = 1.0 / SAMPLERATE;

	for (int i = 0; i < num_samples; i++) {
		float t = i * time_step;
		buff[i] = (float) AMPLITUDE * sin(2.0 * PI * freq * t);
	}

	return num_samples;
}

static void delay(){
	uint32_t time = 4000000;
	while(time--);
}
