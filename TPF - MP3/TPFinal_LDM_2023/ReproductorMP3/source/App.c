/***************************************************************************/ /**
  @file     main.c
  @brief    FW main
  @author   Grupo 3
  ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "App.h"
#include "hardware.h"

#include "player.h"

#include "fsm.h"
#include "fsmtable.h" /*FSM Table*/

#include "gpio.h"
// #include "I2C.h"
#include "board.h"

#include "button.h"
#include "encoder.h"
#include "display.h"
#include "vumeter.h"
#include "equalizer.h"
#include "deepSleep.h"
#include "eventQueue.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
STATE *p2state = NULL; /*Used to store FSM state*/
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	playerInit();
	initTimer_SYS();

	gpioMode(PIN_LED_BLUE, OUTPUT);
	gpioMode(PIN_LED_RED, OUTPUT);
	gpioMode(PIN_LED_GREEN, OUTPUT);
	gpioWrite(PIN_LED_BLUE, HIGH);
	gpioWrite(PIN_LED_RED, HIGH);
	gpioWrite(PIN_LED_GREEN, HIGH);

	I2CInit(0, 0x11, 300000U);

	initButtons();
	initEncoder();
	initDisplay();
	initVumeter();
	initEqualizer();

	deepSleep_init();

	p2state = FSM_GetInitState(); // Inicializo la FSM con el estado inicial
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	static event_t nextEvent;

	nextEvent = getNextEvent();
	if (nextEvent != None)
	{

		p2state = fsm(p2state, nextEvent); // Se lo paso a la maquina de estados
	}
	updatePlayer();
}

/*******************************************************************************
 *******************************************************************************
 LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
