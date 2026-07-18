
#ifndef _FSKDEM_H_
#define _FSKDEM_H_

#include <stdint.h>
#include <stdbool.h>

void initFSKDemodulator();

/**
 * @brief flag that returns if processing of the current word has finished
 * @param void
 * @return true if finished
 */
bool isDataReadyHART();

char getNextValueHART(void);

#endif /* void _FSKDEM_H_ */
