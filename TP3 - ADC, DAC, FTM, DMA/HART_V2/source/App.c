/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <gpio.h>
#include "board.h"
#include "uart.h"
#include "FSKDem.h"
#include "FSKMod.h"
#include "PIT.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
uart_cfg_t config = {
    true,
    true,
    1200,
    TXRX};

void delay(uint32_t a)
{
  while (a)
  {
    a--;
  }
}
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
  initPIT();
  uartInit(0, config);
  initFSKMod();
  initFSKDemodulator();
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
