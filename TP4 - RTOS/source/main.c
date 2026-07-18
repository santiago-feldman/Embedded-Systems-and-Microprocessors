/*
#include "hardware.h"

void App_Init(void);
void App_Run(void);

int main(void)
{
  hw_Init();
  hw_DisableInterrupts();
  App_Init();
  hw_EnableInterrupts();

  __FOREVER__
  App_Run();
}
*/

#include "hardware.h"
#include <os.h>
#include "App.h"
#include <gateway.h>
#include "threads.h"

/* Task 2 */
#define GATEWAY_STK_SIZE 256u
#define GATEWAY_STK_SIZE_LIMIT (GATEWAY_STK_SIZE / 10u)
#define GATEWAY_PRIO 3u
static OS_TCB GatewayTCB;
static CPU_STK GatewayStk[GATEWAY_STK_SIZE];

/* APP */
#define APP_STK_SIZE 512u // MODIFICAR COMO SEA NECESARIO
#define APP_STK_SIZE_LIMIT (APP_STK_SIZE / 10u)
#define APP_PRIO 2u
static OS_TCB AppTCB;
static CPU_STK AppStk[APP_STK_SIZE];

int main(void)
{

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
  CPU_ERR cpu_err;
#endif

  hw_Init();

  OSInit(&err);
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
  /* Enable task round robin. */
  OSSchedRoundRobinCfg((CPU_BOOLEAN)1, 0, &err);
#endif
  OS_CPU_SysTickInit(SystemCoreClock / (uint32_t)OSCfg_TickRate_Hz);

  /* Initialize the uC/CPU Services. */
  CPU_Init();

#if OS_CFG_STAT_TASK_EN > 0u
  /* (optional) Compute CPU capacity with no task running */
  OSStatTaskCPUUsageInit(&err);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
  CPU_IntDisMeasMaxCurReset();
#endif

  /* Create semaphore */
  OSSemCreate(&MySem, "Semaphore", 0u, &err);

  OSSemCreate(&FSMsem, "SemaphoreFSM", 0u, &err);


  OSMutexCreate(&MyMutex, "Mutex", &err);

  /* Create Task2 */
  OSTaskCreate(&AppTCB,            // tcb
               "App",              // name
               App_Run,            // func
               0u,                 // arg
               APP_PRIO,           // prio
               AppStk,             // stack
               APP_STK_SIZE_LIMIT, // stack limit
               APP_STK_SIZE,       // stack size
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);


    OSTaskCreate(&GatewayTCB,
                 "App Task Start",
                 gatewayConnect,
                 0u,
                 GATEWAY_PRIO,
                 &GatewayStk[0u],
                 (GATEWAY_STK_SIZE / 10u),
                 GATEWAY_STK_SIZE,
                 0u,
                 0u,
                 0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 &err);


  OSStart(&err);

  /* Should Never Get Here */
  while (1)
  {
  }
}
