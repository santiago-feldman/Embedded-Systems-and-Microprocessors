#include "encoder.h"
#include <hardware.h>
#include "timer.h"
#include "gpio.h"
#include "config.h"
#include <stdlib.h>
#include "Queue.h"
#include "board.h"
#include "eventQueue.h"

#define SW_ACTIVE LOW
#define ROT_ACTIVE LOW

#define SW_PERIODIC_SAMPLING 1

#define DEBOUNCE_TIME 10
#define SW_SAMPLING_TIME 10
#define DOUBLECLIK_TIME 350
#define ENCODER_ROT_TIME 100
#define MAX_LONGCLICK_TIME 10000
#define LONGCLICK_TIME 2000

static bool ignoreEnc = 0;

static uint8_t encTimerID;

static void RCHA_CB(void);
static void RCHB_CB(void);

static void activate_CHAIRQ_CB(void);


bool initEncoder(void)
{
	gpioMode(RCHA_PIN, INPUT_PULLUP);
	gpioMode(RCHB_PIN, INPUT_PULLUP);

	if (gpioIRQ(RCHA_PIN, GPIO_IRQ_MODE_FALLING_EDGE, &RCHA_CB))
		return 1;
	if (gpioIRQ(RCHB_PIN, GPIO_IRQ_MODE_FALLING_EDGE, &RCHB_CB))
		return 1;

	encTimerID = createTimer_SYS(ENCODER_ROT_TIME, &activate_CHAIRQ_CB, SYS_ONESHOT);
	return 0;
}

static void RCHA_CB(void)
{

	if (ignoreEnc || gpioRead(RCHA_PIN) == !ROT_ACTIVE)
		return;

	if (gpioRead(RCHB_PIN))
	{
		// gpioToggle(PIN_LED_GREEN);	DEBUG
		putEvent(EncoderLeft);
	}

	ignoreEnc = true;
	startTimer_SYS(encTimerID);
}

static void RCHB_CB(void)
{

	if (ignoreEnc || gpioRead(RCHB_PIN) == !ROT_ACTIVE)
		return;

	if (gpioRead(RCHA_PIN))
	{
		// gpioToggle(PIN_LED_BLUE);	DEBUG
		putEvent(EncoderRight);
	}

	ignoreEnc = true;
	startTimer_SYS(encTimerID);
}


//	Interrupcion no periodica
static void activate_CHAIRQ_CB(void)
{
	ignoreEnc = false;
}


