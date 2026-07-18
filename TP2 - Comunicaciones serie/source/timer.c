
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

static TIMER_t timer[MAXTIMERS];

bool initTimer(){

	bool success = false;

	if (SysTick_Config(REFCLK/FREQ_REFERENCE) == 0){ //ms unidad de tiempo minima
		NVIC_EnableIRQ(SysTick_IRQn);
		success = true;
	}

	return success;
}

uint8_t createTimer(uint32_t time, void (*funcallback)(void), timerModes_t mode){

	uint8_t i = 0;
	bool found = false;

	while(i < MAXTIMERS && !found){
		if(timer[i].state == FREE){
			timer[i].mode = mode;
			timer[i].state = IDLE;
			timer[i].topCont = time;
			timer[i].callback = funcallback;
			resetTimer(i);
			found = true;
		}
		else{
			i++;
		}
	}

	return i;
}

void configTimerCallback(uint8_t id, void (*funcallback)(void)){
	timer[id].callback = funcallback;
}

void configTimerTime(uint8_t id, uint32_t time){
	timer[id].topCont = time;
}

void configTimerMode(uint8_t id, uint8_t mode){
	timer[id].mode = mode;
}

void destroyTimer(uint8_t id){
	timer[id].currCont = 0;
	timer[id].state = FREE;
}

void resetTimer(uint8_t id){
	timer[id].currCont = 0;
}

void startTimer(uint8_t id){
	timer[id].currCont = 0;
	if (timer[id].state != FREE)
		timer[id].state = RUNNING;
}

void stopTimer(uint8_t id){
	if (timer[id].state != FREE)
		timer[id].state = IDLE;
	timer[id].currCont = 0;
}

uint32_t getTime(uint8_t id){
	return timer[id].currCont;
}

uint8_t getTimerState(uint8_t id){
	return timer[id].state;
}

void SysTick_Handler(void){

	for(int i = 0; i < MAXTIMERS; i++){
		if(timer[i].state == RUNNING){
			timer[i].currCont++;
			if(timer[i].currCont >= timer[i].topCont){
				timer[i].callback();
				resetTimer(i);
				if(timer[i].mode == ONESHOT){
					stopTimer(i);
				}
			}
		}
	}

	NVIC_ClearPendingIRQ(SysTick_IRQn);
}



