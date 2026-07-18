/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   Grupo 3
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "biquad.h"
#include <arm_math.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define EPSILON (1e-6) // EPSILON value to make float comparisons

#define FS (44100.0f) // sampling frequency in Hz
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
static const float centerFreqs[BANDS] = {34.0f, 80.0f, 190.0f, 450.0f, 1100.0f, 2500.0f, 6000.0f, 14200.0f}; // In Hz

// Target amplitud of each filter, equation g = A^-1 . t
static float g[BANDS];                            // In dB
static float t[BANDS] = {0, 0, 0, 0, 0, 0, 0, 0}; // In dB
static const float Ainv[BANDS][BANDS] = {{1.013886f, -0.118181f, -0.003896f, -0.000588f, -0.000062f, -0.000006f, -0.000001f, -0.000000f},
                                         {-0.118210f, 1.025838f, -0.114718f, -0.005001f, -0.000621f, -0.000063f, -0.000008f, -0.000000f},
                                         {-0.003330f, -0.104414f, 1.023053f, -0.104850f, -0.004168f, -0.000503f, -0.000059f, -0.000003f},
                                         {-0.000540f, -0.004347f, -0.104983f, 1.021494f, -0.096866f, -0.003159f, -0.000494f, -0.000023f},
                                         {-0.000065f, -0.000628f, -0.004859f, -0.107450f, 1.023807f, -0.126551f, -0.005831f, -0.000298f},
                                         {-0.000005f, -0.000047f, -0.000424f, -0.002705f, -0.105323f, 1.021859f, -0.081732f, -0.001360f},
                                         {-0.000001f, -0.000008f, -0.000073f, -0.000646f, -0.006142f, -0.103756f, 1.017764f, -0.049986f},
                                         {-0.000000f, -0.000002f, -0.000019f, -0.000162f, -0.001674f, -0.012223f, -0.184037f, 1.009211f}}; // In dB

static float pState[4 * BANDS] = {0};
static float pCoeffs[BANDS * 5];
static arm_biquad_casd_df1_inst_f32 Sequ;

static biquad_t filter[BANDS];

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

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
    solveLinear(Ainv, t, g, BANDS);

    resetFilters();

    for (int i = 0; i < BANDS; i++)
    {
        filter[i].cosWc = cos(2 * PI * centerFreqs[i] / FS);
        filter[i].B = 2.0f * PI * centerFreqs[i] / (filterQ[i] * FS);

        if (g[i] == 0.0f)
        {
            pCoeffs[i * 5] = 1;
            pCoeffs[i * 5 + 1] = 0;
            pCoeffs[i * 5 + 2] = 0;
            pCoeffs[i * 5 + 3] = 0;
            pCoeffs[i * 5 + 4] = 0;
            continue;
        }

        filter[i].G = DB2TIMES(g[i]);
        filter[i].Gb = filter[i].G / 2;
        filter[i].beta = sqrtf(fabsf(powf(filter[i].Gb, 2) - 1.0f) / fabsf(powf(filter[i].G, 2) - powf(filter[i].Gb, 2))) * tanf(filter[i].B / 2.0f);

        pCoeffs[i * 5] = (1.0f + filter[i].G * filter[i].beta) / (1.0f + filter[i].beta);     // b0
        pCoeffs[i * 5 + 1] = (-2.0f * filter[i].cosWc) / (1.0f + filter[i].beta);             // b1
        pCoeffs[i * 5 + 2] = (1.0f - filter[i].G * filter[i].beta) / (1.0f + filter[i].beta); // b2

        pCoeffs[i * 5 + 3] = (2.0f * filter[i].cosWc / (1.0f + filter[i].beta)); // -a1.
        pCoeffs[i * 5 + 4] = -(1.0f - filter[i].beta) / (1.0f + filter[i].beta); // -a2.
    }

    arm_biquad_cascade_df1_init_f32(&Sequ, 8, pCoeffs, pState);
}

/*
    Biquad difference equation
    y(n) =  [(1+G.b) * x(n)  - 2cos(wc) x(n-1) +
            (1-G.b) * x(n-2) + 2cos(wc) y(n-1) - (1-b) y(n-2)] /(1+b)
*/

void computeFilters(float *in, float *out, uint32_t blockSize)
{

    arm_biquad_cascade_df1_f32(&Sequ, in, out, blockSize);

    return;
}

void resetFilters()
{
    memset(pState, 0, sizeof(pState));
}

void setGain(float value[BANDS]) // value in dB
{

    for (int i = 0; i < BANDS; i++)
    {
        t[i] = value[i];
    }

    // Recalculamos el vector g resolviendo A*g = t
    solveLinear(Ainv, t, g, BANDS);
    resetFilters();
    for (int i = 0; i < BANDS; i++)
    {
        if (g[i] == 0)
        {
            pCoeffs[i * 5] = 1;
            pCoeffs[i * 5 + 1] = 0;
            pCoeffs[i * 5 + 2] = 0;
            pCoeffs[i * 5 + 3] = 0;
            pCoeffs[i * 5 + 4] = 0;
            continue;
        }
        filter[i].G = DB2TIMES(g[i]);
        filter[i].Gb = filter[i].G / 2;
        filter[i].beta = sqrtf(fabsf(powf(filter[i].Gb, 2) - 1.0f) / fabsf(powf(filter[i].G, 2) - powf(filter[i].Gb, 2))) * tanf(filter[i].B / 2.0f);

        pCoeffs[i * 5] = (1.0f + filter[i].G * filter[i].beta) / (1.0f + filter[i].beta);     // b0
        pCoeffs[i * 5 + 1] = (-2.0f * filter[i].cosWc) / (1.0f + filter[i].beta);             // b1
        pCoeffs[i * 5 + 2] = (1.0f - filter[i].G * filter[i].beta) / (1.0f + filter[i].beta); // b2

        pCoeffs[i * 5 + 3] = (2.0f * filter[i].cosWc / (1.0f + filter[i].beta)); // -a1.
        pCoeffs[i * 5 + 4] = -(1.0f - filter[i].beta) / (1.0f + filter[i].beta); // -a2.
    }
}

float getGain(int8_t filterID)
{
    return t[filterID];
}

/*******************************************************************************
 *******************************************************************************
            LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void solveLinear(float Ainv[][BANDS], float b[], float x[], int n)
{
    float sum;
    for (int i = 0; i < n; i++)
    {
        sum = 0;
        for (int j = 0; j < n; j++)
        {
            sum += Ainv[i][j] * b[j];
        }
        x[i] = sum;
    }
}
