/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.h)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

#ifndef _EVENTQUEUE_H_
#define _EVENTQUEUE_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum event_type
{
  None,
  EncoderLeft,
  EncoderRight,
  EncoderClick,
  ButtonCross,
  ButtonPoint,
  ButtonLine,
  AutoNextSong,
  SDCardRemoved,
  SDCardInserted,
} event_t;

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

event_t getNextEvent(void);
void putEvent(event_t event);
event_t popEvent(void);
/*******************************************************************************
 ******************************************************************************/

#endif // _EVENTQUEUE_H_
