/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "uart.h"
#include "MK64F12.h"
#include "hardware.h"
#include "config.h"
#include "Queue.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UARTS_TO_USE 2
#define UART_HAL_DEFAULT_BAUDRATE 9600

#define GET_PORT_BASE_PTRS(id) (PORT_BASE_PTRS[id])
#define GET_UART_RX_TX_IRQS(id) (UART_RX_TX_IRQS[id])

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
	PF,
	FE,
	NF,
	OR,
	IDLE,
	RDRF,
	TC,
	TDRE
}; // UART STATUS REGISTER 1 BITS

/*******************************************************************************
 * GLOBAL VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static UART_Type *const UARTs[] = UART_BASE_PTRS;
static const IRQn_Type UARTs_IRQ[] = UART_RX_TX_IRQS;
static PORT_Type *const PORTs[] = PORT_BASE_PTRS;
static const pin_t PINs[5][2] = UART_PINS;

#if UARTS_TO_USE >= 1
static queue_t RX0queue = {{0}, 0, 0, false, true};
static queue_t TX0queue = {{0}, 0, 0, false, true};
#endif
#if UARTS_TO_USE >= 2
static queue_t RX1queue = {{0}, 0, 0, false, true};
static queue_t TX1queue = {{0}, 0, 0, false, true};
#endif
#if UARTS_TO_USE >= 3
static queue_t RX2queue = {{0}, 0, 0, false, true};
static queue_t TX2queue = {{0}, 0, 0, false, true};
#endif
#if UARTS_TO_USE >= 4
static queue_t RX3queue = {{0}, 0, 0, false, true};
static queue_t TX3queue = {{0}, 0, 0, false, true};
#endif
#if UARTS_TO_USE >= 5
static queue_t RX4queue = {{0}, 0, 0, false, true};
static queue_t TX4queue = {{0}, 0, 0, false, true};
#endif

#if UARTS_TO_USE == 1
static queue_t *const TxQueues[] = {
	&TX0queue,
};

static queue_t *const RxQueues[] = {
	&RX0queue,
};
#endif
#if UARTS_TO_USE == 2
static queue_t *const TxQueues[] = {
	&TX0queue,
	&TX1queue};

static queue_t *const RxQueues[] = {
	&RX0queue,
	&RX1queue};
#endif
#if UARTS_TO_USE == 3
static queue_t *const TxQueues[] = {
	&TX0queue,
	&TX1queue,
	&TX2queue};

static queue_t *const RxQueues[] = {
	&RX0queue,
	&RX1queue,
	&RX2queue};
#endif
#if UARTS_TO_USE == 4
static queue_t *const TxQueues[] = {
	&TX0queue,
	&TX1queue,
	&TX2queue,
	&TX3queue};

static queue_t *const RxQueues[] = {
	&RX0queue,
	&RX1queue,
	&RX2queue,
	&RX3queue};
#endif
#if UARTS_TO_USE == 5
static queue_t *const TxQueues[] = {
	&TX0queue,
	&TX1queue,
	&TX2queue,
	&TX3queue,
	&TX4queue};

static queue_t *const RxQueues[] = {
	&RX0queue,
	&RX1queue,
	&RX2queue,
	&RX3queue,
	&RX4queue};
#endif

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void SetUARTBaudrate(UART_Type *uart, uint32_t baudrate);

/**
 * @brief Write to the TX FIFO buffer
 * @param id UART's number
 * @param msg Buffer to paste onto the FIFO Buffer
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
 */
/**
 * @brief Callback to refill TX Buffer with configured msg
 * @param id UART's number
 */
static void writeTXdata_CB(uint8_t id);
static void readRXdata_CB(uint8_t id);

static void setTxInterrupt(uint8_t id, bool state);
static void setRxInterrupt(uint8_t id, bool state);

/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool uartInit(uint8_t id, uart_cfg_t config)
{
	if (id >= UARTS_TO_USE)
	{
		return false;
	}

	gpioMode(PORTNUM2PIN(PC, 5), OUTPUT);

	uint8_t pinTx = PIN2NUM(PINs[id][TX]);					 // Getting tx pin ref
	uint8_t pinRx = PIN2NUM(PINs[id][RX]);					 // Getting rx pin ref
	UART_Type *const uart = UARTs[id];						 // Getting current UART_Type ptr
	PORT_Type *const portTx = PORTs[PIN2PORT(PINs[id][TX])]; // Getting tx port ref
	PORT_Type *const portRx = PORTs[PIN2PORT(PINs[id][RX])]; // Getting rx port ref

	portTx->PCR[pinTx] = 0x0;														  // Clear all bits
	portRx->PCR[pinRx] = 0x0;														  // Clear all bits
	portTx->PCR[pinTx] |= (PORT_PCR_MUX(PORT_mAlt3) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX
	portRx->PCR[pinRx] |= (PORT_PCR_MUX(PORT_mAlt3) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX and port interrupts

	if (config.IRQEnabled)
	{
		NVIC_EnableIRQ(UARTs_IRQ[id]);
		NVIC_SetPriority(UARTs_IRQ[id], 14);
	}

	// Enable clk gating
	switch (id)
	{
	case 0:
		SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
		break;
	case 1:
		SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;
		break;
	case 2:
		SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
		break;
	case 3:
		SIM->SCGC4 |= SIM_SCGC4_UART3_MASK;
		break;
	case 4:
		SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;
		break;
	default:
		break;
	}
	// config Parity

	uart->C1 = UART_C1_PE_MASK | UART_C1_PT(config.oddParity) | UART_C1_M_MASK;

	// Configure mode
	switch (config.uartMode)
	{
	case TX:
		uart->C2 |= UART_C2_TE_MASK;
		break;

	case RX:
		uart->C2 |= UART_C2_RE_MASK | UART_C2_RIE_MASK;
		setRxInterrupt(id, true);
		break;

	case TXRX:
		uart->C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK;
		setRxInterrupt(id, true);
		break;
	}

	SetUARTBaudrate(uart, config.baudrate);

	return true;
}

/*
 * 								WRITE SECTION
 */

uint8_t uartWriteMsg(uint8_t id, const uint8_t *msg, uint8_t cant)
{
	if (id >= UARTS_TO_USE)
	{
		return -1;
	}

	uint32_t realCant = putArray(TxQueues[id], msg, cant);
	if (!TxQueues[id]->isEmpty)
	{
		setTxInterrupt(id, true);
	}
	else
	{
		setTxInterrupt(id, false);
	}

	return realCant;
}

uint8_t uartWriteString(uint8_t id, const uint8_t *msg)
{
	if (id >= UARTS_TO_USE)
	{
		return -1;
	}

	uint32_t realCant = 0;
	for (realCant = 0; (msg[realCant] != '\0') && (!TxQueues[id]->isFull); realCant++)
	{
		put(TxQueues[id], msg[realCant]);
	}

	if (!TxQueues[id]->isEmpty)
	{
		setTxInterrupt(id, true);
	}
	else
	{
		setTxInterrupt(id, false);
	}

	return realCant;
}

static void writeTXdata_CB(uint8_t id)
{
	queuedata_t aux = getNext(TxQueues[id]);
	if (TxQueues[id]->isEmpty)
	{
		setTxInterrupt(id, false);
	}
	UARTs[id]->D = aux;
}

static void setTxInterrupt(uint8_t id, bool state)
{
	if (id >= UARTS_TO_USE)
	{
		return;
	}
	UARTs[id]->C2 = (UARTs[id]->C2 & ~(1 << 7U)) + UART_C2_TIE(state);
}

/*
 * 								READ SECTION
 */

uint32_t uartReadMsg(uint8_t id, uint8_t *msg, uint32_t cant)
{
	if (id >= UARTS_TO_USE)
	{
		return 0;
	}
	uint32_t realCount = getNextArray(RxQueues[id], msg, cant);

	if (!RxQueues[id]->isFull)
	{
		setRxInterrupt(id, true);
	}

	return realCount;
}

uint8_t uartIsRxMsg(uint8_t id)
{
	if (id >= UARTS_TO_USE)
	{
		return -1;
	}

	return !RxQueues[id]->isEmpty;
}

uint32_t uartGetRxMsgLength(uint8_t id)
{
	if (id >= UARTS_TO_USE)
	{
		return 0;
	}

	return getFillLevel(RxQueues[id]);
}

static void readRXdata_CB(uint8_t id)
{
	if (getFillLevel(RxQueues[id]) >= BUFFER_SIZE - 1)
	{
		setRxInterrupt(id, false);
	}
	put(RxQueues[id], (uint8_t)UARTs[id]->D);
}

static void setRxInterrupt(uint8_t id, bool state)
{
	if (id >= UARTS_TO_USE)
	{
		return;
	}
	UARTs[id]->C2 = (UARTs[id]->C2 & ~(1 << 5U)) + UART_C2_RIE(state);
}

/*
 * 								IRQ SECTION
 */

void UART0_RX_TX_IRQHandler(void)
{
	static uint8_t id = 0;
	static bool state = false;
	static unsigned char status = 0;

	status = UART0->S1; // Read status register
	gpioWrite(PORTNUM2PIN(PC, 5), true);
	for (uint8_t i = 0; i < 8; i++)
	{
		state = (status >> i) & 0b1;
		if (!state)
			continue;

		switch (i)
		{
		case RDRF:
			readRXdata_CB(id);
			break;

		case TDRE:
			if (UART_C2_TIE_MASK & UARTs[id]->C2)
				writeTXdata_CB(id);
			break;
		}
	}
	gpioWrite(PORTNUM2PIN(PC, 5), false);
}

void UART1_RX_TX_IRQHandler(void)
{
	static uint8_t id = 1;
	static bool state = false;
	static unsigned char status = 0;

	status = UART0->S1; // Read status register
	for (uint8_t i = 0; i < 8; i++)
	{
		state = (status >> i) & 0b1;
		if (!state)
			continue;

		switch (i)
		{
		case RDRF:
			readRXdata_CB(id);
			break;

		case TDRE:
			writeTXdata_CB(id);
			break;
		}
	}
}

void UART2_RX_TX_IRQHandler(void)
{
	static uint8_t id = 2;
	static bool state = false;
	static unsigned char status = 0;

	status = UART0->S1; // Read status register
	for (uint8_t i = 0; i < 8; i++)
	{
		state = (status >> i) & 0b1;
		if (!state)
			continue;

		switch (i)
		{
		case RDRF:
			readRXdata_CB(id);
			break;

		case TDRE:
			writeTXdata_CB(id);
			break;
		}
	}
}

void UART3_RX_TX_IRQHandler(void)
{
	static uint8_t id = 3;
	static bool state = false;
	static unsigned char status = 0;

	status = UART0->S1; // Read status register
	for (uint8_t i = 0; i < 8; i++)
	{
		state = (status >> i) & 0b1;
		if (!state)
			continue;

		switch (i)
		{
		case RDRF:
			readRXdata_CB(id);
			break;

		case TDRE:
			writeTXdata_CB(id);
			break;
		}
	}
}

void UART4_RX_TX_IRQHandler(void)
{
	static uint8_t id = 4;
	static bool state = false;
	static unsigned char status = 0;

	status = UART0->S1; // Read status register
	for (uint8_t i = 0; i < 8; i++)
	{
		state = (status >> i) & 0b1;
		if (!state)
			continue;

		switch (i)
		{
		case RDRF:
			readRXdata_CB(id);
			break;

		case TDRE:
			writeTXdata_CB(id);
			break;
		}
	}
}

/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void SetUARTBaudrate(UART_Type *uart, uint32_t baudrate)
{
	uint16_t sbr, brfa;
	uint32_t clock;

	clock = ((uart == UART0) || (uart == UART1)) ? (__CORE_CLOCK__) : (__CORE_CLOCK__ >> 1);

	baudrate = ((baudrate == 0) ? (UART_HAL_DEFAULT_BAUDRATE) : ((baudrate > 3000000) ? (UART_HAL_DEFAULT_BAUDRATE) : (baudrate)));

	sbr = clock / (baudrate << 4);				 // sbr = clock/(Baudrate x 16)
	brfa = (clock << 1) / baudrate - (sbr << 5); // brfa = 2*Clock/baudrate - 32*sbr

	uart->BDH = UART_BDH_SBR(sbr >> 8);
	uart->BDL = UART_BDL_SBR(sbr);
	uart->C4 = (uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);
}

/*******************************************************************************
 ******************************************************************************/
