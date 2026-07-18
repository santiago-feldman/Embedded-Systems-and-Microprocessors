/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "biquad.h"
#include <math.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define EPSILON (1e-6) // EPSILON value to make float comparisons

#define FS (44100.0f)   // sampling frequency in Hz
#define DB2TIMES(x) (pow(10, (x) / 20))
#define GN (10) // normalization gain in dB (depend on gain when A matrix was calculated)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct
{
    float G;     // peak gain
    float cosWc; // center frecuency in rad/s
    float B;     // bandwidth
    float Gb;    // gain at bandwith B
    float beta;  // beta value
    float x[2];  // last two computed inputs, store x(n-1) and x(n-2), respectively
    float y[2];  // last two computed outputs, store y(n-1) and y(n-2), respectively
} biquad_t;

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

static const float filterQ[BANDS] = {1.23f, 1.23f, 1.3f, 1.3f, 1.23f, 1.4f, 1.39f, 1.49f};

// Filter parameters
static const uint16_t centerFreqs[BANDS] = {34, 80, 190, 450, 1100, 2500, 6000, 14200}; // In Hz

// Target amplitud of each filter, equation g = A^-1 . t
static float g[BANDS]; // In dB
static float t[BANDS]; // In dB
static const float A[BANDS][BANDS] = {	{1.000000f, 0.116971f, 0.017231f, 0.002969f, 0.000492f, 0.000094f, 0.000015f, 0.000001f},
										{0.116976f, 1.000000f, 0.114335f, 0.016989f, 0.002740f, 0.000519f, 0.000081f, 0.000007f},
										{0.015469f, 0.104019f, 1.000000f, 0.104674f, 0.014312f, 0.002637f, 0.000409f, 0.000033f},
										{0.002664f, 0.015272f, 0.104806f, 1.000000f, 0.096643f, 0.015350f, 0.002312f, 0.000187f},
										{0.000498f, 0.002770f, 0.016116f, 0.107201f, 1.000000f, 0.125835f, 0.016116f, 0.001265f},
										{0.000076f, 0.000422f, 0.002398f, 0.013963f, 0.104705f, 1.000000f, 0.081895f, 0.005435f},
										{0.000016f, 0.000087f, 0.000490f, 0.002762f, 0.017069f, 0.104253f, 1.000000f, 0.049675f},
										{0.000006f, 0.000032f, 0.000181f, 0.001013f, 0.006055f, 0.031334f, 0.183376f, 1.000000f}}; // In dB

static biquad_t filter[BANDS];

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static float getBeta(float G, float B, float Gb);

static void choleskyDecomposition(float A[][BANDS], float L[][BANDS], int n);
static void forwardSubstitution(float L[][BANDS], float B[], float Y[], int n);
static void backwardSubstitution(float L[][BANDS], float Y[], float X[], int n);
static void solveLinear(float A[][BANDS], float b[], float x[], int n);

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

void initFilters()
{
    solveLinear(A, t, g, BANDS);


    resetFilters();

    for (int i = 0; i < BANDS; i++)
    {
        filter[i].cosWc = cos(2 * PI * centerFreqs[i] / FS);
        filter[i].B = 2 * PI * centerFreqs[i] / (filterQ[i] * FS);
        filter[i].G = DB2TIMES(g[i]);
        filter[i].Gb = filter[i].G / 2;
        filter[i].beta = sqrt(fabs(pow(filter[i].Gb, 2) - 1) / fabs(pow(filter[i].G, 2) - pow(filter[i].Gb, 2))) * tan(filter[i].B / 2);
    }

}

/*
    Biquad difference equation
    y(n) =  [(1+G.b) * x(n)  - 2cos(wc) x(n-1) +
            (1-G.b) * x(n-2) + 2cos(wc) y(n-1) - (1-b) y(n-2)] /(1+b)
*/

float computeFilters(float x)
{
    float G, beta, cosWc, y, output;

    for (int i = 0; i < BANDS; i++)
    {
        G = filter[i].G;
        cosWc = filter[i].cosWc;
        beta = filter[i].beta;

        y = ((1 + G * beta) * x - 2 * cosWc * filter[i].x[0] + (1 - G * beta) * filter[i].x[1] + 2 * cosWc * filter[i].y[0] - (1 - beta) * filter[i].y[1]) / (1 + beta);

        // update filter input taps
        filter[i].x[1] = filter[i].x[0];
        filter[i].x[0] = x;

        // update filter output taps
        x = filter[i].y[1];
        filter[i].y[1] = filter[i].y[0];
        filter[i].y[0] = y;

        output = y; // At the end of loop we have the last filter output
    }

    return output;
}

void resetFilters()
{

    for (int i = 0; i < BANDS; i++)
    {
    	for (int j = 0; j < 2; j++)
    	{
    		filter[i].y[j] = 0;
    		filter[i].x[j] = 0;
    	}
    }
}

void setGain(int8_t value[]) // value in dB
{

	for (int i = 0; i < BANDS; i++)
	{
		t[i] = value[i];
	}

    // Recalculamos el vector g resolviendo A*g = t
    solveLinear(A, g, t, BANDS);

    for (int i = 0; i < BANDS; i++)
    {
        filter[i].G = DB2TIMES(g[i]);
        filter[i].Gb = filter[i].G / 2;
        filter[i].beta = sqrt(fabs(pow(filter[i].Gb, 2) - 1) / fabs(pow(filter[i].G, 2) - pow(filter[i].Gb, 2))) * tan(filter[i].B / 2);
    }
}

float getGain(int8_t filterID){
	return t[filterID];
}

/*******************************************************************************
 *******************************************************************************
            LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void solveLinear(float A[][BANDS], float b[], float x[], int n)
{

    float L[][BANDS] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};
    float y[BANDS] = {0};

    choleskyDecomposition(A, L, n);

    // Solve Ly = b for Y using forward substitution
    forwardSubstitution(L, b, y, n);

    // Solve L^Tx = y for x using backward substitution
    backwardSubstitution(L, y, x, n);
}

static void forwardSubstitution(float L[][BANDS], float B[], float Y[], int n)
{
    for (int i = 0; i < n; i++)
    {
        float sum = 0.0;
        for (int j = 0; j < i; j++)
        {
            sum += L[i][j] * Y[j];
        }
        Y[i] = (B[i] - sum) / L[i][i];
    }
}

static void backwardSubstitution(float L[][BANDS], float Y[], float X[], int n)
{
    for (int i = n - 1; i >= 0; i--)
    {
        float sum = 0.0;
        for (int j = i + 1; j < n; j++)
        {
            sum += L[j][i] * X[j];
        }
        X[i] = (Y[i] - sum) / L[i][i];
    }
}

static void choleskyDecomposition(float A[][BANDS], float L[][BANDS], int n)
{

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            float sum = 0.0;

            // Si estas en la diagonal principal
            if (j == i)
            {
                for (int k = 0; k < j; k++)
                {
                    sum += L[j][k] * L[j][k];
                }

                L[j][j] = sqrt(A[j][j] - sum);
            }
            else
            {
                for (int k = 0; k < j; k++)
                {
                    sum += L[i][k] * L[j][k];
                }

                L[i][j] = (A[i][j] - sum) / L[j][j];
            }
        }
    }
}
