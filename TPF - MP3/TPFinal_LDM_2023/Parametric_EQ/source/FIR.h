/***************************************************************************//**
  @file     FIR.h
  @brief    
  @author   
 ******************************************************************************/

#ifndef _FIR_H_
#define _FIR_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define FIR_FILTER_LENGTH 16

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct{
    float buf[FIR_FILTER_LENGTH];   // Circular buffer
    uint8_t bufIndex;               // Index for the circular buffer
    float out;                      // Current filter output
} FIRFilter;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Clears the buffer, clears the output and resets the index of the FIR filter struct
 * @param fir pointer to the FIR filter struct
 */
void FIRFilter_Init(FIRFilter *fir);

/**
 * @brief Clears the buffer, clears the output and resets the index of the FIR filter struct
 * @param fir pointer to the FIR filter struct
 * @param inp current input sample
 * @return filtered output
 */
float FIRFilter_Update(FIRFilter *fir; float inp);

/*******************************************************************************
 ******************************************************************************/

#endif // _FIR_H_
