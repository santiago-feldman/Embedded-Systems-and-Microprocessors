/***************************************************************************/ /**
   @file     SPI.c
   @brief    SPI Driver for K64F. Non-Blocking and using FIFO feature
   @author
  ******************************************************************************/

#ifndef _SPI_H_
#define _SPI_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SPI_CANT_IDS 3

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
    SPI0,
    SPI1,
    SPI2,
};

typedef enum
{
    MASTER,
    SLAVE
} SPIMode_t;

typedef struct
{
    bool async;
    bool highPolarity;
    bool highPhase;
    uint32_t baudrate;
    SPIMode_t spiMode;
} spi_cfg_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize SPI driver
 * @param config SPI's configuration (baudrate, parity, etc.)
 */
bool spiInit(spi_cfg_t config);

/**
 * @brief Read a received message. Non-Blocking. Only takes effect if there are any msg in RXFIFO.
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes of useful data
 */
uint32_t spiReadMsg(uint8_t *msg, uint32_t RXcant);

/**
 * @brief Write a received message. Non-Blocking. Only takes effect if last msg was sent.
 * @param msg Buffer with bytes to send.
 * @param TXcant Desired quantity of bytes to be transmitted.
 * @param RXcant Desired quantity of bytes to be received after transmition.
 * @return Real quantity of pasted bytes
 */
uint32_t spiWriteReadMsg(const uint8_t *msg, uint32_t TXcant, uint32_t RXcant, void (*cb)(void));

/**
 * @brief Write a message to be transmitted but ignore the reads on spiRead stage. Non-Blocking
 * @param msg Buffer with the bytes to be transfered
 * @param TXcant Desired quantity of bytes to be transfered
 * @return Real quantity of pasted bytes
 */
uint32_t spiWriteOnlyMsg(const uint8_t *msg, uint32_t TXcant, void (*cb)(void));

/**
 * @brief Check if all bytes were transfered
 * @return All bytes were transfered
 */
bool spiIsTxMsgComplete();

/**
 * @brief Check if there are any bytes in RXqueue
 * @return True if any
 */
bool spiIsRxMsg();

/**
 * @brief Get the spi RXqueue length
 * @return RXqueue length
 */
uint32_t spiGetRxMsgLength();

/*******************************************************************************
 ******************************************************************************/

#endif // _SPI_H_
