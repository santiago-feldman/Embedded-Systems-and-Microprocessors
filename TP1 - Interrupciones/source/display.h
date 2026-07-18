/***************************************************************************//**
  @file     display.h
  @brief    Display driver
  @author   Santiago Feldman
 ******************************************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

#define VDISP_SIZE		10U					// Number of slots in the virtual display

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes display pins and timers
 * @param void
 * @return void
 */
void display_init(void);

/**
 * @brief Sets the cursor of the virtual display
 * @param pos position for the cursor to be set
 * @return void
 */
void set_cursor(uint32_t pos);

/**
 * @brief Sets the window of the virtual display
 * @param pos position for the window to be set
 * @return void
 */
void set_window_offset(uint32_t pos);

/**
 * @brief Input a word to the virtual display
 * @param word string containing the word
 * @return void
 */
void write_word(char* word);

/**
 * @brief Input a character to the virtual display
 * @param character ascii character
 * @return void
 */
void write_char(char character);

/**
 * @brief Clears all characters from display
 * @param void
 * @return void
 */
void clear_display(void);

/**
 * @brief Delete character the cursor is pointing to
 * @param void
 * @return void
 */
void delete(void);

/**
 * @brief Enable sliding in display
 * @param a on/off
 * @return void
 */
void enable_sliding(bool a);

/**
 * @brief Configures the brightness level
 * @param brightness brightness level
 * @return void
 */
void set_brightness(uint32_t brightness);

/*******************************************************************************
 ******************************************************************************/

#endif // _	DISPLAY_H_
