#ifndef HEADER_FUNCTIONS_H
#define HEADER_FUNCTIONS_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#define F_CPU 16000000UL
#include "util/delay.h"
#include "header_BUZZER.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//								SYSTEM VARIABLES/FUNCTIONS    									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#define BAUD 9600
#define FOSC 16000000
#define MY_UBBR FOSC/16/BAUD-1

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
extern void GPS_send_byte(char data);
extern char GPS_receive_byte(void);
extern void GPS_flush_buffer(void);
extern void GPS_parse_data(void);
extern void GPS_configure_firmware(void);
extern void GPS_USART_Transmit(unsigned char data);
//Variables:
/* GPS CURRENT READINGS DATA STRUCTURE */
typedef struct{
	//Temporal data
	uint8_t UTC_H;
	uint8_t UTC_M;			//Technically, you can afford greater precision but we don't
	uint8_t UTC_S;			//care in this application.			
			
	//Date, DD/MM/YY
	uint8_t UTC_DAY;			//The current day of the month.
	uint8_t UTC_MONTH;			//The current month.
	uint8_t UTC_YEAR;			//The current year.
		
	//Data validity status byte (A = VALID, V = INVALID)
	uint8_t STATUS;
									
	//Latitude and longitude in degrees
	uint16_t latitude_H;
	uint16_t latitude_L;	//Below the decimal point
	uint8_t NS;				//Indicates your placement relative to the equator.
	uint32_t longitude_H;
	uint16_t longitude_L;	//Below the decimal point
	uint8_t EW;				//Indicates you placement relative to the prime meridian.
	
	//Ground speed
	uint8_t ground_speed_high;
	uint8_t ground_speed_low;
	
	//Course over ground (azimuth angle from GPS North) {DECIMATED}
	uint16_t course_high;	//Indicates the 360 degrees above the decimal point.
	uint8_t  course_low;		//Indicates the precision value below the decimal point.
	
	//***Some parameters have been excluded like magnetic variation to save processor speed***

	}GPS_data;
	
extern GPS_data SYS_GPS;	

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
#endif