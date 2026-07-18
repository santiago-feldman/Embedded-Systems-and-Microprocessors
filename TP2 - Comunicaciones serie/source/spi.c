/***************************************************************************/ /**
   @file     spi.c
   @brief    Application functions
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "spi.h"
#include "MK64F12.h"
#include "hardware.h"
#include "config.h"
#include "Queue.h"
#include "gpio.h"
#include "stdbool.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SPI_DEFAULT_BAUDRATE 9600

typedef enum
{
    PORT_mAnalog,
    PORT_mGPIO,
    PORT_mAlt2,
    PORT_mAlt3,
    PORT_mAlt4,
    PORT_mAlt5,
    PORT_mAlt6,
    PORT_mAlt7,
} PORTMux_t;

typedef enum
{
    PORT_eDisabled = 0x00,
    PORT_eDMARising = 0x01,
    PORT_eDMAFalling = 0x02,
    PORT_eDMAEither = 0x03,
    PORT_eInterruptDisasserted = 0x08,
    PORT_eInterruptRising = 0x09,
    PORT_eInterruptFalling = 0x0A,
    PORT_eInterruptEither = 0x0B,
    PORT_eInterruptAsserted = 0x0C,
} PORTEvent_t;

enum
{
    RFDF = 17,
    TFFF = 25,
    EOQF = 28,
    TCF = 31,
}; // SPI STATUS REGISTER BITS

enum
{
    MSTR = 31,
    RXCTR = 4
}; // MCR STATUS REGISTER BITS

#define SET(reg, mask, val) ((reg) = (((reg) & ~(mask)) | (val)))
#define BIT(source, x) ((source >> x) & 0b1)
#define IGNORE (0)
#define MAXTRASHSIZE (200)
#define BYTELEN(x) (sizeof(x) / sizeof((uint8_t)(x[0])))

/*******************************************************************************
 * GLOBAL VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void (*callback)(void) = 0;
static bool busy = false;
static uint16_t ignoreRead = 0;
static queue_t RX0queue = {{0}, 0, 0, false, true};
static queue_t TX0queue = {{0}, 0, 0, false, true};
static PORT_Type *const PORTs[] = PORT_BASE_PTRS;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void setSPIBaudrate(SPI_Type *spi, uint32_t baudrate);
static uint32_t spiInsertMsg(const uint8_t *msg, uint32_t cant);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
// CTAR FMSZ
// TCF
bool spiInit(spi_cfg_t config)
{
    uint8_t pinClk = PIN2NUM(SPI0_SCK);
    uint8_t pinMOSI = PIN2NUM(SPI0_MOSI);
    uint8_t pinMISO = PIN2NUM(SPI0_MISO);
    uint8_t pinSS = PIN2NUM(SPI0_SS);
    PORT_Type *const portClk = PORTs[PIN2PORT(SPI0_SCK)];   // Getting tx port ref
    PORT_Type *const portMOSI = PORTs[PIN2PORT(SPI0_MOSI)]; // Getting tx port ref
    PORT_Type *const portMISO = PORTs[PIN2PORT(SPI0_MISO)]; // Getting tx port ref
    PORT_Type *const portSS = PORTs[PIN2PORT(SPI0_SS)];     // Getting tx port ref
    SPI_Type *const spi0 = SPI0;                            // Getting current SPI_Type ptr

    portClk->PCR[pinClk] = 0x0;                                                           // Clear all bits
    portMOSI->PCR[pinMOSI] = 0x0;                                                         // Clear all bits
    portMISO->PCR[pinMISO] = 0x0;                                                         // Clear all bits
    portClk->PCR[pinClk] |= (PORT_PCR_MUX(PORT_mAlt2) | PORT_PCR_IRQC(PORT_eDisabled));   // Set MUX
    portMOSI->PCR[pinMOSI] |= (PORT_PCR_MUX(PORT_mAlt2) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX and port interrupts
    portMISO->PCR[pinMISO] |= (PORT_PCR_MUX(PORT_mAlt2) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX and port interrupts
    portSS->PCR[pinSS] |= (PORT_PCR_MUX(PORT_mAlt2) | PORT_PCR_IRQC(PORT_eDisabled));     // Set SS

    if (config.async)
    {
        NVIC_EnableIRQ(SPI0_IRQn);
    }

    // Enable clk gating
    SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;

    // Configure mode
    switch (config.spiMode)
    {
    case MASTER:

        SET(spi0->MCR, SPI_MCR_MSTR_MASK, SPI_MCR_MSTR(1)); // Elegimos el modo master
        SET(spi0->MCR, SPI_MCR_HALT_MASK, SPI_MCR_HALT(0)); // Habilitamos la transmision

        // Activamos 3 interrupciones de la FIFO
        SET(spi0->RSER, SPI_RSER_TCF_RE_MASK, SPI_RSER_TCF_RE(0));

        // Configuramos si los SS, 0 si el slave es activo con 1, 1 si el slave es activo con 0
        SET(spi0->MCR, SPI_MCR_PCSIS_MASK, SPI_MCR_PCSIS(0b11111));

        SET(spi0->MCR, SPI_MCR_MDIS_MASK, SPI_MCR_MDIS(0)); // habilita que el master para poner clk

        SET(spi0->CTAR[0], SPI_CTAR_CPHA_MASK, SPI_CTAR_CPHA(config.highPhase)); // Lo ponemos en MODO (0,0)
        SET(spi0->CTAR[0], SPI_CTAR_CPOL_MASK, SPI_CTAR_CPOL(config.highPolarity));
        SET(spi0->CTAR[0], SPI_CTAR_CSSCK_MASK, SPI_CTAR_CSSCK(0b0011));
        SET(spi0->CTAR[0], SPI_CTAR_PASC_MASK, SPI_CTAR_PASC(0b10));
        SET(spi0->CTAR[0], SPI_CTAR_ASC_MASK, SPI_CTAR_ASC(0b0100));
        SET(spi0->CTAR[0], SPI_CTAR_FMSZ_MASK, SPI_CTAR_FMSZ(7));
        break;

    case SLAVE:

        SET(spi0->MCR, SPI_MCR_MSTR_MASK, SPI_MCR_MSTR(0)); // Elegimos el modo slave
        SET(spi0->MCR, SPI_MCR_HALT_MASK, SPI_MCR_HALT(0)); // Habilitamos la transmision
        break;
    }

    setSPIBaudrate(spi0, config.baudrate); // Elegimos el baudrate

    return true;
}

uint32_t spiReadMsg(uint8_t *msg, uint32_t RXcant)
{

    uint32_t realCount = 0;

    if (spiIsRxMsg())
    {                                                     // Si hay algun mensaje
        realCount = getNextArray(&RX0queue, msg, RXcant); // Agarramos la data propiamente solicitada
    }

    return realCount;
}

uint32_t spiWriteOnlyMsg(const uint8_t *msg, uint32_t TXcant, void (*cb)(void))
{
    uint32_t realCount = 0;
    callback = cb;
    if (spiIsTxMsgComplete())
    {                                          // Si ya se envio el mensaje anterior
        realCount = spiInsertMsg(msg, TXcant); // Enviamos el mensaje
        ignoreRead = TXcant;
    }

    return realCount;
}

uint32_t spiWriteReadMsg(const uint8_t *msg, uint32_t TXcant, uint32_t RXcant, void (*cb)(void))
{
    uint32_t realCount = 0;
    callback = cb;
    if (spiIsTxMsgComplete())
    { // Si ya se envio el mensaje anterior
        ignoreRead = TXcant;
        putArray(&TX0queue, msg, TXcant);
        for (int i = 0; i < RXcant; i++)
        {
            put(&TX0queue, 0);
        }
        queuedata_t data = getNext(&TX0queue);
        uint32_t nextTX = 0;
        SET(nextTX, SPI_PUSHR_PCS_MASK, SPI_PUSHR_PCS(1 << 0)); // Seleccionamos el slave, siempre el 0

        if (getFillLevel(&TX0queue) == 0)
        {
            SET(nextTX, SPI_PUSHR_CONT_MASK, SPI_PUSHR_CONT(0));
        }
        else
        {
            SET(nextTX, SPI_PUSHR_CONT_MASK, SPI_PUSHR_CONT(1));
        }
        SET(nextTX, SPI_PUSHR_TXDATA_MASK, SPI_PUSHR_TXDATA(data)); // Cargamos la informacion a enviar
        busy = true;
        SPI0->PUSHR = nextTX;                                      // Enviamos
        SET(SPI0->RSER, SPI_RSER_TCF_RE_MASK, SPI_RSER_TCF_RE(1)); // Comenzamos a transmitir
    }

    return realCount;
}

uint32_t spiGetRxMsgLength()
{
    return getFillLevel(&RX0queue);
}

bool spiIsTxMsgComplete()
{
    return !busy;
}

bool spiIsRxMsg()
{
    return !RX0queue.isEmpty;
}

void SPI0_IRQHandler(void)
{

    if (BIT(SPI0->SR, TCF)) // Si la TX FIFO no esta llena
    {
        SET(SPI0->SR, SPI_SR_TCF_MASK, SPI_SR_TCF(1));

        if (TX0queue.isEmpty)
        {
            SET(SPI0->RSER, SPI_RSER_TCF_RE_MASK, SPI_RSER_TCF_RE(0));
            uint8_t read = SPI0->POPR;
            if (!ignoreRead)
            {
                put(&RX0queue, read);
            }
            else
            {
                ignoreRead--;
            }
            busy = false;

            if (callback != 0)
            {
                callback();
            }
            return;
        }
        queuedata_t data = getNext(&TX0queue);

        uint32_t nextTX = 0;
        SET(nextTX, SPI_PUSHR_PCS_MASK, SPI_PUSHR_PCS(1 << 0)); // Seleccionamos el slave, siempre el 0
        if (getFillLevel(&TX0queue) == 0)
        {
            SET(nextTX, SPI_PUSHR_CONT_MASK, SPI_PUSHR_CONT(0));
        }
        else
        {
            SET(nextTX, SPI_PUSHR_CONT_MASK, SPI_PUSHR_CONT(1));
        }
        SET(nextTX, SPI_PUSHR_TXDATA_MASK, SPI_PUSHR_TXDATA(data)); // Cargamos la informacion a enviar

        SPI0->PUSHR = nextTX; // Enviamos

        uint8_t read = SPI0->POPR;
        if (!ignoreRead)
        {
            put(&RX0queue, read);
        }
        else
        {
            ignoreRead--;
        }
    }
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void setSPIBaudrate(SPI_Type *spi, uint32_t baudrate)
{
    SPI0->CTAR[0] |= SPI_CTAR_PBR(0b10);
    SPI0->CTAR[0] |= SPI_CTAR_BR(0b0101); // Seteamos la frecuencia de operacion a
}

static uint32_t spiInsertMsg(const uint8_t *msg, uint32_t cant)
{
    uint8_t zero = 0;
    uint32_t realCount = putArray(&TX0queue, msg, cant); // Guardamos el mensaje a mandar

    queuedata_t data = getNext(&TX0queue);
    uint32_t nextTX = 0;
    SET(nextTX, SPI_PUSHR_PCS_MASK, SPI_PUSHR_PCS(1 << 0)); // Seleccionamos el slave, siempre el 0

    if (getFillLevel(&TX0queue) == 0)
    {
        SET(nextTX, SPI_PUSHR_CONT_MASK, SPI_PUSHR_CONT(0));
    }
    else
    {
        SET(nextTX, SPI_PUSHR_CONT_MASK, SPI_PUSHR_CONT(1));
    }
    SET(nextTX, SPI_PUSHR_TXDATA_MASK, SPI_PUSHR_TXDATA(data)); // Cargamos la informacion a enviar
    busy = true;
    SPI0->PUSHR = nextTX;                                      // Enviamos
    SET(SPI0->RSER, SPI_RSER_TCF_RE_MASK, SPI_RSER_TCF_RE(1)); // Comenzamos a transmitir
    return realCount;
}

/*******************************************************************************
 ******************************************************************************/
