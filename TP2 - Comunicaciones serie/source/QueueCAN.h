/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.h)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

#ifndef _QUEUECAN_H_
#define _QUEUECAN_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "canspi.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define CANBUFFER_SIZE 20
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef CANMsg_t canqueuedata_t;
typedef struct canQueue
{
  canqueuedata_t buffer[CANBUFFER_SIZE + 1];
  uint16_t head;
  uint16_t tail;
  bool isFull;
  bool isEmpty;
} canqueue_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief FSM Principal del proyecto
 * @param parametro utilizado dentro de la FSM para guardar los datos que se van escribiendo
 */

canqueuedata_t cangetNext(canqueue_t *queue);
uint32_t cangetNextArray(canqueue_t *queue, canqueuedata_t *data, uint32_t dataAmount);
bool canput(canqueue_t *queue, canqueuedata_t data);
uint32_t canputArray(canqueue_t *queue, canqueuedata_t *data, uint32_t dataAmount);
canqueuedata_t canpop(canqueue_t *queue);
uint32_t cangetFillLevel(canqueue_t *queue);

/*******************************************************************************
 ******************************************************************************/

#endif // _QUEUECAN_H_
