#include <stdio.h>
#include <stdint.h>
#include "fsm.h"
#include "fsmtable.h"
#include "UI/display.h"
#include "UI/privateSTDIO.h"
#include "player.h"
#include "equalizer.h"

// En estas variables voy a guardar lo seleccionado por la UI en cada caso, y en cada rutina
//  de accion al cambiar a alguna de estas voy a llamar a una funcion de ustedes que tiene que responder
//  haciendo los cambios necesarios, y si lo precisasen guardando el numero al cual cambió la variable
uint8_t volumeLevel = 50;
uint8_t songSelected = 1;
uint8_t sdCardON = 0;
bool pause = true;
uint8_t eqLevel = 0;

/*Foward Declarations*/

extern STATE st_off[];
extern STATE st_noSD[];
extern STATE st_rep[];
extern STATE st_vol[];
extern STATE st_eq[];

#define EQCANT 4
const char *eqBands[EQCANT] = {"Flat", "Rock", "Pop", "Bass"};

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
static void autoPlayNextSong(void);
static void goToRepFromNoSD(void);
static void NOSDcard(void);
static void YESSDcard(void);

/*** tablas de estado ***/

/*** estado apagado ***/

STATE st_off[] =
	{
		{SDCardRemoved, st_noSD, NOSDcard},
		{SDCardInserted, st_off, YESSDcard},
		{EncoderClick, st_rep, goToRepFromOff},
		{FIN_TABLA, st_off, do_nothing} // FIN_TABLA sirve como el default de un switch(event),
										// por si llega a venir un evento que no queremos handlear
};

/*** estado no hay SD card ***/

STATE st_noSD[] =
	{
		{SDCardInserted, st_rep, goToRepFromNoSD},
		{EncoderClick, st_off, turnOff},
		{FIN_TABLA, st_off, do_nothing} // FIN_TABLA sirve como el default de un switch(event),
										// por si llega a venir un evento que no queremos handlear
};

/*** estado reproduccion ***/

STATE st_rep[] =
	{
		{EncoderLeft, st_rep, changeSongLeft},
		{EncoderRight, st_rep, changeSongRight},
		{ButtonCross, st_rep, playPauseSong},
		{ButtonPoint, st_vol, goToVolume},
		{ButtonLine, st_eq, goToEq},
		{EncoderClick, st_off, turnOff},
		{AutoNextSong, st_rep, autoPlayNextSong},
		{SDCardRemoved, st_noSD, NOSDcard},
		{FIN_TABLA, st_rep, do_nothing}};

/*** estado volumen ***/

STATE st_vol[] =
	{
		{EncoderLeft, st_vol, changeVolumeLeft},
		{EncoderRight, st_vol, changeVolumeRight},
		{ButtonPoint, st_rep, goToRepFromVol},
		{EncoderClick, st_off, turnOff},
		{SDCardRemoved, st_noSD, NOSDcard},
		{FIN_TABLA, st_vol, do_nothing}};

/*** estado ecualizador ***/

STATE st_eq[] =
	{
		{EncoderLeft, st_eq, changeEqLeft},
		{EncoderRight, st_eq, changeEqRight},
		{ButtonLine, st_rep, goToRepFromEq},
		{EncoderClick, st_off, turnOff},
		{SDCardRemoved, st_noSD, NOSDcard},
		{FIN_TABLA, st_eq, do_nothing}};

//========interfaz=================

STATE *FSM_GetInitState(void)
{
	return (st_off);
}

///=========Rutinas de accion===============

/*Dummy function*/
static void do_nothing(void)
{
}
static void goToRepFromOff(void)
{
	if (sdCardON == false)
	{
		putEvent(SDCardRemoved);
	}
	DisplayOnTimers();
	DisplayDisplay();
	DisplayBacklight();
	char *name = getCurrentSongName();
	uint8_t len = strlen(name);
	DisplayWrite(name, len + 1, 0);

	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);
}
static void goToRepFromVol(void)
{
	char *name = getCurrentSongName();
	uint8_t len = strlen(name);
	DisplayWrite(name, len + 1, 0);

	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);
}
static void goToRepFromEq(void)
{
	char *name = getCurrentSongName();
	uint8_t len = strlen(name);
	DisplayWrite(name, len + 1, 0);

	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);
}
static void playPauseSong(void)
{

	pause = !pause;
	if (pause)
	{
		pausePlayer();
	}
	else
	{
		playPlayer();
	}
	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);
}
static void changeSongLeft(void)
{
	prevSong();
	char *name = getCurrentSongName();
	uint8_t len = strlen(name);
	DisplayWrite(name, len + 1, 0);
}
static void changeSongRight(void)
{
	nextSong();
	char *name = getCurrentSongName();
	uint8_t len = strlen(name);
	DisplayWrite(name, len + 1, 0);
}
static void goToVolume(void)
{

	char *buffer = "    VOLUMEN     ";
	DisplayWrite(buffer, 16, 0);
	char buffer2[17] = "                ";
	volNumPrinter(buffer2, volumeLevel);
	DisplayWrite(buffer2, 16, 1);
}
static void changeVolumeRight(void)
{

	if (volumeLevel < 100)
	{
		volumeLevel += 5;
		char buffer2[17] = "                ";
		volNumPrinter(buffer2, volumeLevel);
		DisplayWrite(buffer2, 16, 1);
		adjustVolume((float)volumeLevel / 100.0f);
	}
}
static void changeVolumeLeft(void)
{

	if (volumeLevel > 0)
	{
		volumeLevel -= 5;
		char buffer2[17] = "                ";
		volNumPrinter(buffer2, volumeLevel);
		DisplayWrite(buffer2, 16, 1);
		adjustVolume((float)volumeLevel / 100.0f);
	}
}
static void goToEq(void)
{

	char *buffer = "       EQ       ";
	DisplayWrite(buffer, 16, 0);
	char buffer2[17] = "                ";
	eqStringPrinter(buffer2, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);
}
static void changeEqRight(void)
{

	if (eqLevel < (EQCANT - 1))
	{
		eqLevel++;
		changePreset(eqLevel);
		char buffer2[17] = "                ";
		eqStringPrinter(buffer2, eqBands[eqLevel]);
		DisplayWrite(buffer2, 16, 1);
	}
}
static void changeEqLeft(void)
{

	if (eqLevel > 0)
	{
		eqLevel--;
		changePreset(eqLevel);
		char buffer2[17] = "                ";
		eqStringPrinter(buffer2, eqBands[eqLevel]);
		DisplayWrite(buffer2, 16, 1);
	}
}
static void turnOff(void)
{
	DisplayNoBacklight();
	char buffer[17] = "                ";
	DisplayWrite(buffer, 16, 0);
	bufferClean(buffer);
	DisplayWrite(buffer, 16, 1);
	DisplayNoDisplay();
	stopPlayer();
	pause = true;
	SDK_DelayAtLeastUs(1000000U, CLOCK_GetCoreSysClkFreq());
	DisplayOffTimers();
	// deepSleep();
}

static void autoPlayNextSong(void)
{
	char *name = getCurrentSongName();
	uint8_t len = strlen(name);
	DisplayWrite(name, len + 1, 0);
}

static void goToRepFromNoSD(void)
{
	sdCardON = true;
	pause = true;
	DisplayBacklight();
	char *name = getCurrentSongName();
	uint8_t len = strlen(name);
	DisplayWrite(name, len + 1, 0);

	char buffer2[17] = "                ";
	repDisplayPrinter(buffer2, pause, volumeLevel, eqBands[eqLevel]);
	DisplayWrite(buffer2, 16, 1);
}

static void NOSDcard()
{
	sdCardON = false;
	DisplayBacklight();
    char buffer[17] = "   INSERTAR SD  ";
    DisplayWrite(buffer, 16, 0);
    bufferClean(buffer);
    DisplayWrite(buffer, 16, 1);
}

static void YESSDcard()
{
	sdCardON = true;
}
