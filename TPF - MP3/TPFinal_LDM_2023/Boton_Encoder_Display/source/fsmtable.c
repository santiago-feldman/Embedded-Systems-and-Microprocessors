#include <stdio.h>
#include <stdint.h>
#include "fsm.h"
#include "fsmtable.h"
#include "UI/display.h"
#include "UI/privateSTDIO.h"

//En estas variables voy a guardar lo seleccionado por la UI en cada caso, y en cada rutina
// de accion al cambiar a alguna de estas voy a llamar a una funcion de ustedes que tiene que responder
// haciendo los cambios necesarios, y si lo precisasen guardando el numero al cual cambió la variable
uint8_t volumeLevel = 15;
uint8_t songSelected = 1;
bool pause = true;
uint8_t eqLevel = 0;
#define EQCANT 5	//TODO: Esto deberia estar en el header del eq
char * eqBands [EQCANT]= {"Flat", "Jazz", "Rock", "Classic", "Atun"};//TODO: Esto deberia estar en el header del eq



/*Foward Declarations*/

extern STATE st_off[];
extern STATE st_rep[];
extern STATE st_vol[];
extern STATE st_eq[];



// prototipos

static void do_nothing(void);
static void goToRepFromOff(void);
static void goToRepFromVol(void);
static void goToRepFromEq(void);
static void playPauseSong(void);
static void changeSongLeft(void);
static void changeSongRight(void);
static void goToVolume(void);
static void changeVolumeRight(void);
static void changeVolumeLeft(void);
static void goToEq(void);
static void changeEqRight(void);
static void changeEqLeft(void);
static void turnOff(void);

/*** tablas de estado ***/

/*** estado apagado ***/

 STATE st_off[]=
{
  	{EncoderClick,st_rep,goToRepFromOff},

  	{FIN_TABLA,st_off,do_nothing}	//FIN_TABLA sirve como el default de un switch(event),
  									//por si llega a venir un evento que no queremos handlear
};


/*** estado reproduccion ***/

STATE st_rep[]=
{
	{EncoderLeft,st_rep,changeSongLeft},
	{EncoderRight,st_rep,changeSongRight},
	{ButtonCross,st_rep,playPauseSong},
	{ButtonPoint,st_vol,goToVolume},
	{ButtonLine,st_eq,goToEq},
	{EncoderClick, st_off, turnOff},
	{FIN_TABLA,st_rep,do_nothing}
};

/*** estado volumen ***/

STATE st_vol[] =
{
	{EncoderLeft,st_vol,changeVolumeLeft},
	{EncoderRight,st_vol,changeVolumeRight},
	{ButtonPoint,st_rep,goToRepFromVol},
	{EncoderClick, st_off, turnOff},
	{FIN_TABLA,st_vol,do_nothing}
};

/*** estado ecualizador ***/

STATE st_eq[] =
{
	{EncoderLeft,st_eq,changeEqLeft},
	{EncoderRight,st_eq,changeEqRight},
	{ButtonLine,st_rep,goToRepFromEq},
	{EncoderClick, st_off, turnOff},
	{FIN_TABLA,st_eq,do_nothing}
};


//========interfaz=================

STATE *FSM_GetInitState(void)
{
 	return (st_off);
}



///=========Rutinas de accion===============


/*Dummy function*/
static void do_nothing(void){


}
static void goToRepFromOff(void){

	char buffer[MAXLETTERS] = "                                                               ";
	//Previsional, imprimo 1,2,etc en vez del nombre de la cancion
	volNumPrinter(buffer, songSelected);
	DisplayWrite(buffer, 16, 0);

	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);

}
static void goToRepFromVol(void){

	char buffer[MAXLETTERS] = "                                                               ";
	//Previsional, imprimo 1,2,etc en vez del nombre de la cancion
	volNumPrinter(buffer, songSelected);
	DisplayWrite(buffer, 16, 0);

	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);

}
static void goToRepFromEq(void){

	char buffer[MAXLETTERS] = "                                                               ";
	//Previsional, imprimo 1,2,etc en vez del nombre de la cancion
	volNumPrinter(buffer, songSelected);
	DisplayWrite(buffer, 16, 0);

	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);

}
static void playPauseSong(void){

	pause = !pause;
	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);
}
static void changeSongLeft(void){

	if (songSelected > 1){
		songSelected--;
		char buffer[MAXLETTERS] = "                                                               ";
		//Previsional, imprimo 1,2,etc en vez del nombre de la cancion
		volNumPrinter(buffer, songSelected);
		DisplayWrite(buffer, 16, 0);
	}

}
static void changeSongRight(void){

	if (songSelected < 255){
		songSelected++;
		char buffer[MAXLETTERS] = "                                                                ";
		//Previsional, imprimo 1,2,etc en vez del nombre de la cancion
		volNumPrinter(buffer, songSelected);
		DisplayWrite(buffer, 16, 0);
	}

}
static void goToVolume(void){

	char * buffer = "    VOLUMEN     ";
	DisplayWrite(buffer, 16, 0);
	char buffer2[17] = "                ";
	volNumPrinter(buffer2, volumeLevel);
	DisplayWrite(buffer2, 16, 1);

}
static void changeVolumeRight(void){

	if (volumeLevel < 29){
		volumeLevel++;
		char buffer2[17] = "                ";
		volNumPrinter(buffer2, volumeLevel);
		DisplayWrite(buffer2, 16, 1);
	}


}
static void changeVolumeLeft(void){

	if (volumeLevel > 0){
		volumeLevel--;
		char buffer2[17] = "                ";
		volNumPrinter(buffer2, volumeLevel);
		DisplayWrite(buffer2, 16, 1);
	}
}
static void goToEq(void){

	char * buffer = "       EQ       ";
	DisplayWrite(buffer, 16, 0);
	char buffer2[17] = "                ";
	eqStringPrinter(buffer2, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);

}
static void changeEqRight(void){

	if (eqLevel < (EQCANT-1)){
		eqLevel++;
		char buffer2[17] = "                ";
		eqStringPrinter(buffer2, eqBands[eqLevel]);
		DisplayWrite(buffer2, 16, 1);
	}


}
static void changeEqLeft(void){

	if (eqLevel > 0){
		eqLevel--;
		char buffer2[17] = "                ";
		eqStringPrinter(buffer2, eqBands[eqLevel]);
		DisplayWrite(buffer2, 16, 1);
	}

}
static void turnOff(void){

	char buffer[17] = "    APAGADO     ";
	DisplayWrite(buffer, 16, 0);
	bufferClean(buffer);
	DisplayWrite(buffer, 16, 1);
}
