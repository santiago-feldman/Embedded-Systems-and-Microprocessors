/***************************************************************************/ /**
  @file     main.c
  @brief    FW main
  @author   Grupo 3
  ******************************************************************************/
#include "hardware.h"
#include "App.h"

int main(void)
{
    hw_Init();
    App_Init(); /* Program-specific setup */

    __FOREVER__
    App_Run(); /* Program-specific loop  */
}
