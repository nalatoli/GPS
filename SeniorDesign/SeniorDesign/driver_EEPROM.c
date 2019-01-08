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
		
////////////////////////////////////////////////////////////////////////////////////////////////////
//								DEFINTIONS				           								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

	/* EEPROM REGISTER BITS */
	#define EEMWE				2
	#define EEWE				1
	
	/* EEPROM ADDRESS ENUMERATION */
	//Every element of this enumeration is used to reference the address of a byte in EEPROM.
	//Update the maximum count when you add contents.
	#define EEPROM_BYTES_COUNT	3
	enum{
		E_BYTE_SIGNATURE_0,			//First character used to sign SD cards from this MCU.
		E_BYTE_SIGNATURE_1,
		E_BYTE_SIGNATURE_2,
		E_FLAG_BUZZER,
		E_FLAG_TRACING,
		E_FLAG_WRITING_DISK,
		};
		
	/* SYSTEM SIGNATURE */
	const char SYS_signature[3] = { "&^K" };
	
////////////////////////////////////////////////////////////////////////////////////////////////////
//								FUNCTIONS            											  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* Prototypes for untested functions */
void EEPROM_set_flag(uint8_t truth, uint16_t flag_address);
void EEPROM_set_signature(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_enable (void){
	
	//Set master write enable bit, as well as read bit:
	EECR = (1 << EEMWE);
	
	//Set default addresses to zero:
	EEARH = 0;
	EEARL = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_recovery(void){
	
	//Use a register to read in each flag from EEPROM.
	//When a flag is loaded, decide what to do with it based on the value.
	uint8_t flag;
	
	/* FLAG EVALUATION */ 
		//FLAG_BUZZER:
		flag = EEPROM_read(E_FLAG_BUZZER);
		if (flag == 1){
		
			//N*N
			//Enable the buzzer
		}
	
		//FLAG TRACING:
		flag = EEPROM_read(E_FLAG_TRACING);
		if (flag == 1){
		
			//N*N
			//Clear out the entry in the SD space that was currently being written:
		
			//Update the manifest after purge of the last incomplete entry:
		}
		
		//FLAG_WRITING_DISK:
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_set_flag(uint8_t truth, uint16_t flag_address){
	
	if(truth != 0 ){truth = 1;}
	EEPROM_write(flag_address, truth);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void EEPROM_set_signature(void){
	
	EEPROM_write(E_BYTE_SIGNATURE_0, SYS_signature[0]);
	EEPROM_write(E_BYTE_SIGNATURE_1, SYS_signature[1]);
	EEPROM_write(E_BYTE_SIGNATURE_2, SYS_signature[2]);
}
