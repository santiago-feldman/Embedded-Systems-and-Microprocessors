
#include "timer.h"
#include "MK64F12.h"
#include "hardware.h"
#include "gpio.h"
#include "config.h"

#define REFCLK (__CORE_CLOCK__)
#define FREQ_REFERENCE 1000U	// 1ms period.

typedef struct{
	uint8_t state;
	uint32_t currCont;
	uint32_t topCont;
	uint8_t mode;
	void (*callback)(void);
}TIMER_t;

static TIMER_t timer[MAXTIMERS_SYS];

bool initTimer_SYS(){

	bool success = false;

	if (SysTick_Config(REFCLK/FREQ_REFERENCE) == 0){ //ms unidad de tiempo minima
		NVIC_EnableIRQ(SysTick_IRQn);
		success = true;
	}

	return success;
}

uint8_t createTimer_SYS(uint32_t time, void (*funcallback)(void), timerModesSYS_t mode){

	uint8_t i = 0;
	bool found = false;

	while(i < MAXTIMERS_SYS && !found){
		if(timer[i].state == SYS_FREE){
			timer[i].mode = mode;
			timer[i].state = SYS_IDLE;
			timer[i].topCont = time;
			timer[i].callback = funcallback;
			resetTimer_SYS(i);
			found = true;
		}
		else{
			i++;
		}
	}

	return i;
}

void configTimerCallback_SYS(uint8_t id, void (*funcallback)(void)){
	timer[id].callback = funcallback;
}

void configTimerTime_SYS(uint8_t id, uint32_t time){
	timer[id].topCont = time;
}

void configTimerMode_SYS(uint8_t id, uint8_t mode){
	timer[id].mode = mode;
}

void destroyTimer_SYS(uint8_t id){
	timer[id].currCont = 0;
	timer[id].state = SYS_FREE;
}

void resetTimer_SYS(uint8_t id){
	timer[id].currCont = 0;
}

void startTimer_SYS(uint8_t id){
	timer[id].currCont = 0;
	if (timer[id].state != SYS_FREE)
		timer[id].state = SYS_RUNNING;
}

void stopTimer_SYS(uint8_t id){
	if (timer[id].state != SYS_FREE)
		timer[id].state = SYS_IDLE;
	timer[id].currCont = 0;
}

uint32_t getTime_SYS(uint8_t id){
	return timer[id].currCont;
}

uint8_t getTimerState_SYS(uint8_t id){
	return timer[id].state;
}

void SysTick_Handler(void){

	for(int i = 0; i < MAXTIMERS_SYS; i++){
		if(timer[i].state == SYS_RUNNING){
			timer[i].currCont++;
			if(timer[i].currCont >= timer[i].topCont){
				timer[i].callback();
				resetTimer_SYS(i);
				if(timer[i].mode == SYS_ONESHOT){
					stopTimer_SYS(i);
				}
			}
		}
	}

	NVIC_ClearPendingIRQ(SysTick_IRQn);
}



