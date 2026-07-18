
#include "LEDMatrix.h"
#include "PWM.h"
#include "PIT.h"
#include "FTM.h"
#include "MK64F12.h"
#include "config.h"

#define TICKSPERPERIOD (63) // 50MHz * 1.25us
#define T0H (20)
#define T1H (40)
#define TREFRESH (2772) // 44ticks * 1.25us = 55us

#define RGBBITS (24)
#define NUMOFLEDS (ROWS * COLS)

#define REFRESHRATE (41666.0f)		  // 24Hz
#define DEFAULTBLINKTIME (1000000.0f) // 1s

#define SCALECOLOR(color, b) ((int)((color) * (b) / 100))
#define ON (1)
#define OFF (0)

typedef enum
{
	R,
	B,
	G,
} states;

typedef struct
{
	color_t color;
	uint8_t onoff : 1;
	uint8_t blink : 1;
} LED_t;

static LED_t LEDMatrix[NUMOFLEDS] = {0};
static uint16_t PWMLEDMatrix[NUMOFLEDS * RGBBITS + 2] = {0};
static uint8_t brightness = MAXBRIGHTNESS / 6;
static uint8_t refreshTimerID = 0;
static uint8_t blinkTimerID = 0;

static void update();
static void refresh();
static void onRefreshEnded();
static void toggle();

void initLEDMatrix()
{
	color_t defaultColor;
	defaultColor.hex = BLACK;
	defaultColor.bright = brightness;

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			changeColor(i, j, defaultColor);
		}
	}

	update();
	initPIT();
	PWM_Init();
	PWM_SetTickPerPeriod(TICKSPERPERIOD);
	PWM_GenWaveform(PWMLEDMatrix, NUMOFLEDS * RGBBITS + 2, 1, onRefreshEnded);

	refreshTimerID = createTimer(REFRESHRATE, refresh);
	blinkTimerID = createTimer(DEFAULTBLINKTIME, toggle);
	startTimer(refreshTimerID);
	startTimer(blinkTimerID);
}

void turnOn(uint8_t row, uint8_t col)
{
	LEDMatrix[row * 8 + col].onoff = ON;
	update();
}

void turnOff(uint8_t row, uint8_t col)
{
	LEDMatrix[row * 8 + col].onoff = OFF;
	update();
}

void turnOnAll()
{
	for (int i = 0; i < NUMOFLEDS; i++)
	{
		LEDMatrix[i].onoff = ON;
	}
	update();
}

void changeBrightness(uint8_t percentage)
{
	if (percentage > MAXBRIGHTNESS)
	{
		percentage = MAXBRIGHTNESS;
	}
	else if (percentage <= MINBRIGHTNESS)
	{
		percentage = MINBRIGHTNESS;
	}

	brightness = percentage;
}

void changeColor(uint8_t row, uint8_t col, color_t color)
{
	LEDMatrix[row * 8 + col].color = color;
	update();
}

void blink(uint8_t row, uint8_t col, float ms)
{
	LEDMatrix[row * 8 + col].blink = ON;
	LEDMatrix[row * 8 + col].onoff = ON;
	configTimerTime(blinkTimerID, ms);
}

void stopBlink(uint8_t row, uint8_t col)
{
	LEDMatrix[row * 8 + col].blink = OFF;
	LEDMatrix[row * 8 + col].onoff = OFF;
}

/*******************************************************************************
 *******************************************************************************
						LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void toggle()
{
	for (int i = 0; i < NUMOFLEDS; i++)
	{
		if (LEDMatrix[i].blink == ON)
		{
			if (LEDMatrix[i].onoff == ON)
			{
				LEDMatrix[i].onoff = OFF;
			}
			else
			{
				LEDMatrix[i].onoff = ON;
			}
		}
	}
	update();
}

static void refresh()
{
	FTM_StartClock(FTM0);
}

static void onRefreshEnded()
{
	FTM_StopClock(FTM0);
}

static void update()
{
	int currBit = 7;
	states currColor = G;
	uint8_t bit = 0;
	uint8_t tempColor;

	for (int i = 0; i < NUMOFLEDS; i++)
	{
		for (int j = 0; j < RGBBITS; j++)
		{
			if (LEDMatrix[i].onoff == ON)
			{
				switch (currColor)
				{
				case G:
					tempColor = SCALECOLOR(LEDMatrix[i].color.g, brightness);
					bit = (tempColor >> currBit) & (0x1);
					if (currBit <= 0)
					{
						currColor = R;
						currBit = 7;
					}
					else
					{
						currBit--;
					}
					break;

				case R:
					tempColor = SCALECOLOR(LEDMatrix[i].color.r, brightness);
					bit = (tempColor >> currBit) & (0x1);
					if (currBit <= 0)
					{
						currColor = B;
						currBit = 7;
					}
					else
					{
						currBit--;
					}
					break;

				case B:
					tempColor = SCALECOLOR(LEDMatrix[i].color.b, brightness);
					bit = (tempColor >> currBit) & (0x1);
					if (currBit <= 0)
					{
						currColor = G;
						currBit = 7;
					}
					else
					{
						currBit--;
					}
					break;
				}
			}
			else
			{
				bit = 0;
			}

			if (bit == 0)
			{
				PWMLEDMatrix[i * RGBBITS + j] = T0H;
			}
			else
			{
				PWMLEDMatrix[i * RGBBITS + j] = T1H;
			}
		}
	}

	PWMLEDMatrix[NUMOFLEDS * RGBBITS] = T0H;
	PWMLEDMatrix[NUMOFLEDS * RGBBITS + 1] = T0H;
}
