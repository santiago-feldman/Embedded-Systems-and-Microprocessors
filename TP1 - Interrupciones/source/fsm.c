/***************************************************************************//**
  @file     +Nombre del archivo (ej: template.c)+
  @brief    +Descripcion del archivo+
  @author   +Nombre del autor (ej: Salvador Allende)+
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "fsm.h"
#include "Encoder.h"
#include "display.h"
#include "cardReader.h"
#include "Queue.h"
#include "timer.h"
#include "led.h"
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ERRORMSG_TIME 1000
#define OPEN_TIME 5000
#define ADMIN_PSSW 1234
#define MAX_TRIES 3

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void userReset (user_t * activeUser);
int getIDNumber (int ID [8]);
void getIDString (int ID [8], char * wordItoA);
int getPINNumber (int PIN [5], int PINDigits);
void getIDArray(uint32_t num , int* ID);
void cardReaderErrorCallback(void);
void IDErrorCallback(void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static fsmState_t state = INSERTID;
static int localCursor = 0;
static int brightness = 50;
static int preventRewrite = 0;
static int activeUserIndex = 0;
static	user_t ActiveUser = {0};
static char wordItoA [VDISP_SIZE] = {0};
static int timerCardErrorId = 0;
static int timerErrorId = 0;

//Available users are listed here

static user_t USUARIOS [MAXUSUARIOS] = {{{1,2,3,4,5,6,7,8},{1,2,3,4,5},5,0} ,
										{{1,2,3,4,5,6,7,8},{2,3,4,5,9},4,0} ,
										{{4,0,4,0,3,1,0,0},{0,0,0,0,1},4,0},
										{{4,1,1,1,9,7,2,0},{0,0,0,0,5},5,0},
										{{0,0,0,0,0,0,0,1},{0,0,0,1,0},4,0}};


static uint32_t adminPassword[] = {0,0,0,0};



/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


void FSM (){
	user_t* activeUser = &ActiveUser;
	switch(state){

		case BRIGHTNESS_MENU:

			if (!preventRewrite){
				clear_display();
				set_cursor(0);
				char string [] = "BRIGHTNESS";
				write_word(string);
				enable_sliding(true);
				preventRewrite = 1;
			}

			switch(getNextEvent()){
			case EncoderLeft:

				state = ADMIN_MENU;
				preventRewrite = 0;
			break;
			case EncoderRight:

				state = ACCESS_MENU;
				preventRewrite = 0;
			break;
			case EncoderClick:

				state = BRIGHTNESS_ADJUST;
				preventRewrite = 0;
			break;
			default:
			break;
			}
		break;

		case BRIGHTNESS_ADJUST:

			if (!preventRewrite){
				clear_display();
				set_cursor(0);
				char string [] = "ABCDEFGHIJ";
				write_word(string);
				enable_sliding(true);
				preventRewrite = 1;
			}

			switch(getNextEvent()){
			case EncoderLeft:

				if (brightness > 5){
					brightness -= 10;
					set_brightness(brightness);
				}
				set_brightness(brightness);
			break;
			case EncoderRight:

				if (brightness < 95){
					brightness += 10;
					set_brightness(brightness);
				}
			break;
			case EncoderClick:

				state = BRIGHTNESS_MENU;
				preventRewrite = 0;
			break;
			default:
			break;
			}

		break;
		case ADMIN_MENU:

			if (!preventRewrite){
				clear_display();
				set_cursor(0);
				char string [] = "  ADMIN   ";
				write_word(string);
				enable_sliding(true);
				preventRewrite = 1;
			}

			switch(getNextEvent()){
			case EncoderLeft:

				state = ACCESS_MENU;
				preventRewrite = 0;
			break;
			case EncoderRight:

				state = BRIGHTNESS_MENU;
				preventRewrite = 0;
			break;
			case EncoderClick:

				state = ADMIN_MENU;		//No hace nada de momento
				//state = CHECK_ADMIN_PASSWORD
				preventRewrite = 0;
			break;
			default:
			break;
			}

		break;

		case CHECK_ADMIN_PASSWORD:

			if (!preventRewrite){
				clear_display();
				set_cursor(0);
				set_window_offset(0);
				char string [] = "0XXXX";
				write_word(string);
				set_cursor(0);
				localCursor = 0;
				enable_sliding(false);
				preventRewrite = 1;
			}

			switch(getNextEvent()){
				case EncoderLeft:

					if (adminPassword[localCursor] > 0){
						adminPassword[localCursor] -= 1;
						set_cursor(localCursor);
						write_char(adminPassword[localCursor] + 0x30);
					}

				break;
				case EncoderRight:

					if (adminPassword[localCursor] >= 0 && adminPassword[localCursor] < 9){
						adminPassword[localCursor] += 1;
						set_cursor(localCursor);
						write_char(adminPassword[localCursor] + 0x30);
					}
				break;
				case EncoderClick:

					set_cursor(localCursor);
					write_char('X');
					localCursor++;
					set_cursor(localCursor);
					write_char(adminPassword[localCursor] + 0x30);
					if (localCursor >= 4){
						if (getPINNumber(adminPassword, 4) == ADMIN_PSSW)
							state = ADMIN;
						else
						{
							userReset(activeUser);
							preventRewrite = 0;
							state = ACCESS_MENU;
						}
					}

				break;
				case EncoderDoubleClick:

					if (localCursor > 0){
						write_char('X');
						localCursor--;
						set_cursor(localCursor);
						write_char(adminPassword[localCursor] + 0x30);
					}
					if (localCursor == 3){
						set_window_offset(0);
					}
				break;
				case EncoderLongPress:

					userReset(activeUser);
					preventRewrite = 0;
					state = ACCESS_MENU;
				break;
			}
		break;

		case ADMIN:

			break;

		case ACCESS_MENU:

			if (!preventRewrite){
				clear_display();
				set_cursor(0);
				char string [] = "  ACCESS  ";
				write_word(string);
				enable_sliding(true);
				preventRewrite = 1;
			}

			switch(getNextEvent()){
			case EncoderLeft:

				state = BRIGHTNESS_MENU;
				preventRewrite = 0;
			break;
			case EncoderRight:

				state = ADMIN_MENU;
				preventRewrite = 0;
			break;
			case EncoderClick:

				state = INSERTID;
				localCursor = 0;
				preventRewrite = 0;
			break;
			default:
			break;
			}

		break;

		case INSERTID:

			if (!preventRewrite){
				clear_display();
				set_cursor(0);
				set_window_offset(0);
				getIDString(activeUser->ID, wordItoA);
				write_word(wordItoA);
				enable_sliding(false);
				preventRewrite = 1;
			}

			switch(getNextEvent()){
			case EncoderLeft:

				if (activeUser->ID[localCursor] > 0){
					activeUser->ID[localCursor] -= 1;
					set_cursor(localCursor);
					write_char(activeUser->ID[localCursor] + 0x30);
				}

			break;
			case EncoderRight:

				if (activeUser->ID[localCursor] >= 0 && activeUser->ID[localCursor] < 9){
					activeUser->ID[localCursor] += 1;
					set_cursor(localCursor);
					write_char(activeUser->ID[localCursor] + 0x30);
				}
			break;
			case EncoderClick:

				localCursor++;
				set_cursor(localCursor);
				if (localCursor == 4){
					set_window_offset(4);
				}
				if (localCursor >= 8){
					state = CHECKID;
					preventRewrite = 0;
				}
			break;
			case EncoderDoubleClick:

				if (localCursor > 0){
					localCursor--;
					set_cursor(localCursor);
				}
				if (localCursor == 3){
					set_window_offset(0);
				}
			break;
			case EncoderLongPress:

				userReset(activeUser);
				preventRewrite = 0;
				state = ACCESS_MENU;
			break;
			case CardReadError:
				clear_display();
				set_cursor(0);
				set_window_offset(0);
				char string [] = "Err ";
				write_word(string);
				enable_sliding(false);
				timerCardErrorId = createTimer(ERRORMSG_TIME, &cardReaderErrorCallback ,ONESHOT);
				if (timerCardErrorId != MAXTIMERS){
					startTimer(timerCardErrorId);
				}

			break;
			case CardRead: {

				uint32_t idTarjeta1 = getID();
				getIDArray(idTarjeta1 , activeUser->ID);
				state = CHECKID;
				preventRewrite = 0;
			}
 			break;
			default:
			break;
			}

		break;

		case CHECKID: {	//Dejar estos corchetes, sino no compila

			bool tempUnlocked = false;
			uint32_t currID = getIDNumber(activeUser->ID);
			uint32_t id2check = 0;
			for (int i=0;i<MAXUSUARIOS;i++){
				id2check = getIDNumber((USUARIOS[i]).ID);
				if (currID == id2check && id2check != 0){
					if (USUARIOS[i].failedTries >= MAX_TRIES)
					{
						activeUserIndex = -1;
						break;
					}
					//Access Allowed

					activeUserIndex = i;
					tempUnlocked = true;
					localCursor = 0;
					preventRewrite = 0;
					state = INSERTPIN;

					break;
				}
			}

			if ((!tempUnlocked) && (preventRewrite == 0)) {
				localCursor = 0;
				clear_display();
				set_cursor(0);
				set_window_offset(0);
				char lockString [] = "lock";
				char noidString [] = "noid";
				if (activeUserIndex == -1)
				{
					write_word(lockString);
					activeUserIndex = 0;
				}
				else
					write_word(noidString);
				enable_sliding(false);
				timerErrorId = createTimer(ERRORMSG_TIME, &IDErrorCallback ,ONESHOT);
				if (timerErrorId != MAXTIMERS){
					startTimer(timerErrorId);
				}
				preventRewrite = 1;
				state = INSERTID;


			}

		}
		break;

		case INSERTPIN:

			if (!preventRewrite){
				clear_display();
				set_cursor(0);
				set_window_offset(0);
				char string [] = "0XXXX";
				write_word(string);
				set_cursor(0);
				localCursor = 0;
				enable_sliding(false);
				preventRewrite = 1;
			}

			switch(getNextEvent()){
			case EncoderLeft:

				if (activeUser->PIN[localCursor] > 0){
					activeUser->PIN[localCursor] -= 1;
					set_cursor(localCursor);
					write_char(activeUser->PIN[localCursor] + 0x30);
				}

			break;
			case EncoderRight:

				if (activeUser->PIN[localCursor] >= 0 && activeUser->PIN[localCursor] < 9){
					activeUser->PIN[localCursor] += 1;
					set_cursor(localCursor);
					write_char(activeUser->PIN[localCursor] + 0x30);
				}
			break;
			case EncoderClick:

				set_cursor(localCursor);
				write_char('X');
				localCursor++;
				set_cursor(localCursor);
				write_char(activeUser->PIN[localCursor] + 0x30);
				if (localCursor == 4){
					set_window_offset(4);
				}
				if (localCursor >= (USUARIOS[activeUserIndex]).pinDigits){
					state = CHECKPIN;
				}

			break;
			case EncoderDoubleClick:

				if (localCursor > 0){
					write_char('X');
					localCursor--;
					set_cursor(localCursor);
					write_char(activeUser->PIN[localCursor] + 0x30);
				}
				if (localCursor == 3){
					set_window_offset(0);
				}
			break;
			case EncoderLongPress:

				userReset(activeUser);
				preventRewrite = 0;
				state = ACCESS_MENU;
			break;
			default:
			break;
			}

		break;

		case CHECKPIN:

			if (getPINNumber(USUARIOS[activeUserIndex].PIN , USUARIOS[activeUserIndex].pinDigits)
					== getPINNumber( activeUser->PIN , USUARIOS[activeUserIndex].pinDigits)){
				//Access Allowed - LED ROJO ENCENDIDO

				set_timed_led(1, OPEN_TIME);
				USUARIOS[activeUserIndex].failedTries = 0;
				localCursor = 0;
				preventRewrite = 0;
				userReset (activeUser);
				state = INSERTID;
			}
			else{
				//Access Denied - LED ROJO ENCENDIDO
				USUARIOS[activeUserIndex].failedTries++;
				localCursor = 0;
				preventRewrite = 0;
				userReset (activeUser);
				state = INSERTID;
			}

		break;
	}
}


void userReset (user_t * activeUser){

	for (int i=0; i<8; i++){
	activeUser->ID[i] = 0;
	}
	for (int i=0; i<5; i++){
		activeUser->PIN[i] = 0;
		}
	activeUser->failedTries = 0;
}

int getIDNumber (int ID [8]){

	return ID[7]+ID[6]*10+ID[5]*100+ID[4]*1000+ID[3]*10000+ID[2]*100000+ID[1]*1000000+ID[0]*10000000;
}

void getIDString (int ID [8], char * wordItoA){

	for (int i=0; i<8; i++){

		wordItoA [i] = ID[i] + 0x30;
	}
	for (int i=8; i<VDISP_SIZE; i++){
		wordItoA [i] = ' ';
	}
}


void getIDArray(uint32_t num , int* ID){

	int i=7;

	while (num > 0){

		ID[i] = num%10;
		num = num/10;
		i--;
	}
}

int getPINNumber (int PIN [5], int PINDigits){

	if (PINDigits == 5){
		return PIN[4]+PIN[3]*10+PIN[2]*100+PIN[1]*1000+PIN[0]*10000;
	}
	else if (PINDigits == 4){
		return PIN[3]+PIN[2]*10+PIN[1]*100+PIN[0]*1000;
	}
	else{
		return 0;
	}

}

void cardReaderErrorCallback(void){

	preventRewrite = 0;
	destroyTimer(timerCardErrorId);
}

void IDErrorCallback(void){
	state = INSERTID;		//Access Denied
	preventRewrite = 0;
	localCursor = 0;
	destroyTimer(timerErrorId);
	userReset(&ActiveUser);
}

