

#include "FSKDem.h"
#include "FIRqueue.h"
#include "Queue.h"
#include "MK64F12.h"
#include "PIT.h"
#include "gpio.h"

#include "uart.h"

#define FILTERTAPS (5) // Number of delta taps un us that must be store to compute filtering
#define LEN(x) (sizeof(x) / sizeof(x[0]))
#define BITTHRESHOLD (8) // Cantidad de bits iguales necesarios para decidir si los samples son 1 o 0
#define THRESHOLD (8)	 // Threshold para determinar si las n muestras del start son iguales
#define SAMPLES (84)	 // Esto lo calculamos (tiempo bit)833 * (cant bits de la palabra despues del start)9/89.2f, cantidad de muestras hasta que termina la lectura de la palabra
#define SENDEVERY (1)

typedef enum
{
	START_DETECT,
	INIT_WAIT,
	READ_STATE,
	WAITANDPROCESS_STATE
} demodulatorStates_t;

// Inicializamos una cola con FILTERTAPS ceros dentro
static uint8_t timerID;
static bool startNow = false;
static FIRqueue_t buffermt = {{0}, 0};
static FIRqueue_t bufferxt = {{0}, 0};
static queue_t output = {{0}, 0, 0, false, true};
static demodulatorStates_t stateDEM = START_DETECT;
static bool errorFlag = false;
static const float FIRConst[] = {
	0.000000000000000000,
	-0.000315768539344851,
	-0.001641663967900394,
	-0.003510424256563872,
	-0.002766793598896892,
	0.006986538156200298,
	0.032649741570846713,
	0.075777199518765062,
	0.127741377143619994,
	0.171072156041247564,
	0.188015275864052728,
	0.171072156041247592,
	0.127741377143620022,
	0.075777199518765062,
	0.032649741570846713,
	0.006986538156200301,
	-0.002766793598896893,
	-0.003510424256563875,
	-0.001641663967900395,
	-0.000315768539344851,
	0.000000000000000000};
int applyFilterAndCompare();
static void startADC();
static bool oddParity(uint8_t word, uint8_t parity);

void initFSKDemodulator()
{
	// Inicializamos el ADC y lo configuramos
	ADC_Init();
	ADC_SetResolution(ADC0, ADC_b16);
	ADC_SetCycles(ADC0, ADC_c24);
	ADC_SetInterruptMode(ADC0, true);
	ADC_SetHardwareAverage(ADC0, ADC_t32);

	timerID = createTimer(89.2f, startADC);
	startTimer(timerID);
}

static void startADC()
{
	ADC_Start(ADC0, 0, ADC_mA);
}

void ADC0_IRQHandler(void)
{
	FSKDemCallback();
	ADC_ClearInterruptFlag(ADC0);
}

void FSKDemCallback(void)
{
	static float xt;
	static float mt = 0;
	static float dt = 0;
	static uint8_t cont = 0;
	static uint8_t stateTAPCounter = 0;
	static int8_t cumsum = 0;
	static uint8_t word = 0;
	static int8_t bitNumber = 7;
	static bool parityCheck = 0;

	xt = ((float)ADC_getData(ADC0) / (32768.0f)) - 1.0f;
	putFIR(&bufferxt, xt);
	mt = xt * readValueFIR(&bufferxt, 5); // m(t) actual
	putFIR(&buffermt, mt);

	switch (stateDEM)
	{
	case START_DETECT:
		dt = applyFilterAndCompare();

		if (dt == 1)
		{
			cont = 0;
		}
		else
		{
			cont++;
		}

		if (cont >= THRESHOLD)
		{
			cont = 0;
			stateDEM = INIT_WAIT;
		}
		break;

	case INIT_WAIT:
		stateTAPCounter++;
		if (stateTAPCounter >= 4)
		{
			stateTAPCounter = 0;
			bitNumber = 7;
			word = 0;
			cumsum = 0;
			stateDEM = READ_STATE;
		}
		break;
		
	case READ_STATE:
		dt = applyFilterAndCompare();
		cumsum += dt;
		stateTAPCounter++;
		if (stateTAPCounter >= 5)
		{
			stateTAPCounter = 0;
			stateDEM = WAITANDPROCESS_STATE;
		}
		break;
	case WAITANDPROCESS_STATE:
		stateTAPCounter++;
		if (stateTAPCounter >= 4 && bitNumber >= 0)
		{
			stateTAPCounter = 0;
			stateDEM = READ_STATE;
			if (cumsum > 0)
			{
				word |= (1 << bitNumber);
			}
			bitNumber--;
			cumsum = 0;
		}
		else if (bitNumber < 0 && stateTAPCounter >= 4)
		{
			stateTAPCounter = 0;
			stateDEM = START_DETECT;
			if (cumsum > 0)
			{
				parityCheck = oddParity(word, 1);
			}
			else
			{
				parityCheck = oddParity(word, 0);
			}
			cumsum = 0;
			if (parityCheck)
			{
				put(&output, word);
			}
			else
			{
				put(&output, word);
				errorFlag = true;
			}
		}

		break;
	}
}

bool isDataReadyHART()
{
	if (getFillLevel(&output) != 0)
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
	char c = getNext(&output);
	return c;
}

int applyFilterAndCompare()
{
	// Computamos el FIR de salida

	float dt = 0;

	for (int i = 0; i < LEN(FIRConst); i++)
	{
		FIRqueuedata_t currT = readValueFIR(&buffermt, i); // Leemos los valores del mas nuevo al mas viejo
		dt += FIRConst[i] * currT;						   // Deberia recorrerse al reves el FIR. Es simetrico el FIR, asi que da lo mismo
	}

	if (dt <= 0)
	{
		dt = 1;
	}
	else
	{
		dt = -1; // 0's are shown as -1. For easy averaging
	}

	return dt;
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
