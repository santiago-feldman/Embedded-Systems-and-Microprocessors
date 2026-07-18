
#ifndef _FSKDEM_H_
#define _FSKDEM_H_

#include "hardware.h"
#include "ADC.h"

void initFSKDemodulator();

/**
 * @brief Funtion callback to compute the algorithm which demodule the signal received from ADC using ADC interruptions
 * @param void
 * @return void
 */
void FSKDemCallback(void);

/**
 * @brief flag that returns if processing of the current word has finished
 * @param void
 * @return true if finished
 */
bool isDataReadyHART();

char getNextValueHART(void);

/**
 * @brief function that return the next word that arrived from ADC
 * @param void
 * @return 8-bit word
 */
bool getWord(uint8_t *wordBuffer);

#endif /* void _FSKDEM_H_ */
