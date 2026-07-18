/***************************************************************************//**
  @file     display.c
  @brief    Display driver
  @author   Santiago Feldman
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "display.h"
#include "displaydef.h"
#include "timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// MASKS
#define IS_LETTER_MSK	0b01000000			// Mask to check whether the character is a letter or a word
#define LETTER_MSK		0b11111				// Mask to get the letter
#define NUMBER_MSK		0b1111				// Mask to get the number

// TIMES
#define REFRESHING_TIME	5
#define SLIDER_TIME		500
#define CURSOR_TIME 	500

#define LETTER_OFFSET	9U					// Offset to get to the letters in the array of characters
#define WINDOW_SIZE		4U					// Size of the window that the display shows

#define EOL				'\0'				// End of line

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Callback function to show the 4 seven segment displays
 * @param void
 * @return void
 */
static void show_display(void);

/**
 * @brief Input a given character in a seven segment display
 * @param character ascii character
 * @return void
 */
static void write_display(uint8_t character);

/**
 * @brief Sets virtual display indexes within the scope of the array
 * @param index index to normalize
 * @return normalized index
 */
static uint32_t normalize_index(uint32_t index);

/**
 * @brief Callback function that moves the window to be shown
 * @param void
 * @return void
 */
static void slider(void);

/**
 * @brief Callback function to turn off the segments of the display, making the brightness lower
 * @param void
 * @return void
 */
static void turn_off_segs(void);

/**
 * @brief Callback function to show the cursor on the display
 * @param void
 * @return void
 */
static void show_cursor(void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static uint8_t virtual_display[VDISP_SIZE] = {' '};	  // Virtual display
static uint32_t cursor = 0;							  // Virtual display cursor
static bool is_cursor_shown = false;				  // Flag used to show the cursor in the display
static uint32_t window_offset = 0;					  // Window to be showcased in the display

static uint32_t display_timer_id = 0;				  // Timer to refresh display
static uint32_t slider_timer_id = 0;					  // Timer for sliding pace
static uint32_t brightness_timer_id = 0;				  // Timer for brightness level
static uint32_t cursor_timer_id = 0;

static bool slider_enable = false;					  // Checks if the sliding is enabled or not
static uint8_t brightness_timer = REFRESHING_TIME;

void display_init(void)
{
	// Initialize display timer
	display_timer_id = createTimer(REFRESHING_TIME, &show_display, PERIODIC);
	startTimer(display_timer_id);

	// Initialize slider timer
	slider_timer_id = createTimer(SLIDER_TIME, &slider, PERIODIC);
	startTimer(slider_timer_id);

	// Initialize brightness timer
	brightness_timer_id = createTimer(brightness_timer, &turn_off_segs, ONESHOT);

	// Initialize cursor timer
	cursor_timer_id = createTimer(CURSOR_TIME, &show_cursor, PERIODIC);
	startTimer(cursor_timer_id);

	// Initialize segment pins
	int i;
	for(i=0; i<SEG_LENGTH; ++i)
		gpioMode(segment_pins[i], OUTPUT);

	// Initialize enconder
	gpioMode(ENC1_PIN, OUTPUT);
	gpioMode(ENC2_PIN, OUTPUT);

	clear_display();
}

void set_cursor(uint32_t pos)
{
	cursor = (pos < VDISP_SIZE) ? pos : cursor;
}

void set_window_offset(uint32_t pos)
{
	window_offset = (pos < VDISP_SIZE) ? pos : window_offset;
}

void write_word(char* word)
{
	while(*word != EOL)
	{
		write_char(*word);
		cursor = (cursor == VDISP_SIZE-1) ? 0 : cursor+1;
		++word;
	}

}

void write_char(char character)
{
	virtual_display[cursor] = character;
}

void clear_display(void)
{
	int i;
	for(i=0; i<VDISP_SIZE; ++i)
		virtual_display[i] = ' ';

	//cursor = 0;
}

void delete(void)
{
	virtual_display[cursor] = 0;
}

void enable_sliding(bool a)
{
	slider_enable = a;
}

void set_brightness(uint32_t brightness)
{
	brightness_timer = brightness/20;
	configTimerTime(brightness_timer_id, brightness_timer);
}

void move_window(uint32_t pos)
{
	window_offset = normalize_index(pos);
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void show_display(void)
{
	static uint8_t display_num = 0;											// Number of display to be written
	uint32_t num_index = normalize_index(window_offset+display_num);		// Position in array

	bool enc1_val = (display_num & 0b10) >> 1;								// Logic value of encoder´s MSB
	bool enc2_val = display_num & 0b01;										// Logic value of encoder´s LSB

	gpioWrite(ENC1_PIN, enc1_val);
	gpioWrite(ENC2_PIN, enc2_val);

	// Show the corresponding character in the seven segment display
	bool show_cursor = (cursor == num_index) && (is_cursor_shown == true);
	write_display(show_cursor ? ' ' : virtual_display[num_index]);

	display_num = (display_num == WINDOW_SIZE-1) ? 0 : display_num+1;					// Get the next character to show
}

static void write_display(uint8_t character)
{
	// Get the index of the character to be showcased
	uint32_t index;
	if(character & IS_LETTER_MSK)
		index = (character & LETTER_MSK) + LETTER_OFFSET;
	else
		index = character & NUMBER_MSK;

	// If the index does not correspond to a character, a space is shown
	if(index >= CHARACTER_SIZE)
		index = CHARACTER_SIZE-1;

	// Word containing the segments that have to be turned off or on
	uint8_t char2segs = (character == ' ') ? char2disp[CHARACTER_SIZE-1] : char2disp[index];

	int i;
	for(i=0; i<8; ++i)
		gpioWrite(segment_pins[i], char2segs & (1 << i));

	if(brightness_timer != REFRESHING_TIME)
		startTimer(brightness_timer_id);
}

static uint32_t normalize_index(uint32_t index)
{
	while(index > VDISP_SIZE-1)
		index -= VDISP_SIZE;
	return index;
}

static void slider(void)
{
	if(slider_enable)
		window_offset = normalize_index(window_offset+1);
}

static void turn_off_segs(void)
{
	int i;
	for(i=0; i<8; ++i)
		gpioWrite(segment_pins[i], LOW);
}

static void show_cursor(void)
{
	is_cursor_shown = is_cursor_shown ? false : true;
}

/*******************************************************************************
 ******************************************************************************/
