#ifndef HEADER_KEYPAD_H
#define HEADER_KEYPAD_H
////////////////////////////////////////////////////////////////////////////////////////////////////
//									          Keypad Header										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
//Screen option count:
#define OPTION_LENGTH_MAIN	3
#define OPTION_LENGTH_NAV	2
#define OPTION_LENGTH_TRACE	4

////////////////////////////////////////////////////////////////////////////////////////////////////
//											     Library										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/pgmspace.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//									        Type Definition										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
	Type Definition: Option (Data Structure)
	Description:
		Defines a 'task' to be performed and its corresponding option 'label'.
		Lists of options are externally available and are easily modifiable within the driver.
			
***************************************************************************************************/
typedef const struct{
	void(*task)();
	char label[30];
} Options;
extern const Options optionsMAIN[];
extern const Options optionsDEBUG[];
extern const Options optionsTRACE[];
extern int globalOption;

////////////////////////////////////////////////////////////////////////////////////////////////////
//										  Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
	Function: init
		- Initializes keypad by enabling and configuring external interrupts.
		- The keypad will be enabled by default
		
***************************************************************************************************/
void KEY_init();

/***************************************************************************************************
	Function: setState
		- Either enables (state = 1) or disables (state = 0) keys
		
***************************************************************************************************/
void KEY_setState(uint8_t state);

/***************************************************************************************************
	Function: execute
		- Calls a task based on the global state of the system
		! This function is handled by an interrupt
		
***************************************************************************************************/
void KEY_execute();

/***************************************************************************************************
	Function: scroll
		- Moves cursor on screen and updates global state of the system
		! This function is handled by an interrupt
		
***************************************************************************************************/
void KEY_scroll(int key);

////////////////////////////////////////////////////////////////////////////////////////////////////
//									        Public MACROS										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
//Keypad:
#define	KEY_UP -1
#define KEY_DOWN 1
#define	KEY_EXECUTE 2

// Pin/Port Assignments
#define BIT_UP			2		
#define BIT_DOWN		3
#define BIT_EXECUTE		2	
#define PIN_UP			PIND
#define PIN_DOWN		PIND
#define PIN_EXECUTE		PINB



#endif
	
