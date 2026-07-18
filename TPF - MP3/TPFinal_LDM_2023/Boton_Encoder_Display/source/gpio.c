#include <stdint.h>
#include <stdbool.h>
#include "MK64F12.h"
#include "gpio.h"
#include "config.h"

#define GPIO_MODE 1

enum pinnum2powerof2_t {
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

void gpioMode (pin_t pin, uint8_t mode){
	
	/*	Asi seria con bit-banding
	*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 10)) = 0;	//Gpio Mode on MUX
	*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 9)) = 0;	//
	*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 8)) = 1;	//
	*/

	//Lo hacemos con mascaras
	bool filt_enable = mode & FILTER_ENABLE;
	mode &= 3UL;

	PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] = (PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] & ~PORT_PCR_MUX_MASK ) | PORT_PCR_MUX(GPIO_MODE);

	if (mode == OUTPUT){
		*((uint32_t*)BITBAND_REGADDR(PORT2GPIOADDR(PIN2PORT(pin))->PDDR , PIN2NUM(pin))) = 1;
	}
	else{
		*((uint32_t*)BITBAND_REGADDR((PORT2GPIOADDR(PIN2PORT(pin))->PDDR) , PIN2NUM(pin))) = 0;
		if(mode == INPUT_PULLUP){
			*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 0)) = 1;	//0 es el bit del Pull Select
			*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 1)) = 1;	//1 es el bit del Pull Enable
		}
		else if(mode == INPUT_PULLDOWN){
			*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 0)) = 0;
			*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 1)) = 1;
		}
	}

	if (filt_enable)
		*((uint32_t*)BITBAND_REGADDR((PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)]) , 4)) = 1;
}

bool gpioRead (pin_t pin){

	return (BITBAND_REG32((PORT2GPIOADDR(PIN2PORT(pin))->PDIR) , PIN2NUM(pin)));
}

void gpioToggle (pin_t pin){

	*((uint32_t*)BITBAND_REGADDR((PORT2GPIOADDR(PIN2PORT(pin))->PTOR) , PIN2NUM(pin))) = 1;
}

void gpioWrite (pin_t pin, bool value){

	*((uint32_t*)BITBAND_REGADDR((PORT2GPIOADDR(PIN2PORT(pin))->PDOR) , PIN2NUM(pin))) = value;
}

bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun){
	if (irqFun == 0 && irqMode != GPIO_IRQ_MODE_DISABLE){
		return 1;
	}
	PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] = (PORT2ADDR(PIN2PORT(pin))->PCR[PIN2NUM(pin)] & ~PORT_PCR_IRQC_MASK ) | PORT_PCR_IRQC(irqMode);
	switch PIN2PORT(pin){
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
	if (irqMode != GPIO_IRQ_MODE_DISABLE){
		NVIC_EnableIRQ(PORTA_IRQn + PIN2PORT(pin));
	}
	return 0;
}


void PORTA_IRQHandler(void){
	uint32_t IRQreg = PORT2ADDR(PA)->ISFR;
	PORT2ADDR(PA)->ISFR = 0xFFFFFFFFU;

	switch (IRQreg){
	case PIN0:
			PORTA_CBs[0]();
			break;
	case PIN1:
			PORTA_CBs[1]();
			break;
	case PIN2:
			PORTA_CBs[2]();
			break;
	case PIN3:
			PORTA_CBs[3]();
			break;
	case PIN4:
			PORTA_CBs[4]();
			break;
	case PIN5:
			PORTA_CBs[5]();
			break;
	case PIN6:
			PORTA_CBs[6]();
			break;
	case PIN7:
			PORTA_CBs[7]();
			break;
	case PIN8:
			PORTA_CBs[8]();
			break;
	case PIN9:
			PORTA_CBs[9]();
			break;
	case PIN10:
			PORTA_CBs[10]();
			break;
	case PIN11:
			PORTA_CBs[11]();
			break;
	case PIN12:
			PORTA_CBs[12]();
			break;
	default:
			break;
	}

}

void PORTB_IRQHandler(void){
	uint32_t IRQreg = PORT2ADDR(PB)->ISFR;
	PORT2ADDR(PB)->ISFR = 0xFFFFFFFFU;

	switch (IRQreg){
	case PIN0:
		PORTB_CBs[0]();
		break;
	case PIN1:
		PORTB_CBs[1]();
		break;
	case PIN2:
		PORTB_CBs[2]();
		break;
	case PIN3:
		PORTB_CBs[3]();
		break;
	case PIN4:
		PORTB_CBs[4]();
		break;
	case PIN5:
		PORTB_CBs[5]();
		break;
	case PIN6:
		PORTB_CBs[6]();
		break;
	case PIN7:
		PORTB_CBs[7]();
		break;
	case PIN8:
		PORTB_CBs[8]();
		break;
	case PIN9:
		PORTB_CBs[9]();
		break;
	case PIN10:
		PORTB_CBs[10]();
		break;
	case PIN11:
		PORTB_CBs[11]();
		break;
	case PIN12:
		PORTB_CBs[12]();
		break;
	default:
		break;

	}

}
void PORTC_IRQHandler(void){
	uint32_t IRQreg = PORT2ADDR(PC)->ISFR;
	PORT2ADDR(PC)->ISFR = 0xFFFFFFFFU;

	switch (IRQreg){
	case PIN0:
		PORTC_CBs[0]();
		break;
	case PIN1:
		PORTC_CBs[1]();
		break;
	case PIN2:
		PORTC_CBs[2]();
		break;
	case PIN3:
		PORTC_CBs[3]();
		break;
	case PIN4:
		PORTC_CBs[4]();
		break;
	case PIN5:
		PORTC_CBs[5]();
		break;
	case PIN6:
		PORTC_CBs[6]();
		break;
	case PIN7:
		PORTC_CBs[7]();
		break;
	case PIN8:
		PORTC_CBs[8]();
		break;
	case PIN9:
		PORTC_CBs[9]();
		break;
	case PIN10:
		PORTC_CBs[10]();
		break;
	case PIN11:
		PORTC_CBs[11]();
		break;
	case PIN12:
		PORTC_CBs[12]();
		break;
	default:
		break;

	}

}
void PORTD_IRQHandler(void){
	uint32_t IRQreg = PORT2ADDR(PD)->ISFR;
	PORT2ADDR(PD)->ISFR = 0xFFFFFFFFU;

	switch (IRQreg){
	case PIN0:
		PORTD_CBs[0]();
		break;
	case PIN1:
		PORTD_CBs[1]();
		break;
	case PIN2:
		PORTD_CBs[2]();
		break;
	case PIN3:
		PORTD_CBs[3]();
		break;
	case PIN4:
		PORTD_CBs[4]();
		break;
	case PIN5:
		PORTD_CBs[5]();
		break;
	case PIN6:
		PORTD_CBs[6]();
		break;
	case PIN7:
		PORTD_CBs[7]();
		break;
	case PIN8:
		PORTD_CBs[8]();
		break;
	case PIN9:
		PORTD_CBs[9]();
		break;
	case PIN10:
		PORTD_CBs[10]();
		break;
	case PIN11:
		PORTD_CBs[11]();
		break;
	case PIN12:
		PORTD_CBs[12]();
		break;
	default:
		break;

	}
}
void PORTE_IRQHandler(void){
	uint32_t IRQreg = PORT2ADDR(PE)->ISFR;
	PORT2ADDR(PE)->ISFR = 0xFFFFFFFFU;
	switch (IRQreg){
	case PIN0:
		PORTE_CBs[0]();
		break;
	case PIN1:
		PORTE_CBs[1]();
		break;
	case PIN2:
		PORTE_CBs[2]();
		break;
	case PIN3:
		PORTE_CBs[3]();
		break;
	case PIN4:
		PORTE_CBs[4]();
		break;
	case PIN5:
		PORTE_CBs[5]();
		break;
	case PIN6:
		PORTE_CBs[6]();
		break;
	case PIN7:
		PORTE_CBs[7]();
		break;
	case PIN8:
		PORTE_CBs[8]();
		break;
	case PIN9:
		PORTE_CBs[9]();
		break;
	case PIN10:
		PORTE_CBs[10]();
		break;
	case PIN11:
		PORTE_CBs[11]();
		break;
	case PIN12:
		PORTE_CBs[12]();
		break;
	default:
		break;

	}

}
