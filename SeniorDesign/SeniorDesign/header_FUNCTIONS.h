#ifndef HEADER_FUNCTIONS_H
#define HEADER_FUNCTIONS_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#define F_CPU 8E6
#include "util/delay.h"
#include "header_SFX.h"
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
//								SYSTEM DEFINITIONS		    									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* FROM GPS */
//Max Length of NMEA Sentence
#define GPS_BUFFER_MAX 100 

//Lengths of ASCII Parameters:			SIGN	LEN		TERMINATOR
#define GPS_BYTES_ASCII_UTC_TIME		(0 +	8 +		1)
#define GPS_BYTES_ASCII_UTC_DATE		(0 +	8 +		1)
#define GPS_BYTES_ASCII_LATITUDE		(1 +	8 +		1)
#define GPS_BYTES_ASCII_LONGITUDE		(1 +	9 +		1)
#define GPS_BYTES_ASCII_COURSE			(0 +	3 +		1)
#define GPS_BYTES_ASCII_SPEED			(0 +	1 +		1)

#define GPS_BYTES_ASCII_TOTAL			(GPS_BYTES_ASCII_UTC_TIME + GPS_BYTES_ASCII_UTC_DATE + GPS_BYTES_ASCII_LATITUDE + GPS_BYTES_ASCII_LONGITUDE + GPS_BYTES_ASCII_COURSE + GPS_BYTES_ASCII_SPEED + 4)

////////////////////////////////////////////////////////////////////////////////////////////////////
//								SYSTEM VARIABLES/FUNCTIONS    									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#define BAUD 9600
#define FOSC 8000000
#define MY_UBBR FOSC/16/BAUD-1

/* FROM SD */
void SD_init_SPI(void);
void SD_init_card(void);
void SD_assert(void);
void SD_unassert(void);
void SD_SPI_send_byte(uint8_t byte);
void SD_SPI_send_command(uint8_t command, uint32_t arguments, uint8_t CRC);
void SD_SPI_set_block_length(uint16_t bytes);
void SD_SPI_save_coordinate(char type);
void SD_format_card(void);

/* FROM GPS DRIVER */
unsigned char GPS_BUFFER[GPS_BUFFER_MAX];
unsigned char GPS_BUFFER_INDEX;
unsigned char GPS_MESSAGE_READY;
void GPS_init_USART(uint16_t UBRR);
void GPS_enable_stream(void);
void GPS_disable_stream(void);
void GPS_send_byte(char data);
char GPS_receive_byte(void);
void GPS_flush_buffer(void);
void GPS_parse_data(void);
void GPS_configure_firmware(void);
void GPS_USART_Transmit(unsigned char data);
void GPS_TX_PARSE_ERROR(void);
//10-20-2018
void GPS_request_update(void);
uint8_t GPS_parse_V3(void);
//11-27-2018
uint8_t GPS_parse();

//Variables:
/* GPS CURRENT READINGS DATA STRUCTURE */
typedef struct{
	
	char UTC_TIME_ASCII[GPS_BYTES_ASCII_UTC_TIME];		// hh:mm:ss.sss
	char UTC_TIME_ASCII_LAST[GPS_BYTES_ASCII_UTC_TIME];
	
	char UTC_DATE_ASCII[GPS_BYTES_ASCII_UTC_DATE];		// dd/mm/yy
	
	char LATITUDE_ASCII[GPS_BYTES_ASCII_LATITUDE];		// 1234.6789
	char LATITUDE_ASCII_LAST[GPS_BYTES_ASCII_LATITUDE];
	
	char LONGITUDE_ASCII[GPS_BYTES_ASCII_LONGITUDE];	// -12345.7890__
	char LONGITUDE_ASCII_LAST[GPS_BYTES_ASCII_LONGITUDE];
	
	char SPEED_ASCII[GPS_BYTES_ASCII_SPEED];			// 23.2
	char COURSE_ASCII[GPS_BYTES_ASCII_COURSE];		// 259.00, 34.12
	char STATUS;					// A
	char NS;						// N
	char EW;						// W
	
	char IS_PROCESSING;				//Value is 1 if a data is being processed.
	
} GPS_data;
extern GPS_data SYS_GPS;

/* FROM EEPROM */
//Persistent (EEPROM-buffered) data:
extern uint8_t	NV_USER_PREFERENCES_0,
NV_USER_PREFERENCES_1,
NV_SYSTEM_STATUS_0,
NV_SYSTEM_STATUS_1;
void EEPROM_enable(void);
void EEPROM_write(uint16_t address, uint8_t data);
uint8_t EEPROM_read(uint16_t address);
void EEPROM_recovery(void);

/* FROM BUZZER */
void init_buzzer(void);
void delay_ms(uint16_t duration);
void tone(uint16_t frequency, uint16_t duration);
void tone_START(uint16_t frequency);
void tone_END (void);
void play(Note *song, uint8_t song_len, uint8_t song_cut);

/* FROM LCD */

/* FROM FSM */
/* State enumeration type */
#define sMAIN_MENU		0
#define sSETTINGS		1
#define sNAVIGATION		2
#define sTRACE			3

#define FSM_menu_items_main	2
#define FSM_menu_items_settings 2

extern uint8_t FSM_menu_counter;
extern uint8_t present_state;

void FSM(uint8_t ps, unsigned char key);
void FSM_init(void);

/* APPLICATION */
uint16_t APP_str2int(char * str, uint8_t len);
void APP_loadArr(char * target, char * source, uint16_t off);
uint16_t APP_course2rot();
void APP_genScreen_loading();
void APP_genScreen_main();
void APP_genScreen_settings();
void APP_genScreen_debug();
void APP_genScreen_nav();
void APP_update_MASTER();
void APP_update_main(uint8_t option);
void APP_update_nav();
void APP_update_debug();
void APP_DGPS_incTime();


//NOTE TO SELF: Migrate all of these extern references (^) to their own header file eventually.
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif