
// #include "timer.h"
#include "cardReader.h"
#include "hardware.h"
#include "Queue.h"
#include "gpio.h"
#include "config.h"

#define SS (0x0B)
#define FS (0x0D)
#define ES (0x1F)

#define TRACK2SIZE (40)
#define IDSIZE (8)

#define LEN(v) (sizeof(v) / sizeof(v[0]))

enum state
{
	IDLE,
	PAN,
	AD,
	DD,
	LRC,
	FINISHED,
	ERROR
};

typedef struct
{
	uint8_t bits : 4;
	uint8_t parity : 1;
} CHAR_t;

typedef struct
{
	CHAR_t ss;
	CHAR_t pan[19];
	CHAR_t fs;
	CHAR_t ad[7];
	CHAR_t dd[8];
	CHAR_t es;
	CHAR_t lrc;
} WORD_t;

typedef union
{
	CHAR_t chars[38];
	WORD_t word;
} TRACK2_t;

static uint32_t ID = NOID;
static uint8_t state = IDLE;
static uint8_t bitCount = 0;
static uint8_t charCount = 0;

static TRACK2_t buffer = {.chars = 0};

static void getBit();
static void reset();
static void convertToID(CHAR_t str[]);
static bool checkLRC(CHAR_t str[]);
static bool checkCharParity(uint8_t curr);
static bool areEqual(uint8_t ch1, uint8_t ch2);
static void storeChar(CHAR_t *ch, uint8_t read);
static void checkEnable();
static uint32_t myAtoi(CHAR_t str[]);

void initCardReader()
{

	gpioMode(DATA, INPUT);
	gpioMode(ENABLE, INPUT);
	gpioMode(CLK, INPUT);
	gpioIRQ(CLK, GPIO_IRQ_MODE_FALLING_EDGE, &getBit);
	gpioIRQ(ENABLE, GPIO_IRQ_MODE_RISING_EDGE, &checkEnable);
	reset();
}

uint32_t getID()
{
	return ID;
}

static bool areEqual(uint8_t ch1, uint8_t ch2)
{
	return (ch1 & 0b00011111) == (ch2 & 0b00011111);
}

static void storeChar(CHAR_t *ch, uint8_t read)
{
	ch->bits = read & 0b00001111;
	ch->parity = (read & 0b00010000) >> 4;
}

static void reset()
{
	bitCount = 0;
	charCount = 0;
	for (int i = 0; i < TRACK2SIZE; i++)
	{
		buffer.chars[i].bits = 0;
		buffer.chars[i].parity = 0;
	}
}

static void checkEnable()
{
	if (state != FINISHED && state != ERROR && state != IDLE)
	{
		state = ERROR;
		putEvent(CardReadError);
	}
}

static void getBit()
{

	static uint8_t currRead = 0;

	uint8_t bit = !gpioRead(DATA);
	currRead = (currRead >> 1) | (bit << 4);

	switch (state)
	{
	case IDLE:
		if (areEqual(currRead, SS))
		{
			storeChar(&(buffer.chars[charCount]), SS);
			charCount++;
			state = PAN;
			bitCount = 0;
		}
		break;

	case PAN:
		if (bitCount >= 4)
		{

			if (areEqual(currRead, FS))
			{
				storeChar(&(buffer.word.fs), currRead);
				state = AD;
				charCount = LEN(buffer.word.pan) + 1;
				bitCount = 0;
			}
			else{
				if (checkCharParity(currRead) == false)
					{
						state = ERROR;
						putEvent(CardReadError);
					}
				storeChar(&(buffer.chars[charCount]), currRead);
				charCount++;
				bitCount = 0;
			}
		}
		else
		{
			bitCount++;
		}

		break;

	case AD:
		if (bitCount >= 4)
		{
			if (charCount == (LEN(buffer.word.pan) + 1 + LEN(buffer.word.ad)))
			{
				storeChar(&(buffer.chars[charCount]), currRead);
				charCount++;
				state = DD;
				bitCount = 0;
			}
			else{
				if (checkCharParity(currRead) == false)
					{
						state = ERROR;
						putEvent(CardReadError);
					}
				storeChar(&(buffer.chars[charCount]), currRead);
				charCount++;
				bitCount = 0;
			}
		}
		else
		{
			bitCount++;
		}

		break;

	case DD:
		if (bitCount >= 4)
		{
			if (areEqual(currRead, ES))
			{
				storeChar(&(buffer.word.es), currRead);
				charCount++;
				bitCount = 0;
				state = LRC;
			}
			else{
				if (checkCharParity(currRead) == false)
					{
						state = ERROR;
						putEvent(CardReadError);

					}
				storeChar(&(buffer.chars[charCount]), currRead);
				charCount++;
				bitCount = 0;
			}
		}
		else
		{
			bitCount++;
		}

		break;

	case LRC:
		if (bitCount >= 4){
			if (checkCharParity(currRead) == false)
			{
					state = ERROR;
					putEvent(CardReadError);
					break;
			}
			storeChar(&(buffer.word.lrc), currRead);

			/*if (checkLRC(buffer.chars) == true)
			{
				state = ERROR;
				putEvent(CardReadError);
				break;
			}
			else*/{
				state = FINISHED;

				break;
			}

		}
		else
		{
			bitCount++;
		}

		break;

	case FINISHED:

		convertToID(buffer.chars);
		putEvent(CardRead);
		reset();
		state = IDLE;
		break;

	case ERROR:
		reset();
		state = IDLE;
		break;
	}
}

static bool checkCharParity(uint8_t curr)
{

	uint8_t cont = 0;
	uint8_t bits = curr & 0b11111;

	for (int i = 0; i < 5; ++i)
	{
		cont += bits & 1;
		bits >>= 1;
	}

	return cont % 2; // 1 si es impar, 0 si es par
}

// Se puede optimizar calculando la suma de los bits a medida que se van leyendo por data
static bool checkLRC(CHAR_t* str)
{

	uint8_t cont = 0;
	bool error = false;

	for (int i = 0; i < 4 && !error; i++)
	{

		for (int j = 0; j < TRACK2SIZE; j++)
		{
			cont += (str[j].bits >> i) & 0b0001;

		}

		if (cont % 2 == 0)
		{
			error = true;
		}

		cont = 0;
	}

	return error;
}

static void convertToID(CHAR_t str[])
{
	ID = myAtoi(str);
}

static uint32_t myAtoi(CHAR_t str[])
{
	// Initialize result
	uint32_t res = 0;

	for (int i = 0; i < IDSIZE; i++)
	{
		res = res * 10 + str[i + 1].bits;
	}

	return res;
}
