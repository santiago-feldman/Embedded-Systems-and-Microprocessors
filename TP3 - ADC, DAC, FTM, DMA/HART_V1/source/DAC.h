
#ifndef _DAC_H_
#define _DAC_H_

#include "hardware.h"
#include "MK64F12.h"

typedef DAC_Type *DAC_t;
typedef uint16_t DACData_t;

void DAC_Init(void);

void DAC_SetData(DAC_t, DACData_t);

#endif /* void _DAC_H_ */
