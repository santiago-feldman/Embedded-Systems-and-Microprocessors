
#include <stdio.h>
#include <stdlib.h>
#include "FSKDem.h"
#include "Queue.h"
#include "MK64F12.h"
#include "PIT.h"
#include "uart.h"

#include "FTM.h"
#include "DMA.h"

#define MAX_COUNT 50000
#define F0_THRESHHOLD 14000
#define F1_THRESHHOLD 19000

typedef enum
{
    START_DETECT,
    INIT_WAIT,
    READ_STATE,
    RESET,

} demodulatorStates_t;

typedef enum
{
    PORT_mAnalog,
    PORT_mGPIO,
    PORT_mAlt2,
    PORT_mAlt3,
    PORT_mAlt4,
    PORT_mAlt5,
    PORT_mAlt6,
    PORT_mAlt7,
} PORTMux_t;

typedef enum
{
    PORT_eDisabled = 0x00,
    PORT_eDMARising = 0x01,
    PORT_eDMAFalling = 0x02,
    PORT_eDMAEither = 0x03,
    PORT_eInterruptDisasserted = 0x08,
    PORT_eInterruptRising = 0x09,
    PORT_eInterruptFalling = 0x0A,
    PORT_eInterruptEither = 0x0B,
    PORT_eInterruptAsserted = 0x0C,
} PORTEvent_t;

void FSKDemCallback(void);
void FSKDemCallback2(void);

static uint8_t timerID;
static uint16_t InputCaptureOut = 0;
static uint16_t lastCapture = 0;
static uint32_t timeDiff = 0;
static queue_t outputQueue = {{0}, 0, 0, false, true};
static demodulatorStates_t stateDEM = START_DETECT;
static bool errorFlag = false;

static uint32_t counter = 0;

static bool oddParity(uint8_t word, uint8_t parity);

void initFSKDemodulator()
{
    FTM_Init();
    DMA_Init();

    timerID = createTimer(833.0f, FSKDemCallback2);

    // Config Comparator 0
    SIM->SCGC4 |= SIM_SCGC4_CMP_MASK; // Clock Gating

    PORTC->PCR[3] = PORT_PCR_MUX(PORT_mAnalog) | PORT_PCR_IRQC(PORT_eDisabled);
    PORTC->PCR[4] = PORT_PCR_MUX(PORT_mAlt6) | PORT_PCR_DSE(true) | PORT_PCR_IRQC(PORT_eDisabled);

    CMP1->CR0 = CMP_CR0_FILTER_CNT(7) | CMP_CR0_HYSTCTR(3);                      // Filtro de 3 muestras e histeresis de 30mv
    CMP1->CR1 = CMP_CR1_EN(true) | CMP_CR1_OPE(1) | CMP_CR1_PMODE(1);            // Config High Speed, salida filtrada Habilitado el pin y el modulo
    CMP1->FPR = 0x20;                                                            // Cuantos ciclos de bus clock quiero usar para que el filtro filtre (Periodo de filtrado)
    CMP1->DACCR = CMP_DACCR_DACEN(1) | CMP_DACCR_VOSEL(31) | CMP_DACCR_VRSEL(1); // Habilito el DAC y hago que divida Vref por la mitad, Vref es Vin1
    CMP1->MUXCR = CMP_MUXCR_PSEL(1) | CMP_MUXCR_MSEL(7);

    //  Set FTM configuration
    FTM_SetPrescaler(FTM1, FTM_PSC_x1);
    FTM_SetInterruptMode(FTM1, FTM_CH_0, false); // Enable interrupts
    FTM_SetModulus(FTM1, MAX_COUNT);

    //	Set FTM as InputCompare mode
    FTM_SetWorkingMode(FTM1, FTM_CH_0, FTM_mInputCapture); // MSA  / B
    FTM_SetInputCaptureEdge(FTM1, FTM_CH_0, FTM_eEither);
    FTM_SetInputCaptureChannelSource(FTM1, CMP1_OUTPUT);

    // Config DMA
    FTM_DmaMode(FTM1, FTM_CH_0, true);
    FTM_SetInterruptMode(FTM1, FTM_CH_0, true);

    DMA_SetSourceModulo(DMA_CH1, 0);
    DMA_SetDestModulo(DMA_CH1, 0);

    DMA_SetSourceAddr(DMA_CH1, (uint32_t) & (FTM1->CONTROLS[FTM_CH_0].CnV));
    DMA_SetDestAddr(DMA_CH1, (uint32_t)(&InputCaptureOut));

    DMA_SetSourceAddrOffset(DMA_CH1, 0);
    DMA_SetDestAddrOffset(DMA_CH1, 0);

    DMA_SetSourceLastAddrOffset(DMA_CH1, 0);
    DMA_SetDestLastAddrOffset(DMA_CH1, 0);

    DMA_SetSourceTransfSize(DMA_CH1, DMA_TransSize_16Bit);
    DMA_SetDestTransfSize(DMA_CH1, DMA_TransSize_16Bit);

    DMA_SetMinorLoopTransCount(DMA_CH1, 2);

    DMA_SetCurrMajorLoopCount(DMA_CH1, 1);
    DMA_SetStartMajorLoopCount(DMA_CH1, 1);

    DMA_SetEnableRequest(DMA_CH1, true);

    DMAMUX_ConfigChannel(DMA_CH1, true, false, kDmaRequestMux0FTM1Channel0);
    DMA_SetChannelInterrupt(DMA_CH1, true, &FSKDemCallback);
    FTM_ClearInterruptFlag(FTM1, FTM_CH_0);
    FTM_ClearOverflowFlag(FTM1);
    FTM_StartClock(FTM1);
}

void FSKDemCallback(void)
{

    static uint32_t semiPeriodcounter = 0;

    timeDiff = InputCaptureOut >= lastCapture ? InputCaptureOut - lastCapture : MAX_COUNT - lastCapture + InputCaptureOut;

    //	itoa(timeDiff > 15000, msg, 10);
    //	uartWriteString(0, msg);
    //	uartWriteMsg(0, &msg2, 1);

    switch (stateDEM)
    {
    case START_DETECT:
        if (timeDiff < F0_THRESHHOLD + 3000)
        {
            stateDEM = INIT_WAIT;
        }
        break;
    case INIT_WAIT:
        if (semiPeriodcounter == 1)
        {
            stateDEM = READ_STATE;
            startTimer(timerID);

            semiPeriodcounter = 0;
            break;
        }
        semiPeriodcounter++;
        break;
    case RESET:
    case READ_STATE:
        break;
    default:
        stateDEM = START_DETECT;
        break;
    }

    lastCapture = InputCaptureOut;
}

void FSKDemCallback2(void)
{
    static uint8_t output = 0;
    static uint32_t bitCounter = 0;
    static bool parity = 0;
    static char msg[7] = {0};
    static char msg2 = '\n';

    switch (stateDEM)
    {
    case READ_STATE:
        if (bitCounter == 8)
        {
            parity |= timeDiff > F0_THRESHHOLD ? 1 : 0;
            bitCounter = 0;
            stateDEM = RESET;
            break;
        }
        output |= timeDiff > F0_THRESHHOLD ? 1 << (7 - bitCounter) : 0;
        bitCounter++;
        break;
    case RESET:
        stopTimer(timerID);
        if (oddParity(output, parity))
        {
            put(&outputQueue, output);
        }
        else
        {
            errorFlag = 1;
        }
        output = 0;
        parity = 0;
        stateDEM = START_DETECT;
        //        itoa(counter, msg, 10);
        //        uartWriteString(0, msg);
        //        uartWriteMsg(0, &msg2, 1);
        break;
    case INIT_WAIT:
    case START_DETECT:
    default:
        stopTimer(timerID);
        stateDEM = START_DETECT;
        output = 0;
        parity = 0;
        break;
    }
}

bool isDataReadyHART()
{
    if (getFillLevel(&outputQueue) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

char getNextValueHART(void)
{
    return getNext(&outputQueue);
}

static bool oddParity(uint8_t word, uint8_t parity)
{
    uint8_t i = 0;
    uint8_t parityCalc = 0;
    for (i = 0; i < 8; i++)
    {
        if (word >> i & 1)
        {
            parityCalc ^= 1;
        }
    }

    // Le calculo la paridad impar a la palabra que llego y luego retorno true o false segun
    // si es igual al bit de paridad dado

    return (parityCalc == parity);
}
