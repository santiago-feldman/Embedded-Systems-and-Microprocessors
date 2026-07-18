/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "I2C.h"
#include "MK64F12.h"
#include "hardware.h"
#include "config.h"
#include "Queue.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_TO_USE 2
#define I2C_DEFAULT_BAUDRATE 400000
#define BUS_CLK 50000000U

#define GET_PORT_BASE_PTRS(id) (PORT_BASE_PTRS[id])

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
	TCF = 7,
	IAAS = 6,
	BUSY = 5,
	ARBL = 4,
	RAM = 3,
	SRW = 2,
	IICIF = 1,
	RXAK = 0
}; // UART STATUS REGISTER BITS

/*******************************************************************************
 * GLOBAL VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static I2C_Type *const I2Cs[] = I2C_BASE_PTRS;
static I2CMode_t I2Cmode[2] = {0};
static uint32_t I2CRxCants[2] = {0};
static const IRQn_Type I2Cs_IRQ[] = I2C_IRQS;
static PORT_Type *const PORTs[] = PORT_BASE_PTRS;
static const pin_t PINs[5][2] = I2C_PINS;

#if I2C_TO_USE >= 1
static queue_t RX0queue = {{0}, 0, 0, false, true};
static queue_t TX0queue = {{0}, 0, 0, false, true};
#endif
#if I2C_TO_USE >= 2
static queue_t RX1queue = {{0}, 0, 0, false, true};
static queue_t TX1queue = {{0}, 0, 0, false, true};
#endif

#if I2C_TO_USE == 1
static queue_t *const TxQueues[] = {
	&TX0queue,
};

static queue_t *const RxQueues[] = {
	&RX0queue,
};
#endif
#if I2C_TO_USE == 2
static queue_t *const TxQueues[] = {
	&TX0queue,
	&TX1queue};

static queue_t *const RxQueues[] = {
	&RX0queue,
	&RX1queue};
#endif

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void SetI2Caudrate(I2C_Type *uart, uint32_t baudrate);

/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool I2CInit(uint8_t id, uint8_t addr, uint32_t baudrate)
{
	if (id >= I2C_TO_USE)
	{
		return false;
	}

	uint8_t pinSCL = PIN2NUM(PINs[id][0]);					 // Getting tx pin ref
	uint8_t pinSDA = PIN2NUM(PINs[id][1]);					 // Getting rx pin ref
	I2C_Type *const i2c = I2Cs[id];							 // Getting current UART_Type ptr
	PORT_Type *const portSCL = PORTs[PIN2PORT(PINs[id][0])]; // Getting tx port ref
	PORT_Type *const portSDA = PORTs[PIN2PORT(PINs[id][1])]; // Getting rx port ref

	portSCL->PCR[pinSCL] = 0x0; // Clear all bits
	portSDA->PCR[pinSDA] = 0x0; // Clear all bits
	if (id == 0)
	{
		portSCL->PCR[pinSCL] |= (PORT_PCR_MUX(PORT_mAlt5) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX
		portSDA->PCR[pinSDA] |= (PORT_PCR_MUX(PORT_mAlt5) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX and port interrupts
	}
	else
	{
		portSCL->PCR[pinSCL] |= (PORT_PCR_MUX(PORT_mAlt2) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX
		portSDA->PCR[pinSDA] |= (PORT_PCR_MUX(PORT_mAlt2) | PORT_PCR_IRQC(PORT_eDisabled)); // Set MUX and port interrupts
	}

	switch (id)
	{
	case 0:
		SIM->SCGC4 |= SIM_SCGC4_I2C0(1);
		break;
	case 1:
		SIM->SCGC4 |= SIM_SCGC4_I2C1(1);
		break;
	}

	NVIC_EnableIRQ(I2Cs_IRQ[id]);
	SetI2Caudrate(i2c, baudrate);
	i2c->A1 = I2C_A1_AD(addr);
	i2c->C1 |= I2C_C1_IICEN(1) | I2C_C1_IICIE(1);

	return true;
}

bool isI2CBusBusy(uint8_t id)
{
	return I2Cs[id]->S & I2C_S_BUSY_MASK;
}

/*
 * 								WRITE SECTION
 */

uint32_t i2cWriteSlave(uint8_t id, uint8_t slave_addr, uint8_t *msg, uint32_t cant)
{
	if (id >= I2C_TO_USE || (I2Cs[id]->S & I2C_S_BUSY_MASK))
	{
		return 0;
	}

	uint32_t realCant = 0;
	slave_addr = (slave_addr << 1) & 0b11111110;
	realCant = putArray(TxQueues[id], msg, cant);

	I2Cmode[id] = MTX;

	I2Cs[id]->C1 |= I2C_C1_TX_MASK; // Modo Tx
	I2Cs[id]->C1 |= I2C_C1_MST_MASK;
	I2Cs[id]->D = slave_addr;

	return realCant;
}

uint8_t i2cIsTxMsgComplete(uint8_t id)
{
	return TxQueues[id]->isEmpty;
}

uint8_t i2cWriteSlaveReg(uint8_t id, uint8_t slave_addr, const uint32_t addr, uint8_t *data, uint32_t cant)
{
	if (id >= I2C_TO_USE || (I2Cs[id]->S & I2C_S_BUSY_MASK))
	{
		return 0;
	}

	uint32_t realCant = 0;
	slave_addr = (slave_addr << 1) & 0b11111110;
	put(TxQueues[id], addr);
	realCant = putArray(TxQueues[id], data, cant);

	I2Cmode[id] = MTX;

	I2Cs[id]->C1 |= I2C_C1_TX_MASK | I2C_C1_MST_MASK; // Modo Tx
	I2Cs[id]->D = slave_addr;

	return realCant;
}
/*
 * 								READ SECTION
 */
uint8_t i2cRequestSlaveRegData(uint8_t id, uint8_t slave_addr, const uint32_t addr, uint32_t cant)
{
	if (id >= I2C_TO_USE || (I2Cs[id]->S & I2C_S_BUSY_MASK))
	{
		return 0;
	}
	I2CRxCants[id] = cant;
	uint8_t Rx_slave_addr = (slave_addr << 1) | 0b1;
	slave_addr = (slave_addr << 1) & 0b11111110;

	put(TxQueues[id], addr);
	put(TxQueues[id], Rx_slave_addr);

	I2Cmode[id] = MTXRX;

	I2Cs[id]->C1 |= I2C_C1_TX_MASK | I2C_C1_MST_MASK; // Modo Tx

	I2Cs[id]->D = slave_addr;

	return true;
}

uint32_t i2cReadSlave(uint8_t id, uint8_t slave_addr, uint32_t cant)
{
	if (id >= I2C_TO_USE || (I2Cs[id]->S & I2C_S_BUSY_MASK))
	{
		return 0;
	}
	I2CRxCants[id] = cant;
	slave_addr = (slave_addr << 1) | 0b00000001;

	I2Cmode[id] = MRX;

	I2Cs[id]->C1 |= I2C_C1_TX_MASK; // Modo Tx
	I2Cs[id]->C1 |= I2C_C1_MST_MASK;
	I2Cs[id]->D = slave_addr;

	return true;
}

uint32_t i2cReadMsg(uint8_t id, uint8_t *msg, uint32_t cant)
{
	uint32_t realCant = getNextArray(RxQueues[id], msg, cant);
	return realCant;
}

uint32_t i2cGetRxMsgLength(uint8_t id)
{
	return getFillLevel(RxQueues[id]);
}

uint8_t i2cIsRxMsg(uint8_t id)
{
	return !RxQueues[id]->isEmpty;
}

/*
 * 								IRQ SECTION
 */
 void I2C0_IRQHandler(void)
 {
 	static uint8_t id = 0;

 	BITBAND_REG8(I2Cs[id]->S, I2C_S_IICIF_SHIFT) = 1; // Clear interrupt

 	if (I2Cs[id]->C1 & I2C_C1_MST_MASK) // If on master Mode
 	{
 		if (I2Cs[id]->C1 & I2C_C1_TX_MASK) // If Tx
 		{
 			if ((TxQueues[id]->isEmpty && I2Cmode[id] == MTX) || (I2Cs[id]->S & I2C_S_RXAK_MASK) != 0) // Ya se mandaron todos los datos en modo Tx o fallo el ack
 			{
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_MST_SHIFT) = 0;
 				return;
 			}

 			if (TxQueues[id]->isEmpty) // Se mando el address, ya sea para el caso de Rx o TxRx (para este ultimo se corresponde con el segundo envio del address)
 			{
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_TX_SHIFT) = 0; // Cambio a modo Rx para quedar ahi.
 				if (I2CRxCants[id] == 1)
 					BITBAND_REG8(I2Cs[id]->C1, I2C_C1_TXAK_SHIFT) = 1;
 				else
 					BITBAND_REG8(I2Cs[id]->C1, I2C_C1_TXAK_SHIFT) = 0;
 				uint8_t dummyRead = I2Cs[id]->D;
 				return;
 			}

 			if (I2Cmode[id] == MTXRX && getFillLevel(TxQueues[id]) == 1) // Si quiero hacer repeated start tengo que mandarlo cuando vaya a pasar nuevamente el address del slvae
 			{
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_RSTA_SHIFT) = 1;
 			}
 			I2Cs[id]->D = getNext(TxQueues[id]); // Mando el datazo
 			return;
 		}
 		else // If Rx
 		{
 			if (I2CRxCants[id] == 1) // If last byte to read
 			{
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_MST_SHIFT) = 0;
 			}
 			else if (I2CRxCants[id] == 2) // Si todavia faltan bytes para leer
 			{
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_TXAK_SHIFT) = 1;
 			}
 			uint8_t aux = I2Cs[id]->D;
 			put(RxQueues[id], aux);
 			I2CRxCants[id]--;
 			return;
 		}
 	}
 	else // If not on master Mode
 	{
 		if (I2Cs[id]->S & I2C_S_ARBL_MASK) // Se perdio control del Bus
 		{
 			BITBAND_REG8(I2Cs[id]->S, I2C_S_ARBL_SHIFT) = 1;
 			if (!(I2Cs[id]->S & I2C_S_IAAS_MASK)) // No me ablaron );
 			{
 				return;
 			}
 		}
 		if (I2Cs[id]->S & I2C_S_IAAS_MASK) // Me hablaron
 		{
 			if (I2Cs[id]->S & I2C_S_SRW_MASK) // Me pidieron transmitir
 			{
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_TX_SHIFT) = 1; // Modo Tx
 				I2Cs[id]->D = 1;								 // Mando 1nos algun dia aca habra algo
 			}
 			else // Me pidieron recibir
 			{
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_TX_SHIFT) = 0; // Modo Rx
 				uint8_t dummyRead = I2Cs[id]->D;
 			}
 			return; // Espero a la proxima
 		}
 		else
 		{
 			if (I2Cs[id]->C1 & I2C_C1_TX_MASK) // Si estoy para mandar
 			{
 				if (I2Cs[id]->S & I2C_S_RXAK_MASK) // Recibi el ACK para seguir mandando datos
 				{
 					I2Cs[id]->D = 1; // Mando 1nos algun dia aca habra algo
 					return;
 				}
 				// Se pincho todo, volvemos a estar en Rx.
 				BITBAND_REG8(I2Cs[id]->C1, I2C_C1_TX_SHIFT) = 0; // Modo Rx
 				uint8_t dummyRead = I2Cs[id]->D;
 			}
 			else // Estoy para recibir
 			{
 				uint8_t aux = I2Cs[id]->D;
 				put(RxQueues[id], aux); // Guardo y sigo
 				return;
 			}
 		}
 	}
 }

 void I2C1_IRQHandler(void)
 {
 	static uint8_t id = 1;
 	static uint32_t startCount = 0;

 	if (I2Cs[id]->FLT & I2C_FLT_STOPF_MASK) // StopFlag up
 	{
 		startCount = 0;
 		I2Cs[id]->FLT &= ~I2C_FLT_STOPF_MASK;
 		BITBAND_REG8(I2Cs[id]->S, I2C_S_IICIF_SHIFT) = 1; // Clear interrupt
 	}
 	else // StopFlag down
 	{
 		if (I2Cs[id]->FLT & I2C_FLT_STARTF_MASK) // StartFlag up
 		{
 			I2Cs[id]->FLT &= ~I2C_FLT_STARTF_MASK;

 			startCount++;
 			if (startCount > 1)
 			{
 				BITBAND_REG8(I2Cs[id]->S, I2C_S_IICIF_SHIFT) = 1; // Clear interrupt
 				return;
 			}
 		}
 		BITBAND_REG8(I2Cs[id]->S, I2C_S_IICIF_SHIFT) = 1; // Clear interrupt
 		if (I2Cs[id]->C1 & I2C_C1_MST_MASK)				  // If on master Mode
 		{
 			if (I2Cs[id]->C1 & I2C_C1_TX_MASK) // If Tx
 			{
 				if ((TxQueues[id]->isEmpty && I2Cmode[id] == MTX) || (I2Cs[id]->S & I2C_S_RXAK_MASK)) // Ya se mandaron todos los datos en modo Tx o fallo el ack
 				{
 					I2Cs[id]->C1 &= ~I2C_C1_MST_MASK;
 					return;
 				}

 				if (TxQueues[id]->isEmpty) // Se mando el address, ya sea para el caso de Rx o TxRx (para este ultimo se corresponde con el segundo envio del address)
 				{
 					I2Cs[id]->C1 &= ~I2C_C1_TX_MASK; // Cambio a modo Rx para quedar ahi.
 					uint8_t dummyRead = I2Cs[id]->D;
 				}

 				if (I2Cmode[id] == MTXRX && getFillLevel(TxQueues[id]) == 1) // Si quiero hacer repeated start tengo que mandarlo cuando vaya a pasar nuevamente el address del slvae
 				{
 					I2Cs[id]->C1 |= I2C_C1_RSTA_MASK;
 				}

 				I2Cs[id]->D = getNext(TxQueues[id]); // Mando el datazo
 			}
 			else // If Rx
 			{
 				if (I2CRxCants[id] == 1) // If last byte to read
 				{
 					I2Cs[id]->C1 &= ~I2C_C1_MST_MASK;
 				}
 				else if (I2CRxCants[id] > 1) // Si todavia faltan bytes para leer
 				{
 					I2Cs[id]->C1 |= I2C_C1_TXAK_MASK;
 				}

 				put(RxQueues[id], I2Cs[id]->D);
 				I2CRxCants[id]--;
 			}
 		}
 		else // If not on master Mode
 		{
 			if (I2Cs[id]->S & I2C_S_ARBL_MASK) // Se perdio control del Bus
 			{
 				I2Cs[id]->S |= I2C_S_ARBL_MASK;
 				if (!(I2Cs[id]->S & I2C_S_IAAS_MASK)) // No me ablaron );
 				{
 					return;
 				}
 			}
 			if (I2Cs[id]->S & I2C_S_IAAS_MASK) // Me hablaron
 			{
 				if (I2Cs[id]->S & I2C_S_SRW_MASK) // Me pidieron transmitir
 				{
 					I2Cs[id]->C1 |= I2C_C1_TX_MASK; // Modo Tx
 					I2Cs[id]->D = 1;				// Mando 1nos algun dia aca habra algo
 				}
 				else // Me pidieron recibir
 				{
 					I2Cs[id]->C1 &= ~I2C_C1_TX_MASK; // Modo Rx
 					uint8_t dummyRead = I2Cs[id]->D;
 				}
 				return; // Espero a la proxima
 			}
 			else
 			{
 				if (I2Cs[id]->C1 & I2C_C1_TX_MASK) // Si estoy para mandar
 				{
 					if (I2Cs[id]->S & I2C_S_RXAK_MASK) // Recibi el ACK para seguir mandando datos
 					{
 						I2Cs[id]->D = 1; // Mando 1nos algun dia aca habra algo
 						return;
 					}
 					// Se pincho todo, volvemos a estar en Rx.
 					I2Cs[id]->C1 &= ~I2C_C1_TX_MASK; // Modo Rx
 					uint8_t dummyRead = I2Cs[id]->D;
 				}
 				else // Estoy para recibir
 				{
 					put(RxQueues[id], I2Cs[id]->D); // Guardo y sigo
 					return;
 				}
 			}
 		}
 	}
 }

/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void SetI2Caudrate(I2C_Type *i2c, uint32_t baudrate)
{
	uint8_t mul = 0b10; // multiplicador de 4
	uint8_t ICR = 0;

	baudrate = ((baudrate == 0) ? (I2C_DEFAULT_BAUDRATE) : ((baudrate < 200000U) ? (I2C_DEFAULT_BAUDRATE) : (baudrate)));
	ICR = BUS_CLK / (4 * baudrate);
	i2c->F |= I2C_F_ICR(0x09) | I2C_F_MULT(mul);
}

/*******************************************************************************
 ******************************************************************************/
