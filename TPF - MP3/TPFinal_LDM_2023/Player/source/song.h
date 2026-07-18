/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.h)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

#ifndef _SONG_H_
#define _SONG_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define BUFFER_SIZE 2000
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef uint8_t queuedata_t; // Queuedata podria ser en principio 8 bits. Cambiarlo despues de ver que funciona

typedef struct _Song
{
  char *filename;
  char *path;
} song_t;

typedef struct _SongInfo
{
  char *title;
  char *artist;
  char *date;

} song_info_t;
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
song_t *
newSong(const char *path, const char *filename);
void destroySong(song_t *song);

/*******************************************************************************
 ******************************************************************************/

#endif // _QUEUE_H_
