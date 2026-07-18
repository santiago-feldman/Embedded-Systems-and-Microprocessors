/***************************************************************************/ /**
   @file     canspi.c
   @brief
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "canspi.h"
#include "canspidefs.h"
#include "QueueCAN.h"
#include "Timer.h"
#include "spi.h"
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define RX_INT_PIN PORTNUM2PIN(PD, 0)

#define RXB0D0 0b01100110
#define TXB0D0 0b00110110

// MASKS
#define RXB0MSK 0b00000001
#define RXB1MSK 0b00000010
#define TXB0MSK 0b00000100
#define TXB1MSK 0b00001000
#define TXB2MSK 0b00010000

#define SIDHMSK 0X07F8
#define SIDLMSK 0x0007

#define FILTER_ON 0b01100000

#define DONT_CARE 0b00000000

#define READ_SIZE 3
#define MAXTXBUFFERS 3 // Esta el 0, el 1 y el 2
#define MAXRXBUFFERS 2

#define BYTELEN(x) (sizeof(x) / sizeof((uint8_t)(x[0])))

/*******************************************************************************
 * GLOBAL VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static canqueue_t RxQueue = {{0, 0, {0}}};
static uint8_t timerID = 0;
static bool usingCan = false;
static spi_cfg_t config = {
	.highPolarity = false,
	.highPhase = false,
	.async = true,
	.baudrate = 0,
	.spiMode = MASTER};

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void write(uint8_t data, uint8_t address);
static uint8_t read_inst(uint8_t address);
static void bit_modify(uint8_t data, uint8_t mask, uint8_t address);
static void request_to_send();
static void tx_buffer_enable(TXBuffer_t txbuff_num, bool state);
static void rx_buffer_enable(RXBuffer_t rxbuff_num, bool state);
static void clear_tx_buffer_flag(TXBuffer_t txbuff_num);
static void clear_rx_buffer_flag(RXBuffer_t rxbuff_num);
static void reset();
static void request_to_send();
static void set_mask_or_filter(RXAcceptance_t acceptance_byte_num, uint16_t acceptance_byte);
static void choose_filters(RXBuffer_t buffer, RXAcceptance_t filter_num);
static void setID(uint8_t address);

static void auxiliary();
static void RX_Callback();
static void parseCanFrames();
static void buildFrame(uint8_t *frame, CANMsg_t *data);
/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*
Modo 0,0 es una configuracion del periferico SPI de la Kinetis. Y refiere a en que momento se cambian (se lachtean)
los datos en función de los flancos del clock. En principio pareceria que el nuestro está bien pero tenemos el
incoveniente de que el dato aparece inestable al comienzo de la transmision, eso es un tema de configuracion
del spi, podria ser algo de "trailing edge".
*/

void initCAN()
{
	static uint8_t aux[4] = {0};
	timerID = createTimer(1, &auxiliary, ONESHOT);
	// Configuramos el SPI en modo 0,0
	spiInit(config);

	gpioMode(RX_INT_PIN, INPUT_PULLDOWN);
	gpioIRQ(RX_INT_PIN, GPIO_IRQ_MODE_FALLING_EDGE, &auxiliary);

	aux[0] = RESET;
	spiWriteOnlyMsg(&aux, 1, 0);
	while (!spiIsTxMsgComplete())
		;

	// Ponemos la placa en modo configuracion
	/*	aux[0] = WRITE;
		aux[1] = CANCTRL;
		aux[3] = 0b10000000;
		spiWriteOnlyMsg(&aux, 3, 0);
		while (!spiIsTxMsgComplete())
			;*/
	// Configuramos el bit de tiempo
	aux[0] = WRITE;
	aux[1] = CNF1;
	aux[2] = 0x03; // Pongo el PBR=63
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;

	aux[0] = WRITE;
	aux[1] = CNF2;
	aux[2] = 0b10110001;
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;

	aux[0] = WRITE;
	aux[1] = CNF3;
	aux[2] = 0b10000101;
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;
	// Borro los flags y habilito las interrupciones

	aux[0] = WRITE;
	aux[1] = RXB0CTRL;
	aux[2] = 0b01100000;
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;

	aux[0] = WRITE;
	aux[1] = RXB1CTRL;
	aux[2] = 0b01100000;
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;

	aux[0] = WRITE;
	aux[1] = CANINTE;
	aux[2] = 0b00000011; // Habilito interrupciones para llegada de mensaje
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;
	aux[0] = WRITE;
	aux[1] = CANINTF;
	aux[2] = 0b00000000; // Habilito interrupciones para llegada de mensaje
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;

	aux[0] = WRITE;
	aux[1] = CANCTRL;
	aux[2] = 0b00000000; // Normal - 0b00000000 ; Loopback - 0b01000000
	spiWriteOnlyMsg(&aux, 3, 0);
	while (!spiIsTxMsgComplete())
		;
}

/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/

bool canisBusy()
{
	return usingCan;
}

uint8_t readCanMsg(CANMsg_t *msg, uint8_t cant)
{
	return cangetNextArray(&RxQueue, msg, cant);
}

uint8_t canRxmsgAvailable()
{
	return cangetFillLevel(&RxQueue);
}

static void auxiliary(void)
{
	if (canisBusy())
	{
		startTimer(timerID);
		return;
	}
	RX_Callback();
}

static void RX_Callback()
{
	static uint8_t state = 0;
	static uint8_t auxmsg[3] = {0};
	static uint8_t interruptState = 0;

	switch (state)
	{
	case 0:
		usingCan = true;
		auxmsg[0] = READ;
		auxmsg[1] = CANINTF;
		spiWriteReadMsg(auxmsg, 2, 1, &RX_Callback);
		state++;
		break;
	case 1:
		spiReadMsg(&interruptState, 1);
		auxmsg[0] = WRITE;
		auxmsg[1] = CANINTF;
		auxmsg[2] = 0;
		spiWriteOnlyMsg(auxmsg, 3, &RX_Callback);
		state++;
		break;
	case 2:
		if ((interruptState & 0b11) != 0b11)
		{
			if ((interruptState & 0b11) == 1)
			{
				auxmsg[0] = READ_RX_BUFFER;

				auxmsg[0] = READ;
				auxmsg[1] = RXB0SIDH;
				spiWriteReadMsg(auxmsg, 2, 13, &RX_Callback);
				state++;
			}
			else if ((interruptState & 0b11) == 0b10)
			{
				auxmsg[0] = READ;
				auxmsg[1] = RXB1SIDH;
				spiWriteReadMsg(auxmsg, 2, 13, &RX_Callback);
				state++;
			}
		}
		else if ((interruptState & 0b11) == 0b11)
		{
			auxmsg[0] = READ;
			auxmsg[1] = RXB0SIDH;
			spiWriteReadMsg(auxmsg, 2, 13, &RX_Callback);
			state = 4;
		}
		else
		{
			state = 0;
			usingCan = false;
		}
		break;
	case 3:
		parseCanFrames();
		state = 0;
		usingCan = false;
		break;
	case 4:
		auxmsg[0] = READ;
		auxmsg[1] = RXB1SIDH;
		spiWriteReadMsg(auxmsg, 2, 13, &RX_Callback);
		state = 3;
		break;
	default:
		usingCan = false;
		state = 0;
		break;
	}
}

void parseCanFrames()
{
	static uint8_t auxFrame[13] = {0};
	static CANMsg_t auxData;

	for (uint8_t i = 0; i < spiGetRxMsgLength() / 13; i++)
	{
		spiReadMsg(auxFrame, 13);
		auxData.ID = (auxFrame[0] << 3) + ((auxFrame[1] & 0b11100000) >> 5);
		auxData.length = auxFrame[4] & 0xF;
		for (uint8_t i = 0; i < auxData.length; i++)
		{
			auxData.data[i] = auxFrame[i + 5];
		}
		canput(&RxQueue, auxData);
	}
}

void sendCanMsg(CANMsg_t msg)
{
	static uint8_t auxFrame[14] = {0};
	static uint8_t usedTXbuff = 0;
	static uint8_t auxmsg[3] = {0};
	static uint8_t status = 0;
	static uint8_t state = 0;

	switch (state)
	{
	case 0:
		usingCan = true;
		buildFrame(auxFrame + 1, &msg);
		auxmsg[0] = READ_STATUS;
		spiWriteReadMsg(auxmsg, 1, 1, &sendCanMsg);
		state++;
		break;
	case 1:
		spiReadMsg(&status, 1);
		if (status ^ 0b01010100)
		{
			if (!(status & 0b01000000)) // TXBuff2
			{
				auxFrame[0] = LOAD_TX_BUFFER + 0b100;
				spiWriteOnlyMsg(auxFrame, 14, &sendCanMsg);
				usedTXbuff |= 0b100;
				state++;
			}
			else if (!(status & 0b00010000)) // TXBuff1
			{
				auxFrame[0] = LOAD_TX_BUFFER + 0b010;
				spiWriteOnlyMsg(auxFrame, 14, &sendCanMsg);
				usedTXbuff |= 0b010;
				state++;
			}
			else if (!(status & 0b00000100)) // TXBuff0
			{
				auxFrame[0] = LOAD_TX_BUFFER + 0b001;
				spiWriteOnlyMsg(auxFrame, 14, &sendCanMsg);
				usedTXbuff |= 0b001;
				state++;
			}
		}
		else
		{
			usedTXbuff = 0b111;
			state = 2;
		}
		break;
	case 2:
		auxmsg[0] = RTS + usedTXbuff;
		spiWriteOnlyMsg(auxmsg, 1, &sendCanMsg);
		state++;
		break;
	case 3:
		usingCan = false;
		state = 0;
		break;
	default:
		usingCan = false;
		state = 0;
		break;
	}
}
// Frame: {TXBnIDH, TXBnIDL, TXBnID8, TXBnID0, TXBnDLC, TXBnD0 ... TXBnD7}
static void buildFrame(uint8_t *frame, CANMsg_t *data)
{
	frame[0] = (data->ID >> 3) & 0xFF;
	frame[1] = (data->ID << 5) & 0xE0;
	frame[2] = 0;
	frame[3] = 0;
	frame[4] = data->length & 0x0F;
	for (uint8_t i = 0; i < 8; i++)
	{
		frame[5 + i] = data->data[i];
	}
}
