////////////////////////////////////////////////////////////////////////////////////////////////////
//										I2C DRIVER												  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	DESCRIPTION: Streamlines TWI/I2C serial communication by providing a set of basic functions
 *				
 *	AUTHOR: Nor
 *	
 *	FUNCTION SET:
 *		void i2c_send (uint8_t i2c_SLA_ADDR, uint8_t * i2c_DATA_BYTES, uint8_t i2c_DATA_LEN)
 *			- Initiates MT (Master Transmitter) mode on AVR device and sends 'i2c_DATA_LEN'
 *				number of 'i2c_DATA_BYTES' to SR (Slave Receiver) at address 'i2c_SLA_ADDR'
 *	
 *		uint8_t i2c_receive (uint8_t i2c_SLA_ADDR, uint8_t i2c_DATA_LEN)
 *			- Initiates MR (Master Receiver) mode on AVR device and returns a
 *				a pointer to 'i2c_DATA_LEN' number of DATA_BYTEs from
 *				ST (Slave Transmitter) at address 'i2c_SLA_ADDR'
 */
////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Libraries/Macros									      //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <stdlib.h> 
#include "header_I2C.h"
#define TWI_FREQ    1000000
#define F_CPU		16E6
////////////////////////////////////////////////////////////////////////////////////////////////////
//											Functions											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void i2c_init () 
{
	/* Initialize I2C Components */
	TWBR = (F_CPU / TWI_FREQ - 16) / 8;	// Set bit rate
	TWSR = (1<<TWPS0);					// Set prescaler to four
	TWCR = (1<<TWEN);					// Activate the TWI interface
	PORTC |= (1<<1);					// Enable pull up on SDA pin
	PORTC |= (1<<0);					// Enable pull up on SCL pin 
}

uint8_t i2c_poll (uint8_t START_STOP)
{
	/* Poll I2C bus for data */
	TWCR = START_STOP | (1<<TWINT) | (1<<TWEN);	// Initiate a TWI transmission
	if (START_STOP == (1<<TWSTO)) return 0;     // Don't poll if the STOP condition was sent
	while (!(TWCR & (1<<TWINT))) ;					// Wait for the interrupt flag to be set
	return (TWSR & 0xf8);							// Return the status code	
}

uint8_t i2c_send (uint8_t SLA, uint8_t * packets, uint8_t packet_num)
{
	/* Initialize transmission */
	uint8_t status = 0;										// Indicate transmission is incomplete
	uint8_t tmp_poll = i2c_poll(1<<TWSTA);					// Send START condition
	if (tmp_poll != START && tmp_poll != RSTART) return 0;	// If problem, return 0
	TWDR = SLA;											// Load TWDR with SLA + W
	if (i2c_poll(0) != MT_SLA_ACK) goto terminate;			// Send SLA + W. If problem, terminate transmission
	
	 
	
	/* Send every packet */
	for(uint8_t i = 0; i < packet_num; i++) {
		TWDR = packets[i];							// Load TWDR with current packet */		
		if (i2c_poll(0) != MT_DATA_ACK) goto terminate; // Send packet. If problem, terminate transmission
	}
	
	/* Indicate transmission is complete */
	status = 1;
	
	terminate:	/* Terminate Transmission */
	i2c_poll(1<<TWSTO);	// Send STOP condition
	return status;
	
}