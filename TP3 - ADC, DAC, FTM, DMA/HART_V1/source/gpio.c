#include <stdint.h>
#include <stdbool.h>
#include "MK64F12.h"
#include "gpio.h"
#include "config.h"

#define GPIO_MODE 1

enum pinnum2powerof2_t
{
	PIN0 = 1UL,
	PIN1 = 2UL,
	PIN2 = 4UL,
	PIN3 = 8UL,
	PIN4 = 16UL,
	PIN5 = 32UL,
	PIN6 = 64UL,
	PIN7 = 128UL,
	PIN8 = 256UL,
	PIN9 = 512UL,
	PIN10 = 1024UL,
	PIN11 = 2048UL,
	PIN12 = 4096UL,
};

static pinIrqFun_t PORTA_CBs[32];
static pinIrqFun_t PORTB_CBs[32];
static pinIrqFun_t PORTC_CBs[32];
static pinIrqFun_t PORTD_CBs[32];
static pinIrqFun_t PORTE_CBs[32];

void gpioMode(pin_t pin, uint8_t mode)
{

	/*	Asi seria con bit-banding
	 *((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 10)) = 0;	//Gpio Mode on MUX
	 *((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 9)) = 0;	//
	 *((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 8)) = 1;	//
	 */

	// Lo hacemos con mascaras
	bool filt_enable = mode & FILTER_ENABLE;
	mode &= 3UL;

	PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] = (PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(GPIO_MODE);

	if (mode == OUTPUT)
	{
		*((uint32_t *)BITBAND_REGADDR(PORT2GPIOADDR(PIN2PORT(pin))->PDDR, PIN2NUM(pin))) = 1;
	}
	else
	{
		*((uint32_t *)BITBAND_REGADDR((PORT2GPIOADDR(PIN2PORT(pin))->PDDR), PIN2NUM(pin))) = 0;
		if (mode == INPUT_PULLUP)
		{
			*((uint32_t *)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]), 0)) = 1; // 0 es el bit del Pull Select
			*((uint32_t *)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]), 1)) = 1; // 1 es el bit del Pull Enable
		}
		else if (mode == INPUT_PULLDOWN)
		{
			*((uint32_t *)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]), 0)) = 0;
			*((uint32_t *)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]), 1)) = 1;
		}
	}

	if (filt_enable)
		*((uint32_t *)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]), 4)) = 1;
}

bool gpioRead(pin_t pin)
{

	return (BITBAND_REG32((PORT2GPIOADDR(PIN2PORT(pin))->PDIR), PIN2NUM(pin)));
}

void gpioToggle(pin_t pin)
{

	*((uint32_t *)BITBAND_REGADDR((PORT2GPIOADDR(PIN2PORT(pin))->PTOR), PIN2NUM(pin))) = 1;
}

void gpioWrite(pin_t pin, bool value)
{

	*((uint32_t *)BITBAND_REGADDR((PORT2GPIOADDR(PIN2PORT(pin))->PDOR), PIN2NUM(pin))) = value;
}

bool gpioIRQ(pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun)
{
	if (irqFun == 0 && irqMode != GPIO_IRQ_MODE_DISABLE)
	{
		return 1;
	}
	PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] = (PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] & ~PORT_PCR_IRQC_MASK) | PORT_PCR_IRQC(irqMode);
	switch
		PIN2PORT(pin)
		{
		case PA:
			PORTA_CBs[PIN2NUM(pin)] = irqFun;
			break;
		case PB:
			PORTB_CBs[PIN2NUM(pin)] = irqFun;
			break;
		case PC:
			PORTC_CBs[PIN2NUM(pin)] = irqFun;
			break;
		case PD:
			PORTD_CBs[PIN2NUM(pin)] = irqFun;
			break;
		case PE:
			PORTE_CBs[PIN2NUM(pin)] = irqFun;
			break;
		default:
			break;
		}
	if (irqMode != GPIO_IRQ_MODE_DISABLE)
	{
		NVIC_EnableIRQ(PORTA_IRQn + PIN2PORT(pin));
	}
	return 0;
}

void PORTA_IRQHandler(void)
{
	uint32_t IRQreg = PORT2ADDR(PA)->ISFR;
	PORT2ADDR(PA)->ISFR = 0xFFFFFFFFU;
	for (int i = 0; i < 32; i++)
	{
		if (IRQreg & (1 << i))
		{
			PORTA_CBs[i]();
		}
	}
}

void PORTB_IRQHandler(void)
{
	uint32_t IRQreg = PORT2ADDR(PB)->ISFR;
	PORT2ADDR(PB)->ISFR = 0xFFFFFFFFU;

	for (int i = 0; i < 32; i++)
	{
		if (IRQreg & (1 << i))
		{
			PORTB_CBs[i]();
		}
	}
}
void PORTC_IRQHandler(void)
{
	uint32_t IRQreg = PORT2ADDR(PC)->ISFR;
	PORT2ADDR(PC)->ISFR = 0xFFFFFFFFU;

	for (int i = 0; i < 32; i++)
	{
		if (IRQreg & (1 << i))
		{
			PORTC_CBs[i]();
		}
	}
}
void PORTD_IRQHandler(void)
{
	uint32_t IRQreg = PORT2ADDR(PD)->ISFR;
	PORT2ADDR(PD)->ISFR = 0xFFFFFFFFU;

	for (int i = 0; i < 32; i++)
	{
		if (IRQreg & (1 << i))
		{
			PORTD_CBs[i]();
		}
	}
}
void PORTE_IRQHandler(void)
{
	uint32_t IRQreg = PORT2ADDR(PE)->ISFR;
	PORT2ADDR(PE)->ISFR = 0xFFFFFFFFU;
	for (int i = 0; i < 32; i++)
	{
		if (IRQreg & (1 << i))
		{
			PORTE_CBs[i]();
		}
	}
}
