/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "FSKMod.h"
#include "NCO.h"
#include "PIT.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void Mod_PISR(void);
int intToBinaryHART(int num);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static queue_t FSKModulation_queue = {{0}, 0, 0, false, true};
static uint8_t actualIndex = 0;
static uint16_t actualModWord;
static bool isSendingWord = false;
static uint8_t parityCalc = 0;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void initFSKMod(void)
{
  DAC_Init();
  uint8_t id = createTimer(833, Mod_PISR);
  initNCO();
  startTimer(id);
  startNCO();
}

void putArrayFSKMod(queuedata_t *data, uint32_t dataAmount)
{
  putArray(&FSKModulation_queue, data, dataAmount);
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// la interrupcion solo deberia setear el M en cada PIT
void Mod_PISR(void)
{
  // If i'm not sending any word and there is one ready, I charge it on my queue
  if (isSendingWord == false && (getFillLevel(&FSKModulation_queue) != 0))
  {
    isSendingWord = true;
    actualModWord = getNext(&FSKModulation_queue);
    actualIndex = 0;
    parityCalc = 0;
  }

  // If I started sending a word I'll change M accordingly
  if (isSendingWord == true)
  {
    switch (actualIndex)
    {
    case 0:
      setM(11); // Start bit
      break;
    case 10:
      setM(6); // Stop bit
      isSendingWord = false;
      break;
    case 9:
    {
      if (parityCalc == 0)
      {
        setM(11);
      }
      else
      {
        setM(6);
      }
      break;
    }
    default:
    {
      int bit = (actualModWord >> (8 - actualIndex)) & 1;
      if (bit == 0)
      {
        setM(11);
      }
      else
      {
        parityCalc ^= 1;
        setM(6);
      }
      break;
    }
    }
    actualIndex++;
  }
}
