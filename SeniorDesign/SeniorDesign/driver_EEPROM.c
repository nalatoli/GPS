////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER EEPROM        											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This module is used to save system information without volatility. This requires transferring
	some of the system variables to the device EEPROM space, which has a 512 byte space.
	9 bits are used in the [EEARH:EEARL] register pair to access bytes within the space.
	Some functions from this module are automatically loaded upon system restart.	
 */ 
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include "header_MACROS.h"
#include "header_FUNCTIONS.h"
#define F_CPU 16000000UL
#include "util/delay.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//								ADDRESS SPACE CONTENTS           								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

	/* PREFERENCES */
	uint8_t SYS_PREFERENCES_0 = 0;
	#define PREFERENCES_0 0
		//UI color {7-4}, Buzzer enabled {3}, Buzz during trace {2}, 
	#define PREFERENCES_1 1
	/* NAVIGATION DATA */
	#define NAV_START 2
		//Temporal data registers
		#define NAV_UTC_H 2
		#define NAV_UTC_M 3
		#define NAV_UTC_S 4

	
	/* SYSTEM FLAGS */
	
////////////////////////////////////////////////////////////////////////////////////////////////////
//								FUNCTIONS            											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_enable (void){
	
	//Set master write enable bit, as well as read bit:
	EECR = (1 << EEMWE);
	
	//Set default addresses to zero:
	EEARH = 0;
	EEARL = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_write(uint16_t address, uint8_t data){
	
	//Wait for previous write-enable strobe to null out:
	while(EECR & (1 << EEWE));
	
	//Load the address:
	EEARH = (address >> 8);
	EEARL = (address) & 0xFF;
	
	//Load the data:
	EEDR = data;
	
	//Enable writes to EEPROM:
	EECR |= (1 << EEMWE);
	
	//Trigger the write-enable strobe:
	EECR |= (1 << EEWE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t EEPROM_read(uint16_t address){
	
	//Wait for previous write-enable strobe to null out:
	while(EECR & (1 << EEWE));
	
	//Load the address:
	EEARH = (address >> 8);
	EEARL = (address) & 0xFF;
	
	//Read in the data by strobing:
	EECR |= (1 << EERE);
	
	//Return the contents of the data register:
	return EEDR;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_recovery(void){
	
	//Enable the EEPROM:
	EEPROM_enable();
	
	//Load in the runtime EEPROM system dashboard:
	SYS_PREFERENCES_0 = EEPROM_read(PREFERENCES_0);
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_system_autosave(void){
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////