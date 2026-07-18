/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "FTM.h"
#include "DMA.h"
#include "MK64F12.h"
#include "peripherals.h"
#include "fsl_edma.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "fsl_edma.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

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
/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
						GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static uint16_t dummyTable[1] = {0};
uint16_t ticksPerPeriod = 100;
uint16_t *waveform = dummyTable;
uint32_t waveform_lenght = 0;
uint32_t waveform_offset = 0;
static void (*Table_end_cb)(void) = NULL;

void PWM_Init(void)
{
	DMA_Init();
	FTM_Init();

	PORTC->PCR[1] = PORT_PCR_MUX(PORT_mAlt4) | PORT_PCR_DSE(true) | PORT_PCR_IRQC(PORT_eDisabled);

	//  Set FTM configuration
	FTM_SetPrescaler(FTM0, FTM_PSC_x1);
	FTM_SetInterruptMode(FTM0, FTM_CH_0, false); // Enable interrupts

	//	Set FTM as PWM mode
	FTM_SetWorkingMode(FTM0, FTM_CH_0, FTM_mPulseWidthModulation);		 // MSA  / B
	FTM_SetPulseWidthModulationLogic(FTM0, FTM_CH_0, FTM_lAssertedHigh); // ELSA / B

	//  Set PWWM Modulus and initial Duty
	FTM_SetModulus(FTM0, ticksPerPeriod);
	FTM_SetCounter(FTM0, FTM_CH_0, ticksPerPeriod / 2);

	//  Enable FTM0-CH0 DMA Request

	// FTM_StartClock(FTM0);
}
void PWM_SetDC(float DC)
{
	FTM_SetCounter(FTM0, FTM_CH_0, ticksPerPeriod * (DC / 100.0f));
}
float PWM_GetDC()
{
	return ((float)FTM_GetCounter(FTM0, FTM_CH_0)) / ((float)ticksPerPeriod);
}

void PWM_SetTickPerPeriod(uint16_t tPerPeriod)
{
	FTM_StopClock(FTM0);
	float DC = PWM_GetDC();

	if (ticksPerPeriod > tPerPeriod)
	{
		ticksPerPeriod = tPerPeriod;
		PWM_SetDC(DC);
		FTM_SetModulus(FTM0, ticksPerPeriod);
	}
	else
	{
		ticksPerPeriod = tPerPeriod;
		FTM_SetModulus(FTM0, ticksPerPeriod);
		PWM_SetDC(DC);
	}

}
uint16_t PWM_GetTickPerPeriod()
{
	return ticksPerPeriod;
}

void PWM_GenWaveform(uint16_t *waveform_pointer, uint32_t wave_length, uint32_t waveTable_offset)
{
	waveform = waveform_pointer;
	waveform_lenght = wave_length;
	waveform_offset = waveTable_offset;

	FTM_StopClock(FTM0);
	FTM_DmaMode(FTM0, FTM_CH_0, true);
	FTM_SetInterruptMode(FTM0, FTM_CH_0, true);


	 DMA_SetSourceModulo(DMA_CH0, 0);
	 DMA_SetDestModulo(DMA_CH0, 0);

	 DMA_SetSourceAddr(DMA_CH0, (uint32_t)waveform);
	 DMA_SetDestAddr(DMA_CH0, (uint32_t) & (FTM0->CONTROLS[FTM_CH_0].CnV));

	 DMA_SetSourceAddrOffset(DMA_CH0, waveform_offset * 2);
	 DMA_SetDestAddrOffset(DMA_CH0, 0);

	 DMA_SetSourceLastAddrOffset(DMA_CH0, -2 * (int32_t)(waveform_lenght - waveform_offset));
	 DMA_SetDestLastAddrOffset(DMA_CH0, 0);

	 DMA_SetSourceTransfSize(DMA_CH0, DMA_TransSize_16Bit);
	 DMA_SetDestTransfSize(DMA_CH0, DMA_TransSize_16Bit);

	 DMA_SetMinorLoopTransCount(DMA_CH0, 2);

	 DMA_SetCurrMajorLoopCount(DMA_CH0, waveform_lenght / waveform_offset - 1);
	 DMA_SetStartMajorLoopCount(DMA_CH0, waveform_lenght / waveform_offset - 1);

	 DMA_SetEnableRequest(DMA_CH0, true);

	 DMAMUX_ConfigChannel(DMA_CH0, true, false, kDmaRequestMux0FTM0Channel0);
	 DMA_SetChannelInterrupt(DMA_CH0, true, NULL);
	 DMA_StartTransfer(DMA_CH0);

	FTM_ClearInterruptFlag(FTM0, FTM_CH_0);
	FTM_ClearOverflowFlag(FTM0);
}

void PWM_burst(void)
{
	FTM_StartClock(FTM0);
}
/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void DMA_callback_CH0(edma_handle_t *, void *, bool, uint32_t)
{
	FTM_StopClock(FTM0);

}
