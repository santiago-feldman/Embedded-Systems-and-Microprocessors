/***************************************************************************//**
  @file     +Nombre del archivo (ej: template.h)+
  @brief    +Descripcion del archivo+
  @author   +Nombre del autor (ej: Salvador Allende)+
 ******************************************************************************/

#ifndef _PRIVATESTDIO_H_
#define _PRIVATESTDIO_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/



/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief TODO: completar descripcion
 * @param param1 Descripcion parametro 1
 * @param param2 Descripcion parametro 2
 * @return Descripcion valor que devuelve
*/
// +ej: char lcd_goto (int fil, int col);+


//Imprime el volumen en el buffer para mandarlo a DisplayWrite
void volNumPrinter (char * buffer, uint8_t numVol);

//Imprime la banda seleccionada en el buffer para mandarla a DisplayWrite
void eqStringPrinter (char * buffer, char * eqBand);

//Imprime la parte inferior de la pantalla de reproduccion para mandarla a displayWrite
void repDisplayPrinter ( char * buffer, bool pauseOrPlay, uint8_t numVol, char * eqBand);

void bufferClean (char * buffer); //Borra los primeros 16 caracteres del buffer

/*******************************************************************************
 ******************************************************************************/

#endif // _PRIVATESTDIO_H_
