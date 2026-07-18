/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <gpio.h>
#include <math.h>
#include <stdlib.h>
#include "board.h"
#include "canspi.h"
#include "Acelerometro.h"
#include "uart.h"
#include "timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define OURBOARD (2)
#define GROUPS (6)
#define MAXCOUNT (40) // Cantidad de ciclos de 50ms para volver a actualizar si o si
#define MINTIME (50)  // In ms
#define RAD2DEG(x) ((x) * 180.0f / (3.1416f))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define CAN_ID 0x103

typedef struct sensorReading
{
	int16_t pitch;
	int16_t roll;
	int16_t orientation;
} sensorReading_t;

static uart_cfg_t uartConfig = {
	.IRQEnabled = true,
	.oddParity = true,
	.baudrate = 9600,
	.uartMode = TXRX};

static uint8_t timerMinID;
static uint8_t timerWaitID;
static sensorReading_t boardReading[GROUPS];				// Datos de cada board
static uint8_t yawCount = 0, rollCount = 0, pitchCount = 0; // Contamos la cantidad de 50ms que pasaron desde la ultima actualizacion del eje

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void fromAccToAngle(SRAWDATA *pAccelData, SRAWDATA *pMagnData);
static bool changedMoreThan5Degrees(int16_t axis, int16_t curr);
static void update();

/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	initAccelerometer();
	initTimer();
	uartInit(0, uartConfig);
	initCAN();

	timerMinID = createTimer(MINTIME, &update, PERIODIC);
	timerWaitID = createTimer(1, &update, ONESHOT);
	startTimer(timerMinID);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	static CANMsg_t aux;
	static char groupStr[] = "G3\n";
	static char lineScape = '\n';

	if (canRxmsgAvailable())
	{
		readCanMsg(&aux, 1);
		groupStr[1] = 0x30 + (aux.ID & 0x7);
		uartWriteString(0, groupStr);
		uartWriteMsg(0, aux.data, aux.length);
		uartWriteMsg(0, &lineScape, 1);
	}

}

void delay(uint32_t cont)
{
	while (cont--);
}

/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void update()
{
	static SRAWDATA Adata, Mdata;
	static sensorReading_t lastRead;
	static CANMsg_t auxMsg = {CAN_ID, 0, {0}};
	static char groupStr[] = "G3\n";
	static char lineScape = '\n';

	ReadAccelMagnData(&Adata, &Mdata);
	fromAccToAngle(&Adata, &Mdata);

	// Guardamos los strings de la informacion

	// Preguntamos si alguno de los ejes cambio mas de 5 grados desde la ultima actualizacion
	if (changedMoreThan5Degrees(boardReading[OURBOARD].orientation, lastRead.orientation) || yawCount >= MAXCOUNT)
	{
		itoa(boardReading[OURBOARD].orientation, auxMsg.data + 1, 10);
		auxMsg.data[0] = 'O';
		uint8_t i = 0;
		for (; auxMsg.data[i] != '\0'; i++);
		auxMsg.length = i;
		if(canisBusy()){
			startTimer(timerWaitID);
		}
		else{
			sendCanMsg(auxMsg);
			uartWriteString(0, groupStr);
			uartWriteString(0, auxMsg.data);
			uartWriteMsg(0, &lineScape, 1);
			yawCount = 0;
		}
	}

	if (changedMoreThan5Degrees(boardReading[OURBOARD].pitch, lastRead.pitch) || pitchCount >= MAXCOUNT)
	{
		itoa(boardReading[OURBOARD].pitch, auxMsg.data + 1, 10);
		auxMsg.data[0] = 'C';
		uint8_t i = 0;
		for (; auxMsg.data[i] != '\0'; i++)
			;
		auxMsg.length = i;
		if(canisBusy()){
			startTimer(timerWaitID);
		}
		else{
			sendCanMsg(auxMsg);
			uartWriteString(0, groupStr);
			uartWriteString(0, auxMsg.data);
			uartWriteMsg(0, &lineScape, 1);
			pitchCount = 0;
		}
	}

	if (changedMoreThan5Degrees(boardReading[OURBOARD].roll, lastRead.roll) || rollCount >= MAXCOUNT)
	{
		itoa(boardReading[OURBOARD].roll, auxMsg.data + 1, 10);
		auxMsg.data[0] = 'R';
		uint8_t i = 0;
		for (; auxMsg.data[i] != '\0'; i++)
			;
		auxMsg.length = i;
		if(canisBusy()){
			startTimer(timerWaitID);
		}
		else{
			sendCanMsg(auxMsg);
			uartWriteString(0, groupStr);
			uartWriteString(0, auxMsg.data);
			uartWriteMsg(0, &lineScape, 1);
			rollCount = 0;
		}
	}

	// Actualizamos la posicion actual de los ejes de nuestra placa
	lastRead = boardReading[OURBOARD];
	yawCount++;
	pitchCount++;
	rollCount++;
}

static bool changedMoreThan5Degrees(int16_t axis, int16_t curr)
{

	bool changed = false;

	if (ABS(axis - curr) > 5)
	{
		changed = true;
	}

	return changed;
}

static void fromAccToAngle(SRAWDATA *pAccelData, SRAWDATA *pMagnData)
{

	double x = pAccelData->x;
	double y = pAccelData->y;
	double z = pAccelData->z;
	boardReading[OURBOARD].roll = RAD2DEG(atan2(x, sqrt(pow(y, 2) + pow(z, 2))));
	boardReading[OURBOARD].pitch = RAD2DEG(atan2(y, sqrt(pow(x, 2) + pow(z, 2))));
	boardReading[OURBOARD].orientation = RAD2DEG(atan2(y, x));
	if (boardReading[OURBOARD].orientation < 0)
	{
		boardReading[OURBOARD].orientation += 360;
	}
}

/*******************************************************************************
 ******************************************************************************/
