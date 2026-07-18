/***************************************************************************//**
  @file     +Nombre del archivo (ej: template.c)+
  @brief    +Descripcion del archivo+
  @author   +Nombre del autor (ej: Salvador Allende)+
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "button.h"
#include "config.h"
#include "gpio.h"
#include "board.h"
#include "timer.h"
#include "eventQueue.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/



/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/



/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void initbutton1(void);

void initbutton2(void);

void initbutton3(void);

void initButtonEncoder(void);

void button1_ISR(void);

void button2_ISR(void);

void button3_ISR(void);

void buttonEncoder_ISR(void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

uint8_t buttonTimer = 0;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void initButtons(void){

	initbutton1();
	initbutton2();
	initbutton3();
	initButtonEncoder();
	buttonTimer = createTimer_SYS(100, buttonManagement_ISR, SYS_PERIODIC);
	startTimer_SYS(buttonTimer);

}

void buttonManagement_ISR(void){

	button1_ISR();
	button2_ISR();
	button3_ISR();
	buttonEncoder_ISR();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void initbutton1(void){

	gpioMode(BUTTON1_PIN, INPUT);
}

void initbutton2(void){

	gpioMode(BUTTON2_PIN, INPUT);
}

void initbutton3(void){

	gpioMode(BUTTON3_PIN, INPUT);
}

void initButtonEncoder(void){

	gpioMode(BUTTON_ENCODER_PIN, INPUT);
}

void button1_ISR(void){

	static bool lastButtonState = HIGH;
	bool state = gpioRead(BUTTON1_PIN);

	if(lastButtonState == HIGH && state == LOW){

		lastButtonState = state;

		putEvent(ButtonLine);

		//DEBUG
		//gpioToggle(PIN_LED_BLUE);
	}
	else if (lastButtonState == LOW && state == HIGH){

		lastButtonState = state;

		//buttonReleased event

		//DEBUG
		//gpioToggle(PIN_LED_BLUE);
	}
}

void button2_ISR(void){

	static bool lastButtonState = HIGH;
	bool state = gpioRead(BUTTON2_PIN);

	if(lastButtonState == HIGH && state == LOW){

		lastButtonState = state;

		putEvent(ButtonPoint);

		//DEBUG
		//gpioToggle(PIN_LED_RED);
	}
	else if (lastButtonState == LOW && state == HIGH){

		lastButtonState = state;

		//buttonReleased event

		//DEBUG
		//gpioToggle(PIN_LED_RED);
	}
}

void button3_ISR(void){

	static bool lastButtonState = HIGH;
	bool state = gpioRead(BUTTON3_PIN);

	if(lastButtonState == HIGH && state == LOW){

		lastButtonState = state;

		putEvent(ButtonCross);

		//DEBUG
		//gpioToggle(PIN_LED_GREEN);
	}
	else if (lastButtonState == LOW && state == HIGH){

		lastButtonState = state;

		//buttonReleased event

		//DEBUG
		//gpioToggle(PIN_LED_GREEN);
	}
}

void buttonEncoder_ISR(void){

	static bool lastButtonState = HIGH;
		bool state = gpioRead(BUTTON_ENCODER_PIN);

		if(lastButtonState == HIGH && state == LOW){

			lastButtonState = state;

			putEvent(EncoderClick);

			//DEBUG
			//gpioToggle(PIN_LED_RED);
		}
		else if (lastButtonState == LOW && state == HIGH){

			lastButtonState = state;

			//buttonReleased event

			//DEBUG
			//gpioToggle(PIN_LED_BLUE);
		}
}

