/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.h)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

#ifndef _FIRQUEUE_H_
#define _FIRQUEUE_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define FIR_ORDER 21
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef float FIRqueuedata_t;
typedef struct FIRQueue
{
  FIRqueuedata_t buffer[FIR_ORDER];
  uint16_t front;
} FIRqueue_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// Add a new element at the end of the queue
bool putFIR(FIRqueue_t *queue, FIRqueuedata_t data);

/**
 * @brief Return the i-th value of the queue, where 0 is the first position in the queue corresponding to the front of the queue.
 * @param queue queue to return value
 * @param index taps from the front to value read
 * @return value of the front + tap wrapped around
 */
FIRqueuedata_t readValueFIR(FIRqueue_t *queue, uint16_t tap);

/*******************************************************************************
 ******************************************************************************/

#endif // _FIRQUEUE_H_
