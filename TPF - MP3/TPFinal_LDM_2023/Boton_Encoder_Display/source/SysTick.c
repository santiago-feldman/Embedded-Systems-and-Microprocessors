
#include "SysTick.h"
#include "MK64F12.h"
#include "hardware.h"

static void (*periodicCallback)(void);

#define NORM (100)

bool SysTick_Init (void (*funcallback)(void)){

	bool success = false;
	uint32_t ticks = (__CORE_CLOCK__/NORM) * EVEDELAY;

	if (SysTick_Config(ticks) == 0){
		NVIC_EnableIRQ(SysTick_IRQn);
		periodicCallback = funcallback;
		success = true;
	}

	return success;
}

