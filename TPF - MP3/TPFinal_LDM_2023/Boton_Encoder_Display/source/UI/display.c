/***************************************************************************//**
  @file     display.c
  @brief    El control del display se basa en la libreria de LiquidCrystal_I2C para Arduino. link: https://github.com/johnrickman/LiquidCrystal_I2C
  @author	Grupo 3
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "..\config.h"
#include "..\I2C.h"
#include "..\Queue.h"
#include "display.h"
#include "timer.h"
#include <string.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define WriteDriverByteIIC(args) i2cWriteSlave(0, DISPLAY_ADDR, (args) , 1)
#define NOCHARACTER 32
#define SENDINGTIME 1 //ms

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/



/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void send(uint8_t data, uint8_t mode);	//Send modes are Rw and Rs or none(command).
void command(uint8_t data);
void write4bits(uint8_t * value);
void DelayInit(uint32_t delay);
void DisplayPeriodicISR();
void displaySendPeriodicISR();

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint8_t _backlightval = LCD_BACKLIGHT;
static uint8_t _displayfunction;
static uint8_t _displaycontrol;
static uint8_t _displaymode;

static char upperLine[MAXLETTERS] = {' ', ' ', ' ', ' ', 'A', 'P', 'A', 'G', 'A', 'D', 'O', ' ', ' ', ' ', ' ', ' ', '\0', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0', '\0'};

static char lowerLine[MAXLETTERS] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0', '\0'};

static uint8_t offsetCursor = 0; //Este es el cursor que se mueve dentro de la palabra
static uint8_t upperCharCant = 16;

uint8_t displayTimer = 0;
uint8_t displayDataTimer = 0;

static queue_t displayDataBuffer = {{0}, 0, 0, false, true};

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


void initDisplay() {

	//Empiezo a usar send por eso ya arranco el timer displayDataTimer
	displayDataTimer = createTimer_SYS(SENDINGTIME, displaySendPeriodicISR, PERIODIC);
	startTimer_SYS(displayDataTimer);

	_displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
	uint8_t zero = 0;

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 100ms
	DelayInit(1428572UL);

	// put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	uint8_t word_msg = 0x03 << 4;
	WriteDriverByteIIC(&word_msg);
	DelayInit(59000UL); // wait more than 4.1ms

	WriteDriverByteIIC(&zero);
	DelayInit(2000UL); // wait more than 100us
    WriteDriverByteIIC(&word_msg);
    DelayInit(2000UL); // wait more than 100us

    WriteDriverByteIIC(&zero);
    DelayInit(2000UL); // wait more than 100us
    WriteDriverByteIIC(&word_msg);
    DelayInit(1500UL);

    // finally, set to 4-bit interface
    word_msg = 0x02 << 4;
    WriteDriverByteIIC(&word_msg);
    DelayInit(58572UL);

    WriteDriverByteIIC(&zero);
    DelayInit(58572UL);
    WriteDriverByteIIC(&word_msg);
    DelayInit(58572UL);

    word_msg = LCD_FUNCTIONSET | _displayfunction;
    WriteDriverByteIIC(&word_msg);
    DelayInit(58572UL);

    WriteDriverByteIIC(&zero);
    DelayInit(58572UL);

    word_msg = 0x08 << 4;
    WriteDriverByteIIC(&word_msg);
    DelayInit(58572UL);

    WriteDriverByteIIC(&zero);
    DelayInit(58572UL);

    word_msg = 0x01 << 4;
    WriteDriverByteIIC(&word_msg);
    DelayInit(58572UL);

    WriteDriverByteIIC(&zero);
    DelayInit(58572UL);

    word_msg = LCD_ENTRYMODESET | _displaymode | _backlightval;
    WriteDriverByteIIC(&word_msg);
    DelayInit(58572UL);

    displayTimer = createTimer_SYS(REFRESHTIME, DisplayPeriodicISR, PERIODIC);
    startTimer_SYS(displayTimer);
}


void DisplayClear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero

	DelayInit(22142UL);
	// delayMicroseconds(2000);  // this command takes a long time!
}

void DisplayHome(){
	command(LCD_RETURNHOME);  // set cursor position to zero

	DelayInit(22142UL);
	//delayMicroseconds(2000);  // this command takes a long time!
}

void DisplaySetCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > DISPLAY_ROWS ) {
		row = DISPLAY_ROWS-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void DisplayNoDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void DisplayDisplay() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void DisplayNoCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void DisplayCursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void DisplayNoBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void DisplayBlink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void DisplayScrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void DisplayScrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void DisplayLeftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void DisplayRightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void DisplayAutoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void DisplayNoAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Turn the (optional) backlight off/on
void DisplayNoBacklight(void) {
	_backlightval=LCD_NOBACKLIGHT;
	uint8_t zero = 0;	//Así apago el backlight
	WriteDriverByteIIC(&zero);
}

void DisplayBacklight(void) {
	_backlightval=LCD_BACKLIGHT;
	uint8_t backlight = LCD_BACKLIGHT;	//Así lo prendo el backlight
	WriteDriverByteIIC(&backlight);
}


uint8_t DisplayWrite(char * msg, uint8_t cant, uint8_t line){

	int i = 0;
	if (line == 0){ //upper line

		for(i = 0;i<cant;i++){
			upperLine[i] = msg[i];
		}
		upperCharCant = cant;
	}
	else if(line == 1){	//lower line

		for(i = 0;i<cant;i++){
			lowerLine[i] = msg[i];
		}
	}
	else{
		return 1; //error
	}

	DisplayClear();
	DisplayHome();
	offsetCursor = 0;

	return 0;
}

void DisplayPeriodicISR(){

	//DisplayClear();
	DisplayHome();
	int i=0;
	while (i<16 && upperLine[i + offsetCursor ]!='\0' ){	//Relleno con lo que haya para escribir
		send(upperLine[i  + offsetCursor], DATA_Rs);
		i++;
	}
	if (i<16){	//Relleno el resto del display con nada, (borro lo que estaba antes)
		while (i<16){
			send(NOCHARACTER, DATA_Rs);
			i++;
		}
	}


	if (upperCharCant > 16){	//Ajusto con el offset para hacer la ventana corrediza, si hace falta
		offsetCursor++;
		if ( (16 + offsetCursor) > upperCharCant ){
			offsetCursor= 0;
		}
	}

	DisplaySetCursor(0, 1);	//Ahora la de abajo, esta no tiene funcion de ventana corrediza
	i = 0;

	while (i<16 && lowerLine[i]!='\0' ){
		send(lowerLine[i], DATA_Rs);
		i++;
	}
	if (i<16){	//Relleno el display con nada, (borro lo que estaba antes)
		while (i<16){
			send(NOCHARACTER, DATA_Rs);
			i++;
		}
	}
}

void displaySendPeriodicISR(){

	static int sendingState = 0;
	static uint8_t wordSend;

	//Primero lo mando sin el enable, y después pulseando el enable. No sé porque... la libreria lo hace asi

	switch(sendingState){
	case 0:
		if (getFillLevel(&displayDataBuffer) != 0){
			wordSend = getNext(&displayDataBuffer);
			WriteDriverByteIIC(&wordSend);
			sendingState=1;
		}
	break;
	case 1:
		wordSend |= En;
		WriteDriverByteIIC(&wordSend);
		sendingState=2;
	break;
	case 2:
		wordSend &= ~En;
		WriteDriverByteIIC(&wordSend);
		sendingState = 0;
	break;
	default:

	break;
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



void send(uint8_t data, uint8_t mode){

	//Primero mando el nibble alto y después el bajo.

	uint8_t highnib= (data&0xf0) | mode | _backlightval;
	uint8_t lownib= ((data<<4)&0xf0) |mode | _backlightval;

	put(&displayDataBuffer, highnib);
	put(&displayDataBuffer, lownib);


	/*	OLD SEND FUNCTION
	uint8_t highnib= (data&0xf0) | mode | _backlightval;
	uint8_t lownib= ((data<<4)&0xf0) |mode | _backlightval;

	//Primero mando el nibble alto y después el bajo.
	//Primero lo mando sin el enable, y después pulseando el enable. No sé porque... la libreria lo hace asi

	WriteDriverByteIIC(&highnib);
	DelayInit(1500UL);
	highnib |= En;
	WriteDriverByteIIC(&highnib);
	DelayInit(1500UL);
	highnib &= ~En;
	WriteDriverByteIIC(&highnib);
	DelayInit(1500UL);

	//Después de esto habría que hacer un delay >37us segun la libreria. Asi funca

	WriteDriverByteIIC(&lownib);
	DelayInit(1500UL);
	lownib |= En;
	WriteDriverByteIIC(&lownib);
	DelayInit(1500UL);
	lownib &= ~En;
	WriteDriverByteIIC(&lownib);
	DelayInit(1500UL);

	*/
}

void command(uint8_t data){

	send(data, COMMAND);
}

void DelayInit(uint32_t delay){	//time is aprox = delay * 7 / timeClock.

    while (delay--);
}
