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
//								DRIVER SYSTEM VARIABLES           								  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include "header_MACROS.h"
#include "header_FUNCTIONS.h"
#define F_CPU 16000000UL
#include "util/delay.h"

uint8_t	NV_USER_PREFERENCES_0,
		NV_USER_PREFERENCES_1,
		NV_SYSTEM_STATUS_0,
		NV_SYSTEM_STATUS_1;
////////////////////////////////////////////////////////////////////////////////////////////////////
//								ADDRESS SPACE CONTENTS           								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

	//Use this space to define addresses in the EEPROM space, that you have delegated
	//specific roles to.
	
	//For example:
	#define EEPROM_USER_PREF_0		0
	#define EEPROM_USER_PREF_1		1
	#define EEPROM_SYSTEM_STATUS_0	2
	#define EEPROM_SYSTEM_STATUS_1	3
	#define EEPROM_SD_NEXT_SERIAL	4		//Used to format unique SD cards.
	
	
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
	
	//Clear out erroneous data from powerdown:

	//Fetch all of the data stored in the EEPROM space, and return it to its space in memory:
	NV_SYSTEM_STATUS_0 = EEPROM_read(EEPROM_SYSTEM_STATUS_0);
	NV_SYSTEM_STATUS_1 = EEPROM_read(EEPROM_SYSTEM_STATUS_1);
	NV_USER_PREFERENCES_0 = EEPROM_read(EEPROM_USER_PREF_0);
	NV_USER_PREFERENCES_1 = EEPROM_read(EEPROM_USER_PREF_1);
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
