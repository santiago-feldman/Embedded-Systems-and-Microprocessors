#include <os.h>
#include <stdint.h>


OS_MUTEX MyMutex;
OS_ERR err;

OS_SEM MySem;
OS_SEM FSMsem;

uint8_t floor = 0;
