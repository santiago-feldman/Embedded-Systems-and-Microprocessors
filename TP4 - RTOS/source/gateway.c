/***************************************************************************/ /**
   @file     gateway.c
   @brief    gateway task
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gateway.h"
#include "timer.h"
#include "uart.h"
#include "board.h"
#include "gpio.h"
#include "threads.h"
#include "stdint.h"
#include <os.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TBQ (15000)   // Time between queries, 15 sec
#define KAF (500)     // Keep alive flag
#define TIMEOUT (200) // Keep alive flag timeout
#define SIZEOF(x) (sizeof(x) / sizeof(x[0]))

static bool queryEnabled = true;
static uint8_t timerId;
static uint8_t keepAliveId;
static uint8_t timeoutId;
static uint8_t buffer[6];
static uint8_t floorCount[6] = {0};
static const uint8_t sendData[] = {0xAA, 0x55, 0xC3, 0x3C, 0x07, 0x01};
static const uint8_t sendDataOk[] = {0xAA, 0x55, 0xC3, 0x3C, 0x01, 0x81};
static const uint8_t sendDataFail[] = {0xAA, 0x55, 0xC3, 0x3C, 0x01, 0xC1};
static const uint8_t keepAlive[] = {0xAA, 0x55, 0xC3, 0x3C, 0x01, 0x02};
static const uint8_t keepAliveOk[] = {0xAA, 0x55, 0xC3, 0x3C, 0x01, 0x82};

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void enableQuery();
static void initGateway(void);
static bool areEqual(uint8_t *v1, uint8_t *v2, uint8_t cant);
static void sendKeepAlive(void);
static void timeout(void);
static OS_MUTEX mutexUART;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void gatewayConnect(void)
{

  bool running = true;
  uint8_t cbn = '\n';

  OSTimeDly(1, OS_OPT_TIME_DLY, &err);
  initGateway();

  while (true)
  {
    OSSemPend(&MySem, 0, OS_OPT_PEND_BLOCKING, 0, &err);

    if (queryEnabled)
    {
      OSMutexPend(&MyMutex, 0, OS_OPT_PEND_BLOCKING, 0, &err);
      floorCount[2 * (floor - 1)]++;
      OSMutexPost(&MyMutex, OS_OPT_POST_NONE, &err);

      // Mandar la data por UART
      OSMutexPend(&mutexUART, 0, OS_OPT_PEND_BLOCKING, 0, &err);

      uartWriteMsg(0, &sendData, SIZEOF(sendData));
      uartWriteMsg(0, &floorCount, SIZEOF(floorCount));
      uartWriteMsg(0, &cbn, 1);

      while (!uartIsRxMsg(0))
        ;

      uartReadMsg(0, &buffer, 6);

      OSMutexPost(&mutexUART, OS_OPT_POST_NONE, &err);

      if (areEqual(&sendDataOk, &buffer, 6))
      {
        // Que hacemos si llego bien?
      }
      else if (areEqual(&sendDataFail, &buffer, 6))
      {
        // Que hacemos si llego mal?
      }
    }

    queryEnabled = false;
  }
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static bool areEqual(uint8_t *v1, uint8_t *v2, uint8_t cant)
{
  bool equal = true;
  for (int i = 0; i < cant && equal; i++)
  {
    if (v1[i] != v2[i])
    {
      equal = false;
    }
  }
  return equal;
}

static void sendKeepAlive(void)
{
  OSMutexPend(&mutexUART, 0, OS_OPT_PEND_BLOCKING, 0, &err);
  uartWriteMsg(0, &keepAlive, SIZEOF(keepAlive));
  OSMutexPost(&mutexUART, OS_OPT_POST_NONE, &err);

  startTimer(timeoutId);
}

static void timeout()
{
  OSMutexPend(&mutexUART, 0, OS_OPT_PEND_BLOCKING, 0, &err);
  uartReadMsg(0, &buffer, 6);
  OSMutexPost(&mutexUART, OS_OPT_POST_NONE, &err);

  if (areEqual(&keepAliveOk, &buffer, 6))
  {
    gpioWrite(PIN_LED_GREEN, HIGH);
    gpioWrite(PIN_LED_RED, LOW);
  }
  else
  {
    gpioWrite(PIN_LED_RED, HIGH);
    gpioWrite(PIN_LED_GREEN, LOW);
  }
}

static void enableQuery(void)
{
  queryEnabled = true;
}

static void initGateway(void)
{
  timerId = createTimer(TBQ, enableQuery, PERIODIC);
  keepAliveId = createTimer(KAF, sendKeepAlive, PERIODIC);
  timeoutId = createTimer(TIMEOUT, timeout, ONESHOT);
  startTimer(timerId);
  startTimer(keepAliveId);
}

/*******************************************************************************
 ******************************************************************************/
