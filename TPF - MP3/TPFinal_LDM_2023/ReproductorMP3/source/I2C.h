/***************************************************************************/ /**
   @file     I2C.c
   @brief    I2C Driver for K64F. Non-Blocking and using FIFO feature
   @author   Nicol�s Magliola
  ******************************************************************************/

#ifndef _I2C_H_
#define _I2C_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_CANT_IDS 2

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
  MTX,
  MRX,
  MTXRX
} I2CMode_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize I2C driver
 * @param id I2C's number
 * @param config I2C's configuration (baudrate, parity, etc.)
 */
bool I2CInit(uint8_t id, uint8_t addr, uint32_t baudrate);

/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return 1 if the RX FIFO has something
 */
uint8_t i2cIsRxMsg(uint8_t id);

/**
 * @brief Check how many bytes were received
 * @param id UART's number
 * @return Quantity of received bytes
 */
uint32_t i2cGetRxMsgLength(uint8_t id);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
 */
uint32_t i2cReadMsg(uint8_t id, uint8_t *msg, uint32_t cant);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
 */
uint32_t i2cReadSlave(uint8_t id, uint8_t slave_addr, uint32_t cant);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
 */
uint32_t i2cWriteSlave(uint8_t id, uint8_t slave_addr, uint8_t *msg, uint32_t cant);

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
 */
uint8_t i2cRequestSlaveRegData(uint8_t id, uint8_t slave_addr, const uint32_t addr, uint32_t cant);
uint8_t i2cWriteSlaveReg(uint8_t id, uint8_t slave_addr, const uint32_t addr, uint8_t *data, uint32_t cant);
/**
 * @brief Check if all bytes were transfered
 * @param id UART's number
 * @return All bytes were transfered
 */
uint8_t i2cIsTxMsgComplete(uint8_t id);

bool isI2CBusBusy(uint8_t id);
/*******************************************************************************
 ******************************************************************************/

#endif // _I2C_H_
