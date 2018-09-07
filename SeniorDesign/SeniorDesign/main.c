////////////////////////////////////////////////////////////////////////////////////////////////////
//								MAIN   PROJECT   BUILD											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This section of the program acts as the main hub for system functionality. This module call on
	several of the other header, and driver files that are critical to control all of the system
	peripherals/subsystems.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
//								VERSION NOTES													  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	-Current build is aimed at preliminary testing.
	-Testing must be done to verify:
		-GPS satellite acquisition and 1 Hz updates.
		-Function of GPS passive parsing.
		-Writing of GPS data to the display.
		-Writing to EEPROM
		-Reading from EEPROM
		-Recovering session data from EEPROM
	-Certain interrupt driven functions may collide with one another.
	-The SPI transmissions may interfere with the USART data reception.
	
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <avr/interrupt.h>
#include "header_MACROS.h"
#include "header_FUNCTIONS.h"
#define F_CPU 16000000UL
#include "util/delay.h"
#include "header_LCD.h"
#include "header_BUZZER.h"
#include <string.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
//								SYSTEM VARIABLES/FUNCTIONS    									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* FROM SD */
extern void SD_init_SPI(void);
extern void SD_init_card(void);
extern void SD_assert(void);
extern void SD_unassert(void);
extern void SD_SPI_send_byte(uint8_t byte);
extern void SD_SPI_send_command(uint8_t command, uint32_t arguments, uint8_t CRC);
extern void SD_SPI_set_block_length(uint16_t bytes);
extern void SD_SPI_save_coordinate(char type);
extern void SD_format_card(void);

/* FROM GPS DRIVER */
extern unsigned char GPS_BUFFER[125];
extern unsigned char GPS_BUFFER_INDEX;
extern unsigned char GPS_MESSAGE_READY;
extern void GPS_init_USART(uint16_t UBRR);
extern void GPS_enable_stream(void);
extern void GPS_disable_stream(void);
extern void GPS_send_byte(unsigned char data);
extern char GPS_receive_byte(void);
extern void GPS_flush_buffer(void);
extern void GPS_parse_data(void);
extern void GPS_configure_firmware(void);

/* FROM EEPROM */
//Persistent (EEPROM-buffered) data:
extern uint8_t	NV_USER_PREFERENCES_0,
				NV_USER_PREFERENCES_1,
				NV_SYSTEM_STATUS_0,
				NV_SYSTEM_STATUS_1;	
extern void EEPROM_enable(void);
extern void EEPROM_write(uint16_t address, uint8_t data);
extern uint8_t EEPROM_read(uint16_t address);
extern void EEPROM_recovery(void);
				
/* FROM BUZZER */
extern void init_buzzer(void);
extern void delay_ms(uint16_t duration);
extern void tone(uint16_t frequency, uint16_t duration);
extern void tone_START(uint16_t frequency);
extern void tone_END (void);
extern void play(Note *song, uint8_t song_len, uint8_t song_cut);

/* FROM LCD */

//NOTE TO SELF: Migrate all of these extern references (^) to their own header file eventually. 
////////////////////////////////////////////////////////////////////////////////////////////////////
//								INTERRUPTS    											          //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* KEYPAD INTERRUPT FOR FSM */

/* GPS INTERRUPT FOR PARSING */
ISR(USARTRXC_vect){
	
	//Receive the byte to empty out the buffer, then operate on it:
	char vector_data = GPS_receive_byte();
	
	//Always start with an overflow check:
	// -Since the sentence length won't exceed 120, we should worry if it gets to 121.
	// -Should this happen, a hard reset is necessary, so we'll do exactly that.
	if(GPS_BUFFER_INDEX > 120){GPS_flush_buffer();}
	
	//Filter out whether or not this data can be transferred to the buffer:
	//	-This requires either that the received character is '$' or GPS_MESSAGE_READY = 1;
	if(vector_data == '$'){
		GPS_MESSAGE_READY = 1;
		//These assignments help with buffering alignment.
		GPS_BUFFER[0] = '$';
		GPS_BUFFER_INDEX = 1;
	}
	//If the character received is instead a terminator, begin the parsing!
	else if(vector_data == '*'){
		GPS_parse_data();
	}
	//If neither character was detected, but '$' appeared previously, then just buffer the characters:
	else if(GPS_MESSAGE_READY){
		GPS_BUFFER[GPS_BUFFER_INDEX] = vector_data;
		GPS_BUFFER_INDEX++;
	}
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//								MAIN PROGRAM   											          //
////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void)
{		
	///////////////
	//   SETUP   //
	///////////////
	
	/* PORTS */
	DDRA = 0xFF;	//Port A is not currently used, but may assert slaves in the future.
	PORTA = 0x00;	
	
	DDRB = 0xCD;	//Port B contains:
	PORTB = 0x32;	// SPI, INT2 ( Pin 3 ), connection to LCD ( data out ).
	
	DDRC = 0x00;	//Port C contains:
	PORTC = 0x00;	// JTAG interface; this renders some of the pins unusable as I/O.
	
	DDRD = 0x4F;	//Port D contains:
	PORTD = 0x80;	// USART, INT0, INT1, buzzer driver pin
	
	/* RECOVERY */
	//Recovery program is currently defined in the EEPROM driver.
	EEPROM_enable();
	//This recovery program must have two components!
	//EEPROM_recovery(); AND SD_recovery(); which is not yet available!
	EEPROM_recovery();				//Reacquire old persistent information.
	//SD Card must be initialized here as well, and erroneous data must be purged:
	SD_assert();
	//Initialize the SD card: {Be sure to look into the function when single-stepping}
	SD_init_card();
	//Find the last entry in memory, then check it byte-wise for both the beginning of
	//an entry ('%') as well as a few other entry-confirming characters.
	
	
	
	/* INTERRUPTS */
	//Must enable global interrupts to use GPS hardware.
	
	/* GPS */
	//Initialize communication, then configure MTK3339 firmware, then open up.
	GPS_BUFFER_INDEX = 0;			//Begin at start of data buffer.
	GPS_MESSAGE_READY= 0;			//Buffer will begin to fill normally.
	GPS_init_USART(9600);
	GPS_configure_firmware();
	//Consider actually enabling the stream.
	GPS_disable_stream();

	
	/* TIMERS/BUZZER */
	//Initialize the buzzer:
	init_buzzer();
	
	/* DISPLAY */
	
	
		
	//Dormant program loop for testing.
    while (1) 
    {
		_delay_us(0.5);
		PORTB &= ~(1<<0);
		
    }
}
