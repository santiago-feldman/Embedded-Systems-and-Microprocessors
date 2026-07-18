/***************************************************************************//**
  @file     display.h
  @brief    El control del display se basa en la libreria de LiquidCrystal_I2C para Arduino. link: https://github.com/johnrickman/LiquidCrystal_I2C
  @author	Grupo 3
 ******************************************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/



/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

// send "modes"
#define En 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define DATA_Rs 0b00000001  // Register select bit
#define COMMAND 0

// display hardware features
#define DISPLAY_COLS 16
#define DISPLAY_ROWS 2
#define DISPLAY_ADDR 0x27


#define MAXLETTERS 64
#define REFRESHTIME 500 //in miliseconds

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

//Funciones principales

void initDisplay();

uint8_t DisplayWrite(char * msg, uint8_t cant, uint8_t line);
//Acá se escribe lo que querés que ponga en la pantalla
//Y cada REFRESHTIME por una interrupcion de systick periodica se escribe el display
//Line=0 escribe en la linea de arriba y Line=1 escribe en la de abajo
//Automaticamente si la cantidad es mayor a 16 entonces va a activar la marquesina movible
//La cual solo está activa en la parte de arriba de momento



//Funciones secundarias
void DisplayClear();

void DisplayHome();

void DisplaySetCursor(uint8_t col, uint8_t row);

// Turn the display on/off (quickly)
void DisplayNoDisplay();
void DisplayDisplay();

// Turns the underline cursor on/off
void DisplayNoCursor();
void DisplayCursor();

// Turn on and off the blinking cursor
void DisplayNoBlink();
void DisplayBlink();

// These commands scroll the display without changing the RAM
void DisplayScrollDisplayLeft(void);
void DisplayScrollDisplayRight(void);

// This is for text that flows Left to Right
void DisplayLeftToRight(void);

// This is for text that flows Right to Left
void DisplayRightToLeft(void);

// This will 'right justify' text from the cursor
void DisplayAutoscroll(void);

// This will 'left justify' text from the cursor
void DisplayNoAutoscroll(void);

// Turn the (optional) backlight off/on
void DisplayNoBacklight(void);
void DisplayBacklight(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _ENCODER_H_
