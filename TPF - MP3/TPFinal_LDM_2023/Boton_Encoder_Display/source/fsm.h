/* 
 * fsm.h
 *
 *  Created on: 28/07/2014
 *      Author: Daniel Jacoby (template)
 */


#ifndef FSM_H_
#define FSM_H_


#define FIN_TABLA 0xFF

#include "stdlib.h"
#include "UI/eventQueue.h"


typedef struct state_diagram_edge STATE;

struct state_diagram_edge

{
	event_t  evento;
	STATE *proximo_estado;
	void (*p_rut_accion)(void);
};

// Interfaz
STATE* fsm(STATE *p_tabla_estado, event_t evento_actual);

#endif /* FSM_H_ */
