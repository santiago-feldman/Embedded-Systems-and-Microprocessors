/***************************************************************************/ /**
   @file     UART.c
   @brief    UART Driver for K64F. Non-Blocking and using FIFO feature
   @author   Nicol�s Magliola
  ******************************************************************************/

#ifndef _UART_H_
#define _UART_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_CANT_IDS 5

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
	UARTID0,
	UARTID1,
	UARTID2,
	UARTID3,
	UARTID4,
	UARTID5
};

typedef enum
{
	TX,
	RX,
	TXRX
} UARTMode_t;

typedef struct
{
	bool IRQEnabled;
	bool oddParity;
	uint32_t baudrate;
	UARTMode_t uartMode;
} uart_cfg_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize UART driver
 * @param id UART's number
 * @param config UART's configuration (baudrate, parity, etc.)
 */
bool uartInit(uint8_t id, uart_cfg_t config);

/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return 1 if the RX FIFO has something
 */
uint8_t uartIsRxMsg(uint8_t id);

/**
 * @brief Check how many bytes were received
 * @param id UART's number
 * @return Quantity of received bytes
 */
uint32_t uartGetRxMsgLength(uint8_t id);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
 */
uint32_t uartReadMsg(uint8_t id, uint8_t *msg, uint32_t cant);

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
 */
uint8_t uartWriteMsg(uint8_t id, const uint8_t *msg, uint8_t cant);

uint8_t uartWriteString(uint8_t id, const uint8_t *msg);

/**
 * @brief Check if all bytes were transfered
 * @param id UART's number
 * @return All bytes were transfered
 */
uint8_t uartIsTxMsgComplete(uint8_t id);

/*******************************************************************************
 ******************************************************************************/

#endif // _UART_H_
