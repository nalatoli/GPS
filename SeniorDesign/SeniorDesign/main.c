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
#define F_CPU 8000000UL
#include "util/delay.h"
#include "header_LCD.h"
#include "header_BUZZER.h"
#include "header_APPLICATION.h"
#include <string.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Temporary test function prototype */
void HYBRID_print_GPS_data(void);
//Temporary memory for debug:
char text_buffer[2] = {0,'|'};
unsigned char text_column = 0;
unsigned char text_x = 0;
const char test_msg[51] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";

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
	else if((vector_data == '*') && (GPS_MESSAGE_READY)){
		GPS_parse_data();
		APP_update_menu_debug_updateAll();
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
	
	DDRD = 0b00010010;	//Port D contains:
	PORTD = 0x80;	// USART, INT0, INT1, buzzer driver pin
	
	/* RECOVERY */
	//Recovery program is currently defined in the EEPROM driver.
	EEPROM_enable();
	//This recovery program must have two components!
	//EEPROM_recovery(); AND SD_recovery(); which is not yet available!
	EEPROM_recovery();				//Reacquire old persistent information.
	//SD Card must be initialized here as well, and erroneous data must be purged:
	//SD_assert();
	//Initialize the SD card: {Be sure to look into the function when single-stepping}
	//SD_init_card();
	//Find the last entry in memory, then check it byte-wise for both the beginning of
	//an entry ('%') as well as a few other entry-confirming characters.
	
	/* TIMERS/BUZZER */
	

	/* DISPLAY */
	uint16_t text_x = 92, text_y = 184;
	LCD_init_system();				// Init LCD
	init_buzzer();					// Init Buzzer
	APP_generate_menu(LOADINGMENU);	// Display Loading Screen
	LCD_setText_all(text_x,text_y,1,WHITE,GREY);
	_delay_ms(20);
	
	
	/* GPS */
	//Initialize communication, then configure MTK3339 firmware, then open up.
	GPS_BUFFER_INDEX = 0;			//Begin at start of data buffer.
	GPS_MESSAGE_READY= 0;			//Buffer will begin to fill normally.
	LCD_print_str("Initializing USART...    ");
	GPS_init_USART(MY_UBBR);
	
	LCD_setText_cursor(text_x,text_y);
	LCD_print_str("Configuring Firmware...  ");
	GPS_configure_firmware();
	
	LCD_setText_cursor(text_x,text_y);
	LCD_print_str("Enabling Stream...       ");
	APP_generate_menu(DEBUGMENU);
	GPS_enable_stream();			//Begin receiving data

	/* INTERRUPTS */
	sei();
	
	/* DEBUG */
	char gay = 0;
	

	/* P R O G R A M */
	//Dormant program loop for testing.
    while (1){
		gay = !gay;
	}
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//								FUNCTION BUILD SITE  								              //
////////////////////////////////////////////////////////////////////////////////////////////////////
void HYBRID_print_GPS_data(){
	
	//Print out UTC:
	uint8_t temporary;
	uint16_t large_temp;
	uint32_t huge_temp;
	
	temporary = SYS_GPS.UTC_H;
	//Print text:
	LCD_print_str("HOUR: ");
	LCD_print_num(temporary,2,0);
	LCD_print_str("\n");
	LCD_print_str("MINUTE: ");
	LCD_print_num(SYS_GPS.UTC_M,2,0);
	LCD_print_str("\n");
	LCD_print_str("SECOND: ");
	LCD_print_num(SYS_GPS.UTC_S,2,0);
	
	//Reset cursor
	LCD_setText_cursor(0,0);

}