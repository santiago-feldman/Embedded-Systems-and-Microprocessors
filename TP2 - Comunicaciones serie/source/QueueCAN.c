/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "QueueCAN.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
static canqueuedata_t nullItem = {0, 0, {0}};
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

canqueuedata_t cangetNext(canqueue_t *queue)
{
    if (queue->tail == queue->head) // Si la cola esta vacia returna 0 por bobi
    {
        return nullItem;
    }
    else if (queue->isFull) // Si la cola estaba llena ya no lo esta
    {
        queue->isFull = false;
    }

    canqueuedata_t aux_event = queue->buffer[queue->head];                   // Guardo el dato en un auxiliar para correr la cabeza de la cola
    queue->head = (queue->head == CANBUFFER_SIZE - 1) ? 0 : queue->head + 1; // Corro la cabeza para apuntar al siguiente dato

    if (queue->tail == queue->head) // Si con esto se acabaron los datos, flag de cola vacia ):
        queue->isEmpty = true;

    return aux_event;
}

canqueuedata_t canpop(canqueue_t *queue)
{
    if (queue->tail == queue->head) // Si la cola esta vacia returna 0 por bobi
    {
        return nullItem;
    }
    else if (queue->isFull) // Si la cola estaba llena ya no lo esta
    {
        queue->isFull = false;
    }

    queue->tail = (queue->tail == 0) ? CANBUFFER_SIZE - 1 : queue->tail - 1; // Corro la cola de la fila un lugar para atras dando lugar a otro posible dato

    if (queue->tail == queue->head) // Si habia un solo valor y se saco, se vacio la cola
        queue->isEmpty = true;

    return queue->buffer[queue->tail];
}

bool canput(canqueue_t *queue, canqueuedata_t data)
{
    if (queue->tail == queue->head) // Si la cola estaba vacia, cambiar la flag porq se va a agregar un valor
    {
        queue->isEmpty = false;
    }
    else if (queue->isFull) // Si la cola esta llena no pongas nada
    {
        return false;
    }

    queue->buffer[queue->tail] = data;                                    // Coloco el dato
    queue->tail = (queue->tail == CANBUFFER_SIZE - 1) ? 0 : queue->tail + 1; // Corro la cola un lugar

    if ((queue->tail == (queue->head - 1)) || (queue->head == 0 && queue->tail == (CANBUFFER_SIZE - 1))) // Si la cola esta un lugar antes que la cabeza significa que esta llena la queue
    {
        queue->isFull = true;
    }

    return true;
}

uint32_t cangetNextArray(canqueue_t *queue, canqueuedata_t *data, uint32_t dataAmount)
{
    if (queue->isEmpty)
    {
        return 0;
    }

    uint32_t i = 0;
    for (i = 0; i < dataAmount && !queue->isEmpty; i++)
    {
        data[i] = cangetNext(queue);
    }
    return i;
}

uint32_t canputArray(canqueue_t *queue, canqueuedata_t *data, uint32_t dataAmount)
{
    if (queue->isFull)
    {
        return 0;
    }

    uint32_t i = 0;
    for (; i < dataAmount && !queue->isFull; i++)
    {
    	canput(queue, data[i]);
    }
    return i;
}

uint32_t cangetFillLevel(canqueue_t *queue)
{
    int diff = queue->tail - queue->head;
    return diff < 0 ? diff + CANBUFFER_SIZE : diff;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
