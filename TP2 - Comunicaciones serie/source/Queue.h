/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.h)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

#ifndef _QUEUE_H_
#define _QUEUE_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define BUFFER_SIZE 1000
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef uint8_t queuedata_t;
typedef struct Queue
{
  queuedata_t buffer[BUFFER_SIZE + 1];
  uint16_t head;
  uint16_t tail;
  bool isFull;
  bool isEmpty;
} queue_t;

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

queuedata_t getNext(queue_t *queue);
uint32_t getNextArray(queue_t *queue, queuedata_t *data, uint32_t dataAmount);
bool put(queue_t *queue, queuedata_t data);
uint32_t putArray(queue_t *queue, queuedata_t *data, uint32_t dataAmount);
queuedata_t pop(queue_t *queue);
uint32_t getFillLevel(queue_t *queue);

/*******************************************************************************
 ******************************************************************************/

#endif // _QUEUE_H_
