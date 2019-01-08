#include "header_DISK.h"
#include "header_LCD.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//									   Disk Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void DISK_init_spi();
uint8_t DISK_spi_transmit(uint8_t data);
uint8_t DISK_wait4ready(uint16_t count);
uint8_t DISK_assert();
void DISK_unassert();
uint8_t DISK_send_command(uint8_t cmd, uint32_t args);
uint8_t DISK_send_packet(uint8_t token);
uint8_t DISK_recieve_packet();

////////////////////////////////////////////////////////////////////////////////////////////////////
//										Disk Driver Objects									      //
////////////////////////////////////////////////////////////////////////////////////////////////////
DISKHandler disk;
		
////////////////////////////////////////////////////////////////////////////////////////////////////
//										Disk Public Functions								      //
////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t DISK_init()
{
	/* Perform Driver Initialization */	// ***
	uint8_t OCR[4];						// Declare OCR storage 
	_delay_ms(2);						// Wait 2 ms					
	DISK_init_spi();					// Initialize the SPI
	disk.type = NOINIT;					// Initialize card type
	
	/* Put Card into Native Mode (Send 20 Dummy Bytes) */
	for(int i = 0; i < 10; i++) DISK_spi_transmit(0xFF);
	
	/* Put Card into SPI Mode (Soft Reset) */	// ***
	if(DISK_send_command(CMD0,0) != 0x01){		// If soft reset response is NOT 0x01 (idle)
		DISK_unassert();						//  Unassert card			
		SPCR0 &= ~(1<<SPR10);					//  Increase SPI speed
		disk.type = NOINIT;						//  Mark card as uninitialized
		return 1;								//  Return (from failure)	
	}
	
	/* Perform SDv2 Voltage Check (res = 0x01 (idle): SUCCESS) */
	if(DISK_send_command(CMD8,0x01AA) == 0x01){			
		
		/* Record OCR (4-bytes After Status Response) */
		for(int i = 0; i < 4; i++)	OCR[i] = DISK_spi_transmit(0xFF);
		
		/* If OCR matches Voltage Check Argument (0x01AA) */
		if(OCR[2] == 0x01 && OCR[3] == 0xAA){		
			
			/* Perform SDv2 Card Initialization / Wait Until Card is NOT idle */
			while(DISK_send_command(ACMD41, 1UL << 30)) ;	
			
			/* Perform OCR Extraction (res = 0x00 (awake): SUCCESS */	
			if(DISK_send_command(CMD58,0) == 0){
				
				/* Record OCR (4-bytes After Status Response) */
				for(int i = 0; i < 4; i++) OCR[i] = DISK_spi_transmit(0xFF);
				
				/* Record Card Type based on OCR[0] (CCS bit) */
				disk.type = (OCR[0] & 0x40) ? SDv2_BLOCK : SDv2_BYTE;
			}
		}
	}
	
	/* If SDv2 Voltage Check Response Yielded Errors */
	else{

		/* Check if Card is SDv1 / Initialize Suitably */	// ***
		if(DISK_send_command(ACMD41,0) <= 0x01){			// If SDC initialization response indicated NO errors						
			disk.type = SDv1;								//	Mark card type as 'SDv1'
			while(DISK_send_command(ACMD41,0) == 0x01) ;	//  Send SDC initialization command until SDC wakes up (res != 0x01) 
		}
		
		/* Else, Card Must Be MMv3 / Initialize MMC */	// ***
		else{
			disk.type = MMv3;							// Mark card type as 'MMv3'
			while(DISK_send_command(CMD1,0) == 0x01) ;	// Send MMC initialization command until MMC wakes up (res != 0x01)
		}
		
		/* Set Block Length to 512 (FAT Compatible) */	// ***
		if(DISK_send_command(CMD16,512) != 0x00)		// If response is NOT 0x00 (awake)
			disk.type = UNKNOWN;						//  Indicate error by marking card type as 'UNKNOWN'
	}
	
	/* Return From Success */	// ***
	DISK_unassert();			// Unassert card
	SPCR0 &= ~(1<<SPR10);		// Increase SPI speed
	return 0;					// Return
}

void DISK_loadBuff_char(char data, uint8_t off)
{
	/* Load Buffer with Character: 'data' */
	disk.buff[off] = data;
	disk.buff[off+1] = 0;
	disk.buffIt = off + 2;
}

void DISK_loadBuff_str(char * data, uint8_t off)
{
	strcpy(disk.buff + off, data);
	disk.buffIt = off + strlen(data) + 1;
}

void DISK_loadBuff_int(int data, uint8_t off)
{
	itoa(data, disk.buff + off, 10);
	disk.buffIt = off + strlen(disk.buff + off) + 1;
}

uint8_t DISK_write(uint32_t sector)
{	
	/* Convert Sector To Byte Address For Non-Block Card Types */
	if(disk.type != SDv2_BLOCK) sector *= 512;
	
	/* Perform Single-Block Write */				// ***
	if(DISK_send_command(CMD24,sector))	return 1; 	// Send 'Write Block' command			
	if(DISK_send_packet(0xFE)) return 1;			// Send packet
	
	/* Return From Success */	// ***	
	DISK_unassert();			// Unassert card to release SPI buses
	DISK_spi_transmit(0xFF);	// Send dummy byte (initiate card's internal write process)
	return 0;					// Return success
}

uint8_t DISK_read(uint32_t sector)
{	
	/* Convert Sector and Offset to Byte Address */
	if(disk.type != SDv2_BLOCK) sector *= 512;
			
	/* Perform Single-Block Read */					// ***
	if(DISK_send_command(CMD17,sector)) return 1; 	//  Send 'Read Block' command
	if(DISK_recieve_packet())		    return 1;	//  Receive packet
		
	/* Return From Success */	// ***
	DISK_unassert();			// Unassert card to release SPI buses
	return 0;					// Return success
}

uint8_t DISK_wipe(uint32_t sector, uint32_t count)
{
	/* Declare Fail Tracker */
	uint8_t fail = 0;
	
	/* Convert Sector To Byte Address For Non-Block Card Types */
	if(disk.type != SDv2_BLOCK) sector *= 512;
	
	/* Send Pre-Erase Command For SDCs */
	if(disk.type == SDv1 || disk.type == SDv2_BLOCK || disk.type == SDv2_BYTE)
		if(DISK_send_command(ACMD23,count)) return 1;
	
	/* Send Multi-Block Write Command */
	if(DISK_send_command(CMD25,sector)) return 1;

	/* Fill Buffer With Empty Bytes */
	for(int i = 0; i < BUFFMAXBYTES; i++) disk.buff[i] = 0;
	
	do {
		/* Send 'count' Packets of Empty Data */
		disk.buffIt = BUFFMAXBYTES;
		if(DISK_send_packet(0xFC)) { fail = 1; break; }
	} while(--count);
	
	/* Send STOP Token After Final Ready Up */
	DISK_send_packet(0xFD);
	
	/* Return From Success */	// ***
	DISK_unassert();			// Unassert card to release SPI buses
	DISK_spi_transmit(0xFF);	// Send dummy byte (initiate card's internal write process)
	_delay_ms(5);
	return fail;				// Return result
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   Disk Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

void DISK_init_spi()
{
	/* Set SPI Speed and Settings */			// ***
	DDRB |= (1<<0)|(1<<4)|(1<<5)|(1<<7);		// Set CS(0), SS(4), MOSI(5) and SCK(7) as outputs
	DDRB &= ~((1<<1)|(1<<6));					// Set CD(1) and MISO(6) as inputs
	PORTB |= (1<<1)|(1<<6);							// Enable MISO internal pull up
	PORTB |= (1<<DISK_CS);						// Disable SD Chip Select
	SPCR0 |= (1<<SPE0)|(1<<MSTR0)|(1<<SPR10);	// Enable SPI - fclk/64
	SPSR0 |= (1<<SPI2X0);						// Enable 2x SPI speed					
}

uint8_t DISK_spi_transmit(uint8_t data)
{
	/* Transmit SPI data */			// ***
	SPDR0 = data;					// Send 'data' to card
	while(!(SPSR0 & (1<<SPIF0))) ;	// Wait till the transmission is finished
	return SPDR0;					// Return received data from card
}

uint8_t DISK_wait4ready(uint16_t count)
{
	/* Wait For Response From Card */
	uint8_t res;						// Declare response indicator
	
	do                                  // Do the following:
		res = DISK_spi_transmit(0xFF);	//  Receive response from card
	while(res != 0xFF && --count);		//  While card response is invalid or NOT timed out
	
	return (res == 0xFF) ? 0 : 1;		// Return {0:Ready, 1:Timeout}
}

uint8_t DISK_assert()
{
	/* Assert Card For Command */
	PORTB &= ~(1<<DISK_CS);
	DISK_spi_transmit(0xFF);	// Send dummy clock (force card DO enabled)
	
	if(DISK_wait4ready(50)){	// Wait 50 dummy clocks for card to ready up
		DISK_unassert();		// Unassert card
		return 1;				//  Return if card ready up has timed out
	}
	
	return 0;					// Return success if card is ready
}

void DISK_unassert()
{
	/* Unassert Card */
	PORTB |= (1<<DISK_CS);		// Deselect card
	DISK_spi_transmit(0xFF);	// Send dummy clock (force card DO hi-z for multiple slaves)
}

uint8_t DISK_send_command(uint8_t cmd, uint32_t args)
{
	/* Delare Control Parameters */	// ***
	uint8_t res;					// Storage for received data
	uint8_t count = 50;				// Timeout count			
	
	/* Send Special Command for ACMD<n> Type */	// ***
	if(cmd & 0x80){								// If cmd[7] = 1 (cmd is ACMD<n> type),
		cmd &= 0x7F;							//  Clear cmd[7] (cmd is now CMD<n> type)
		res = DISK_send_command(CMD55,0);		//  Send ACMD<n> preliminary command
		if(res > 1) return res;					//  Return response if error flag indicated
	}
	
	/* Assert Card */					// ***
	if(cmd != CMD12){					// If cmd is NOT multi-block read cmd
		DISK_unassert();				//  Unassert card
		if(DISK_assert()) return 0xFF;	//  Assert card (return for no response)
	}
	
	/* Send 01 + CMD + ARGS */				// ***
	DISK_spi_transmit(0x40|cmd);			// Send 01 + cmd
	DISK_spi_transmit((uint8_t)(args>>24));	// Send MS args byte
	DISK_spi_transmit((uint8_t)(args>>16));	// ...
	DISK_spi_transmit((uint8_t)(args>>8));	// ...
	DISK_spi_transmit((uint8_t)(args));		// Send LS args byte
	
	/* Send CRC Field + STOP */						// ***
	if(cmd == CMD0)	     DISK_spi_transmit(0x95);	// Send valid CRC + STOP for software reset
	else if(cmd == CMD8) DISK_spi_transmit(0x87);	// Send valid CRC + STOP for SDv2 voltage check
	else                 DISK_spi_transmit(0x01);	// Else, send dummy CRC + STOP
	
	/* Return Response */
	if(cmd == CMD12) DISK_spi_transmit(0xFF);		// For multi-block read, skip stuff byte

	do {										// Do the following:
		res = DISK_spi_transmit(0xFF);			//  Receive response from card
	} while ((0x80 & res && --count));			//  While card is busy and NOT timed out
	
	return res;									// Return response
}

uint8_t DISK_send_packet(uint8_t token)
{	
	/* Return Failure If Card is Busy */
	if(DISK_wait4ready(50000)) return 1;
	
	/* Send Token */			// ***
	DISK_spi_transmit(token);	// Send token
	
	/* Return Success if STOP Token */
	if(token == 0xFD) return 0;
	
	/* Send Data */													// ***
	uint16_t i = 0;													// Initialize iterator
	for(; i < disk.buffIt; i++){									// For all valid bytes,
		DISK_spi_transmit(*(disk.buff+i));							//  Send byte
		*(disk.buff+i) = 0;											//  Clear byte in buffer
	}
	
	for(; i < 512; i++)			DISK_spi_transmit(DISK_EMPTYBYTE);	// Place empty bytes into rest of block
	
	/* Send Dummy CRC Code */
	DISK_spi_transmit(0xFF);
	DISK_spi_transmit(0xFF);
	
	/* Return From Failure If Data Response is NOT 0x05 (SUCCESS) */
	if((DISK_spi_transmit(0xFF) & 0x1F) != 0x05) return 1;
		
	/* Return From Success */
	disk.buffIt = 0;
	return 0;
}

uint8_t DISK_recieve_packet()
{
	/* Wait For Card To Ready Up */			// ***
	uint8_t token;							// Declare token storage
	
	do{										// Do the following:
		token = DISK_spi_transmit(0xFF);	//  Receive data from card
	} while(token == 0xFF);					//  While data is NOT a token
		
	if(token != 0xFE) return 1;				//  Return (from failure) if token is NOT SUCCESS
		
	/* Extract Data */															// ***
	uint16_t i = 0, count = BUFFMAXBYTES;										// Initialize iterator and valid data counter
	for(; i < BUFFMAXBYTES; i++) *(disk.buff + i) = DISK_spi_transmit(0xFF);	// Place 'len' bytes into card
	for(; i < 512; i++)								DISK_spi_transmit(0xFF);	// Skip rest of bytes
	while(!(*(disk.buff + --count)) && count) ;		// Place counter to last valid byte
	
	/* Discard CRC Code */
	DISK_spi_transmit(0xFF);
	DISK_spi_transmit(0xFF);
	
	/* Set Buffer Iterator to Pending String Offset and Return From Success */
	disk.buffIt = count == 0 ? count : count + 2;
	return 0;	
}