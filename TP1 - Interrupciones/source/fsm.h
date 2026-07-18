/***************************************************************************//**
  @file     +Nombre del archivo (ej: template.h)+
  @brief    +Descripcion del archivo+
  @author   +Nombre del autor (ej: Salvador Allende)+
 ******************************************************************************/

#ifndef _FSM_H_
#define _FSM_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "user.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MAXUSUARIOS 10

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum FSMstate {ACCESS_MENU, ADMIN_MENU, CHECK_ADMIN_PASSWORD, ADMIN, BRIGHTNESS_MENU, BRIGHTNESS_ADJUST, INSERTID,
	CHECKID, INSERTPIN, CHECKPIN} fsmState_t;

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
void FSM ();


/*******************************************************************************
 ******************************************************************************/

#endif // _FSM_H_
