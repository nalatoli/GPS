////////////////////////////////////////////////////////////////////////////////////////////////////
//											  DISK Header										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef HEADER_DISK_H
#define HEADER_DISK_H
////////////////////////////////////////////////////////////////////////////////////////////////////
//											   Libraries										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#define F_CPU 8E6
#include <util/delay.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//									       Type Definitions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
	Type Definition: DiskType (Enumeration)
	Description:
		Organizes the various possible card types that can be encountered, including:
		
			NOINIT:     Initialization has NOT been called
			UNKNOWN:    Initialization has failed
			SDv2_BLOCK: 2nd generation Secure Digital Card with Block Addressing Format
			SDv2_BYTE:  2nd generation Secure Digital Card with Byte Addressing Format
			SDv1:       1rst generation Secure Digital Card
			MMv3:       3rd generation Multi Media Card
			
***************************************************************************************************/
typedef enum{
	NOINIT,
	UNKNOWN,
	SDv2_BLOCK,
	SDv2_BYTE,
	SDv1,
	MMv3
} DISKType;

/***************************************************************************************************
	Type Definition: DISKHandler (Data Structure) [Externally Available As 'disk']
	Description:
		Records key parameters of card activity, including:
		
			type:       card type found during initialization
			lastSector: last sector that was written to or read from
			buff:       SRAM stored buffer for transmitting to and from card
			buffIt:		current index of buffer based on loading data
		
***************************************************************************************************/
#define BUFFMAXBYTES 78
typedef struct{
	DISKType type;
	uint16_t lastSector;
	char buff[BUFFMAXBYTES];
	uint8_t buffIt;
	
} DISKHandler;
extern DISKHandler disk;

////////////////////////////////////////////////////////////////////////////////////////////////////
//										   Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
	Function: init
		- Used for initializing card. The MCU performs the following procedure:
		
			1. Initializes the SPI
			2. Puts the Card into Native Mode
			3. Puts the Card into SPI mode
			4. Performs Software Reset
			5. Performs Voltage Checks to Determine Card Type
		
***************************************************************************************************/
uint8_t DISK_init();

/***************************************************************************************************
	Function: loadBuff_char
		- Loads buffer with character 'data' at 'off'
		
***************************************************************************************************/
void DISK_loadBuff_char(char data, uint8_t off);

/***************************************************************************************************
	Function: loadBuff_str
		- Loads buffer with string 'data' at 'off'
		
***************************************************************************************************/
void DISK_loadBuff_str(char * data, uint8_t off);

/***************************************************************************************************
	Function: loadBuff_int
		- Loads buffer with integer 'data' at 'off'
		
***************************************************************************************************/
void DISK_loadBuff_int(int data, uint8_t off);

/***************************************************************************************************
	Function: write
		- Writes buffer into 'sector'.
		! sector <  16777216
		
***************************************************************************************************/
uint8_t DISK_write(uint32_t sector);

/***************************************************************************************************
	Function: read
		- Reads into buffer from 'sector'.
		! sector <  16777216
		
***************************************************************************************************/
uint8_t DISK_read(uint32_t sector);

/***************************************************************************************************
	Function: wipe
		- Fills 'count' blocks starting from 'sector' with NULL characters
		! sector <  16777216
		! count  < 16777216
		
***************************************************************************************************/
uint8_t DISK_wipe(uint32_t sector, uint32_t count);

uint16_t DISK_getBuffIt();

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Public MACROS										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Control Parameters */
extern DISKHandler disk;
#define DISK_CS						0
#define DISK_CD						1
#define DISK_EMPTYBYTE				0
#define DISK_CAPACITY				16777216 	
	
/* COMMANDS */
#define CMD0						(0)			//Software reset.
#define CMD1						(1)			//Initialize.
#define CMD8						(8)			//Check voltage range.
#define CMD9						(9)			//Read CSD (card specific data) register.
#define CMD10						(10)		//Read CID register.
#define CMD12						(12)		//Stop to read data.
#define CMD16						(16)		// Set block length
#define CMD17						(17)		// Read a block
#define CMD18						(18v		// Read multiple blocks
#define CMD24						(24)		// Write a block
#define CMD25						(25)		// Write multiple blocks
#define CMD55						(55)		// Leading command of ACMD<n> command
#define CMD58						(58)		// Read OCR	
#define ACMD23						(23+0x80)	// Set # of blocks to erase (SDC)
#define ACMD41					    (41+0x80)	// Starts initialization (SDC) 

#endif