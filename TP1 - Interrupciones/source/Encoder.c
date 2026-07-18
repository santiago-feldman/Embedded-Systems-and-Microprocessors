#include <Encoder.h>
#include <hardware.h>
#include <timer.h>
#include "gpio.h"
#include "config.h"
#include <stdlib.h>
#include "Queue.h"

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


#if SW_PERIODIC_SAMPLING == 0

static uint8_t SWIRQTimerID;
static uint8_t swTimerID;
static uint8_t doubleClickTimerID;
static bool ignoreSWIRQ = 0;
static void activate_SWIRQ_CB(void);
static void doubleclick_CB(void);
static void dummy_CB(void);

#else

static uint8_t swSampPeriodTimerID;

#endif
static void RCHA_CB(void);
static void RCHB_CB(void);
static void RSW_CB(void);
static void activate_CHAIRQ_CB(void);


bool InitEncoder(void)
{
	gpioMode(RCHA_PIN, INPUT);
	gpioMode(RCHB_PIN, INPUT);
	gpioMode(RSW_PIN, INPUT);
	gpioMode(GPIOTPPIN, OUTPUT);
	gpioMode(SYSTICKTPPIN, OUTPUT);

	if (gpioIRQ(RCHA_PIN, GPIO_IRQ_MODE_FALLING_EDGE, &RCHA_CB))
		return 1;
	if (gpioIRQ(RCHB_PIN, GPIO_IRQ_MODE_FALLING_EDGE, &RCHB_CB))
		return 1;
#if SW_PERIODIC_SAMPLING == 0
	if (gpioIRQ(RSW_PIN, GPIO_IRQ_MODE_BOTH_EDGES, &RSW_CB))
		return 1;
	SWIRQTimerID = createTimer(DEBOUNCE_TIME, &activate_SWIRQ_CB, ONESHOT);
	swTimerID = createTimer(MAX_LONGCLICK_TIME, &dummy_CB, ONESHOT);
	doubleClickTimerID = createTimer(DOUBLECLIK_TIME, &RSW_CB, ONESHOT);
#else
	swSampPeriodTimerID = createTimer(SW_SAMPLING_TIME, &RSW_CB, PERIODIC);
	startTimer(swSampPeriodTimerID);
#endif
	encTimerID = createTimer(ENCODER_ROT_TIME, &activate_CHAIRQ_CB, ONESHOT);
	return 0;
}

static void RCHA_CB(void)
{

	if (ignoreEnc || gpioRead(RCHA_PIN) == !ROT_ACTIVE)
		return;

	if (gpioRead(RCHB_PIN))
	{
		putEvent(EncoderLeft);
	}

	ignoreEnc = true;
	startTimer(encTimerID);
}

static void RCHB_CB(void)
{

	if (ignoreEnc || gpioRead(RCHB_PIN) == !ROT_ACTIVE)
		return;

	if (gpioRead(RCHA_PIN))
	{
		putEvent(EncoderRight);
	}

	ignoreEnc = true;
	startTimer(encTimerID);
}

#if SW_PERIODIC_SAMPLING == 0
static void RSW_CB(void)
{
	static bool prevState = 1;

	if (ignoreSWIRQ)
	{
		return;
	}

	bool pinState = gpioRead(RSW_PIN);
	if ((prevState != pinState) && pinState == LOW)
	{
		prevState = pinState;
		if (getTimerState(swTimerID) == IDLE && getTimerState(doubleClickTimerID) == IDLE)
			startTimer(swTimerID);
	}
	else if ((prevState != pinState) && (pinState == HIGH) && (getTime(swTimerID) > LONGCLICK_TIME))
	{
		putEvent(EncoderLongPress);
		stopTimer(swTimerID);
		prevState = pinState;
		ignoreSWIRQ = true;
		startTimer(SWIRQTimerID);
	}
	else if ((prevState != pinState) && (pinState == HIGH) && (getTime(swTimerID) > DEBOUNCE_TIME))
	{
		if (getTimerState(doubleClickTimerID) == RUNNING)
		{
			stopTimer(doubleClickTimerID);
			putEvent(EncoderDoubleClick);
			stopTimer(swTimerID);
			ignoreSWIRQ = true;
			startTimer(SWIRQTimerID);
		}
		else
		{

			startTimer(doubleClickTimerID);
		}
		prevState = pinState;
	}
}

static void activate_SWIRQ_CB(void)
{
	ignoreSWIRQ = false;
}

static void doubleclick_CB(void)
{
	putEvent(EncoderClick);
	stopTimer(swTimerID);
	ignoreSWIRQ = true;
	startTimer(SWIRQTimerID);
}

static void dummy_CB(void)
{
}
#else
static void RSW_CB(void)
{
	static bool prevState = !SW_ACTIVE;
	static uint32_t pressTimeCounter = 0;
	static uint32_t clickCounter = 0;
	bool SWState = gpioRead(RSW_PIN);

	if (prevState != SWState && SWState == SW_ACTIVE)
	{
		prevState = SWState;
	}
	else if (prevState != SWState && SWState == !SW_ACTIVE && pressTimeCounter >= (LONGCLICK_TIME / SW_SAMPLING_TIME))
	{
		putEvent(EncoderLongPress);
		pressTimeCounter = 0;
		prevState = !SW_ACTIVE;
	}
	else if (prevState != SWState && SWState == !SW_ACTIVE)
	{
		clickCounter++;
		pressTimeCounter++;
		prevState = !SW_ACTIVE;
	}
	else if ((SWState == SW_ACTIVE) || clickCounter)
	{
		pressTimeCounter++;
	}

	if (clickCounter && pressTimeCounter >= (DOUBLECLIK_TIME / SW_SAMPLING_TIME))
	{
		if (clickCounter == 1)
		{
			putEvent(EncoderClick);
			pressTimeCounter = 0;
			clickCounter = 0;
		}
		else
		{
			putEvent(EncoderDoubleClick);
			pressTimeCounter = 0;
			clickCounter = 0;
		}
	}
}
#endif
//	Interrupcion no periodica

static void activate_CHAIRQ_CB(void)
{
	ignoreEnc = false;
}


