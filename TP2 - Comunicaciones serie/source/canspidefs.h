/***************************************************************************/ /**
   @file     canspidef.h
   @brief    Definitions for the CAN transciever board interface using SPI
   @author   Grupo 3
  ******************************************************************************/

#ifndef _CANSPIDEF_H_
#define _CANSPIDEF_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// SPI INSTRUCTION SET
#define RESET 			0b11000000
#define READ 			0b00000011
#define READ_RX_BUFFER	0b10010000
#define WRITE 			0b00000010
#define LOAD_TX_BUFFER 	0b01000000
#define RTS 			0b10000000
#define RTSTX0 			0b10000001
#define RTSTX1 			0b10000010
#define RTSTX2 			0b10000100
#define READ_STATUS 	0b10100000
#define RX_STATUS 		0b10110000
#define BIT_MODIFY		0b00000101

// CAN CONTROLLER REGISTER MAP
#define RXF0SIDH		0b00000000
#define RXF0SIDL		0b00000001
#define RXF0EID8		0b00000010
#define RXF0EID0		0b00000011
#define RXF1SIDH		0b00000100
#define RXF1SIDL		0b00000101
#define RXF1EID8		0b00000110
#define RXF1EID0		0b00000111
#define RXF2SIDH		0b00001000
#define RXF2SIDL		0b00001001
#define RXF2EID8		0b00001010
#define RXF2EID0		0b00001011
#define BFPCTRL			0b00001100
#define TXRTSCTRL		0b00001101

#define RXF3SIDH		0b00010000
#define RXF3SIDL		0b00010001
#define RXF3EID8		0b00010010
#define RXF3EID0		0b00010011
#define RXF4SIDH		0b00000100
#define RXF4SIDL		0b00010101
#define RXF4EID8		0b00010110
#define RXF4EID0		0b00010111
#define RXF5SIDH		0b00011000
#define RXF5SIDL		0b00011001
#define RXF5EID8		0b00011010
#define RXF5EID0		0b00011011
#define TEC				0b00011100
#define REC				0b00011101

#define RXM0SIDH		0b00100000
#define RXM0SIDL		0b00100001
#define RXM0EID8		0b00100010
#define RXM0EID0		0b00100011
#define RXM1SIDH		0b00100100
#define RXM1SIDL		0b00100101
#define RXM1EID8		0b00100110
#define RXM1EID0		0b00100111
#define CNF3 			0b00101000
#define CNF2 			0b00101001
#define CNF1 			0b00101010
#define CANINTE 		0b00101011
#define CANINTF 		0b00101100
#define EFLG			0b00101101

#define TXB0CTRL		0b00110000
#define	TXB0SIDH		0b00110001
#define TXB0SIDL 		0b00110010
#define TXB0EID8		0b00110011
#define TXB0EID0		0b00110100
#define TXB0DLC			0b00110101
#define TXB0D0			0b00110110
#define TXB0D1			0b00110111
#define TXB0D2			0b00111000
#define TXB0D3			0b00111001
#define TXB0D4			0b00111010
#define TXB0D5			0b00111011
#define TXB0D6			0b00111100
#define TXB0D7			0b00111101

#define TXB1CTRL		0b01000000
#define	TXB1SIDH		0b01000001
#define TXB1SIDL 		0b01000010
#define TXB1EID8		0b01000011
#define TXB1EID0		0b01000100
#define TXB1DLC			0b01000101
#define TXB1D0			0b01000110
#define TXB1D1			0b01000111
#define TXB1D2			0b01001000
#define TXB1D3			0b01001001
#define TXB1D4			0b01001010
#define TXB1D5			0b01001011
#define TXB1D6			0b01001100
#define TXB1D7			0b01001101

#define TXB2CTRL		0b01010000
#define	TXB2SIDH		0b01010001
#define TXB2SIDL 		0b01010010
#define TXB2EID8		0b01010011
#define TXB2EID0		0b01010100
#define TXB2DLC			0b01010101
#define TXB2D0			0b01010110
#define TXB2D1			0b01010111
#define TXB2D2			0b01011000
#define TXB2D3			0b01011001
#define TXB2D4			0b01011010
#define TXB2D5			0b01011011
#define TXB2D6			0b01011100
#define TXB2D7			0b01011101

#define RXB0CTRL		0b01100000
#define	RXB0SIDH		0b01100001
#define RXB0SIDL 		0b01100010
#define RXB0EID8		0b01100011
#define RXB0EID0		0b01100100
#define RXB0DLC			0b01100101
#define RXB0D0			0b01100110
#define RXB0D1			0b01100111
#define RXB0D2			0b01101000
#define RXB0D3			0b01101001
#define RXB0D4			0b01101010
#define RXB0D5			0b01101011
#define RXB0D6			0b01101100
#define RXB0D7			0b01101101

#define RXB1CTRL		0b01110000
#define	RXB1SIDH		0b01110001
#define RXB1SIDL 		0b01110010
#define RXB1EID8		0b01110011
#define RXB1EID0		0b01110100
#define RXB1DLC			0b01110101
#define RXB1D0			0b01110110
#define RXB1D1			0b01110111
#define RXB1D2			0b01111000
#define RXB1D3			0b01111001
#define RXB1D4			0b01111010
#define RXB1D5			0b01111011
#define RXB1D6			0b01111100
#define RXB1D7			0b01111101

#define CANSTAT			0b00001110
#define CANCTRL			0b00001111


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
  RXB0,
  RXB1
} RXBuffer_t;

typedef enum
{
  TXB0,
  TXB1,
  TXB2
} TXBuffer_t;

typedef enum
{
	RXF0,
	RXF1,
	RXF2,
	RXF3,
	RXF4,
	RXF5,
	RXM0,
	RXM1
} RXAcceptance_t;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/*******************************************************************************
 ******************************************************************************/

#endif // _CANSPIDEF_H_
