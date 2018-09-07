////////////////////////////////////////////////////////////////////////////////////////////////////
//								NON-VOLATILE SD DATA HANDLING DRIVER							  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This module is used to generate all of the system functions that handle the dataflow from the
	master microprocessor, to the SD card, and vice-versa. This module also helps to structure the
	partitions of data 
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#define F_CPU 16000000UL
#include "util/delay.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//								DEFINITIONS														  //
////////////////////////////////////////////////////////////////////////////////////////////////////

	//This is used to determine the pin that is used to assert the slave:
	#define SD_SLAVE_PIN				7

	//These are the addresses of the SD card that we have delegated tasks to:
	#define SD_DATA_ID_H				0			//Used to serialize each SD card.
	#define SD_DATA_ID_L				1
	#define SD_DATA_LAST_ENTRY_H		2			//The last location that some entry was made
	#define SD_DATA_LAST_ENTRY_L		3			//necessary for the recovery purging function.
	#define SD_DATA_TOTAL_ENTRIES_H		4			//Needed to track total entries for error
	#define SD_DATA_TOTAL_ENTRIES_L		5			//correction. Also generally useful.
	#define SD_DATA_USAGE_3				6			//Space is used to track the total number
	#define SD_DATA_USAGE_2				7			//of bytes consumed by the SD card at the
	#define SD_DATA_USAGE_1				8			//present moment.
	#define SD_DATA_USAGE_0				9
	
	/* COMMANDS */
	#define CMD0						0x40		//Software reset.
	#define CMD1						0x41		//Initialize.
	#define CMD8						0x48		//Check voltage range.
	#define CMD9						0x49		//Read CSD (card specific data) register.
	#define CMD10						0x4A		//Read CID register.
	#define CMD12						0x4C		//Stop to read data.
	
	/* BLOCK LENGTHS */
	//These numbers all correspond to the length of the data block associated with the entry:
	#define BLOCK_begin_entry			0
	#define BLOCK_origin				0
	#define BLOCK_node					0
	#define BLOCK_supernode				0
	#define BLOCK_terminator			0	
	
////////////////////////////////////////////////////////////////////////////////////////////////////
//								SYSTEM VARIABLES												  //
////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t Card_Type;		//Value of this will either be ('*') or ('&') depending on the generation.
						//('*') = Gen1	('&') = Another generation
////////////////////////////////////////////////////////////////////////////////////////////////////
//								FUNCTION PROTOTYPES												  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void SD_init_SPI(void);
void SD_init_card(void);
void SD_assert(void);
void SD_unassert(void);
void SD_SPI_send_byte(char byte);
void SD_SPI_send_command(uint8_t command, uint32_t arguments, uint8_t CRC);
void SD_SPI_set_block_length(uint16_t bytes);
void SD_SPI_save_coordinate(char type);
void SD_format_card(void);
//function for reading
//function for writing
//functions for sending commands to the card
//functions for wiping all of the data from the card
//functions for locating the last address, then purging the last entry
//function that pads entire data-space with the "\" character, which is useful for detecting
		//corrupted data.
////////////////////////////////////////////////////////////////////////////////////////////////////
//								FUNCTION IMPLEMENTATION											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void SD_assert(void){
	
	//Whatever the pin is that is associated with the SD card's chip select, drive active:
	PORTA &= !(1 << SD_SLAVE_PIN);		//Drive the bit low! That is active.
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SD_unassert(void){
	
	//Drive the CS bit high. OR with the bit:
	PORTA |= (1 << SD_SLAVE_PIN);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SD_init_SPI(void){
	
	//Byte used for cleaning up the SPI system:
	char clear_byte;
	
	//Set up the SPI register for parameters congruent with the Micro SD:
			/* PARAMETERS
			-Mode:					(0,0)
			-Frequency divider:		64
			*/
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(1<<SPR1)|(0<<SPR0);
	
	//Read in the status register to clear it:
	clear_byte = SPSR;
	
	//Write a byte to the data register to prepare it:
	SPDR = clear_byte;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SD_SPI_send_byte(char byte){
	
	//DO NOT assert the device. 
	//Simply write the byte and wait until transmission has concluded:
	SPDR = byte;
	//Wait for transmission termination:
	while(!(SPSR & (1 << SPIF)));
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SD_SPI_send_command(uint8_t command, uint32_t arguments, uint8_t CRC){
	
	//Send command:
	SD_SPI_send_byte(command);
	//Send arguments MSB first:
	SD_SPI_send_byte((char)(arguments >> 24));
	SD_SPI_send_byte((char)(arguments >> 16));
	SD_SPI_send_byte((char)(arguments >> 8));
	SD_SPI_send_byte((char)(arguments));
	//Send the CRC:
	SD_SPI_send_byte(CRC);
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SD_init_card(void){
	
	//Assert the device and hold:
	PORTA &= !(1 << SD_SLAVE_PIN);
	
	//Send ten dummy bytes:
	for(int bytes = 0; bytes < 10; bytes++){

		//Dummy bytes for SD are always 0xFF:
		SPDR = 0xFF;
		//Wait for transmission:
		while(!(SPSR & (1 << SPIF)));
	}
	
	/* INITIALIZATION PROCESS */
	//1. COMMAND 1: CMD0
		SD_SPI_send_byte(0x40);
		for(char i = 0; i < 4; i++){
			SD_SPI_send_byte(0x00);
		}
		//CRC byte:
		SD_SPI_send_byte(0x95);
		//Check receiver constantly for the IDLE response:
		char helper;
		while(1){
			//Read data register
			helper = SPDR;
			if(helper == 0x01){break;}
			//Generate another 8 pulses:
			SD_SPI_send_byte(0xFF);
		}
	//2. COMMAND 2: CMD8
		//This is the voltage range command.
		//We are using it to determine what generation card ours is.
		SD_SPI_send_command(0x48, 0x000001AA, 0x87);
		//Wait for either GEN1 (0x05) or GEN2 (0x01)
		while(1){
			helper = SPDR;
			if(helper == 0x05){
				//GEN1
				Card_Type = '*';
				break;
			}
			else if(helper == 0x01){
				//GEN2
				Card_Type = '&';
				break;
			}
			SD_SPI_send_byte(0x00);
		}
		//Send a few dummy bytes to terminate the command:
		for(char i = 0; i < 5; i++){
			SD_SPI_send_byte(0xFF);
		}
	//3. COMMAND 3: CMD1
		//This command begins initialization, and has no arguments:
		SD_SPI_send_command(0x61, 0x00000000, 0x00);
		//Wait for "0x00":
		while(1){
			helper = SPDR;
			if(helper == 0x00){
				break;
			}
			SD_SPI_send_byte(0x00);
		}
	
	//This is just to use a breakpoint, please get rid of it after testing:
	helper = '*';
	
	//END OF INITIALIZATION
}
////////////////////////////////////////////////////////////////////////////////////////////////////