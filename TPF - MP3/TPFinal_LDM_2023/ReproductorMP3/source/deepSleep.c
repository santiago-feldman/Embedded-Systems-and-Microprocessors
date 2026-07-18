/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "deepSleep.h"

#include "MK64F12.h"
#include "fsl_smc.h"
#include "fsl_llwu.h"
#include "fsl_pmc.h"

#include "config.h"
#include "eventQueue.h"
#include "gpio.h"
#include "App.h"

#define WAKEUP_PIN BUTTON_ENCODER_PIN
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/
static llwu_external_pin_filter_mode_t filter_mode = {9, kLLWU_PinFilterFallingEdge};
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void dummyCB();
/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
            GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void deepSleep_init(void)
{
    gpioIRQ(WAKEUP_PIN, GPIO_IRQ_MODE_RISING_EDGE, &dummyCB);

    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeLls);

    LLWU_SetExternalWakeupPinMode(LLWU, 9, kLLWU_ExternalPinFallingEdge);

    LLWU_SetPinFilterMode(LLWU, 1, filter_mode);

    LLWU_SetResetPinMode(LLWU, false, false);

    NVIC_EnableIRQ(LLWU_IRQn);

}

void deepSleep(void)
{
    volatile unsigned int dummyread;

    SMC_PreEnterStopModes();

    SMC_SetPowerModeLls(SMC);

    __WFI();

    SMC_PostExitStopModes();
}
/*******************************************************************************
 *******************************************************************************
            LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void dummyCB()
{
    return;
}

void LLWU_IRQHandler(void)
{
    LLWU_ClearExternalWakeupPinFlag(LLWU, 9);
    SMC_PostExitStopModes();

    SDK_DelayAtLeastUs(100000U, CLOCK_GetCoreSysClkFreq());

    putEvent(EncoderClick);

    //initDisplay();
    //App_Init(); /* Program-specific setup */
}
