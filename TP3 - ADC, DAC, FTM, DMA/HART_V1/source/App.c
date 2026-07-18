/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <gpio.h>
#include <stdlib.h>
#include "board.h"
#include "uart.h"
#include "FSKDem.h"
#include "FSKMod.h"
#include "PIT.h"
#include "DAC.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

static uart_cfg_t uartConfig = {
	.IRQEnabled = true,
	.oddParity = true,
	.baudrate = 9600,
	.uartMode = TXRX};

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	initPIT();
	initFSKMod(); // No hace falta iniciar el NCO ni el DAC aparte de esto, con esta inicializacion ya es suficiente
	initFSKDemodulator();
	uartInit(0, uartConfig);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	static bool finished = false;

	while (!finished)
	{
		// Si recibimos algo por UART, lo enviamos por FSK
		if (uartIsRxMsg(0))
		{
			uint8_t c1 = 'c';
			uartReadMsg(0, &c1, 1);
			putArrayFSKMod(&c1, 1);
		}

		// Si recibimos algo por FSK, lo enviamos por UART
		if (isDataReadyHART())
		{
			uint8_t c2 = 'c';
			c2 = getNextValueHART();
			uartWriteMsg(0, &c2, 1);
		}
	}
}

/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
