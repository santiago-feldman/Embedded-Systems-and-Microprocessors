/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"
#include "board.h"

#include "uart.h"
#include "I2C.h"

#include "timer.h"

#include "fsm.h"      /*FSM engine (interprete)*/
#include "fsmtable.h" /*FSM Table*/

#include "UI/button.h"
#include "UI/encoder.h"
#include "UI/display.h"
#include "UI/eventQueue.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void delayLoop(uint32_t veces);

static uart_cfg_t uartConfig = {
    .IRQEnabled = true,
    .oddParity = true,
    .baudrate = 9600,
    .uartMode = TXRX};

STATE *p2state = NULL; /*Used to store FSM state*/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	initTimer_SYS();

    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioWrite(PIN_LED_BLUE, HIGH);
    gpioWrite(PIN_LED_RED, HIGH);
    gpioWrite(PIN_LED_GREEN, HIGH);


    uartInit(0, uartConfig);

    I2CInit(0, 0x11, 300000U);

    initButtons();
    initEncoder();
    initDisplay();

    p2state=FSM_GetInitState();// Inicializo la FSM con el estado inicial

}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	/*	DISPLAY TEST

	char * msg1 = "Hola, como andas?";
	char * msg2 = "Todo bn y vos?";
	char * msg3 = "123456789abcdefgh";
	char * msg4 = "sos un maquina";
	char msg5 = '\0';
	char * msg6 = "toma pinia";

	//delayLoop(90000000UL);

	DisplayWrite(msg1, 19, 0);
	DisplayWrite(&msg5, 1, 1);

	delayLoop(90000000UL);

	DisplayWrite(msg2, 15, 0);

	delayLoop(90000000UL);

	DisplayWrite(msg3, 18, 0);
	DisplayWrite(msg4, 15, 1);

	delayLoop(180000000UL);

	DisplayWrite(msg6, 11, 0);
	DisplayWrite(&msg5, 1, 1);

	delayLoop(90000000UL);

	*/

	event_t nextEvent = getNextEvent();
	if (nextEvent != None){

		p2state = fsm(p2state,nextEvent);      //Se lo paso a la maquina de estados
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*
static void delayLoop(uint32_t veces)
{
    while (veces--)
        ;
}
*/

/*******************************************************************************
 ******************************************************************************/
