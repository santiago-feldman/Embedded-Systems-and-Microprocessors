/***************************************************************************/ /**
  @file     main.c
  @brief    FW main
  @author   Grupo 3
  ******************************************************************************/

#include <board.h>
#include <clock_config.h>
#include <diskio.h>
#include <ff.h>
#include <fsl_debug_console.h>
#include <fsl_sd.h>
#include <fsl_sysmpu.h>
#include <hardware.h>
#include <pin_mux.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "player.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void App_Init(void);
void App_Run(void);

int main(void)
{

	hw_Init();
	hw_DisableInterrupts();
	App_Init(); /* Program-specific setup */
	hw_EnableInterrupts();

	__FOREVER__
	App_Run(); /* Program-specific loop  */
}

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	playerInit();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	static char culero = '\0';

	//culero = GETCHAR();

//	switch (culero)
//	{
//	case 'p':
//		play();
//		break;
//	case 's':
//		stop();
//		break;
//	case 'w':
//		pause();
//		break;
//	case 'a':
//		prevSong();
//		break;
//	case 'd':
//		nextSong();
//		break;
//	case 't':
//		break;
//	default:
//		break;
//	}
	updatePlayer();
}

/*******************************************************************************
 *******************************************************************************
 LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
