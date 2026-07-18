/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "Queue.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

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

queuedata_t getNext(queue_t *queue)
{
    if (queue->tail == queue->front) // Si la cola esta vacia returna 0 por bobi
    {
        return 0;
    }
    else if (queue->isFull) // Si la cola estaba llena ya no lo esta
    {
        queue->isFull = false;
    }

    queuedata_t aux_event = queue->buffer[queue->front];                   // Guardo el dato en un auxiliar para correr la cabeza de la cola
    queue->front = (queue->front == BUFFER_SIZE - 1) ? 0 : queue->front + 1; // Corro la cabeza para apuntar al siguiente dato

    if (queue->tail == queue->front) // Si con esto se acabaron los datos, flag de cola vacia ):
        queue->isEmpty = true;

    return aux_event;
}

queuedata_t pop(queue_t *queue)
{
    if (queue->tail == queue->front) // Si la cola esta vacia returna 0 por bobi
    {
        return 0;
    }
    else if (queue->isFull) // Si la cola estaba llena ya no lo esta
    {
        queue->isFull = false;
    }

    queue->tail = (queue->tail == 0) ? BUFFER_SIZE - 1 : queue->tail - 1; // Corro la cola de la fila un lugar para atras dando lugar a otro posible dato

    if (queue->tail == queue->front) // Si habia un solo valor y se saco, se vacio la cola
        queue->isEmpty = true;

    return queue->buffer[queue->tail];
}

bool put(queue_t *queue, queuedata_t data)
{
    if (queue->tail == queue->front) // Si la cola estaba vacia, cambiar la flag porq se va a agregar un valor
    {
        queue->isEmpty = false;
    }
    else if (queue->isFull) // Si la cola esta llena no pongas nada
    {
        return false;
    }

    queue->buffer[queue->tail] = data;                                    // Coloco el dato
    queue->tail = (queue->tail == BUFFER_SIZE - 1) ? 0 : queue->tail + 1; // Corro la cola un lugar

    if ((queue->tail == (queue->front - 1)) || (queue->front == 0 && queue->tail == (BUFFER_SIZE - 1))) // Si la cola esta un lugar antes que la cabeza significa que esta llena la queue
    {
        queue->isFull = true;
    }

    return true;
}

uint32_t getNextArray(queue_t *queue, queuedata_t *data, uint32_t dataAmount)
{
    if (queue->isEmpty)
    {
        return 0;
    }

    uint32_t i = 0;
    for (i = 0; i < dataAmount && !queue->isEmpty; i++)
    {
        data[i] = getNext(queue);
    }
    return (i + 1);
}

uint32_t putArray(queue_t *queue, queuedata_t *data, uint32_t dataAmount)
{
    if (queue->isFull)
    {
        return 0;
    }

    uint32_t i = 0;
    for (; i < dataAmount && !queue->isFull; i++)
    {
        put(queue, data[i]);
    }
    return (i + 1);
}

uint32_t getFillLevel(queue_t *queue)
{
    int diff = queue->tail - queue->front;
    return diff < 0 ? diff + BUFFER_SIZE : diff;
}

queuedata_t readValue(queue_t *queue, uint16_t index){
	queuedata_t value = 0;

	if(getFillLevel(queue) > index){
		value =	queue->buffer[queue->front-index];
	}

	return value;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
