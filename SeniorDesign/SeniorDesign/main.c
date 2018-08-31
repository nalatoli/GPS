////////////////////////////////////////////////////////////////////////////////////////////////////
//								MAIN   PROJECT   BUILD											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This section of the program acts as the main hub for system functionality. This module call on
	several of the other header, and driver files that are critical to control all of the system
	peripherals/subsystems.
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
/* FROM GPS DRIVER */
extern unsigned char GPS_BUFFER [256];
extern unsigned char GPS_BUFFER_INDEX;
extern void GPS_init_USART(uint16_t UBRR);
extern void GPS_enable_stream(void);
extern void GPS_disable_stream(void);
extern void GPS_send_byte(unsigned char data);
extern char GPS_receive_byte(void);
extern void GPS_flush_buffer(void);
extern void GPS_parse_data(char datum);
//Variables:
extern unsigned char GPS_BUFFER [256];
extern unsigned char GPS_BUFFER_INDEX;
extern unsigned char PRE_PARSING_STATUS;	//Used to indicate whether or not we can currently receive



uint8_t SYS_PREFERENCES_0;	//{7:4} Color option, {3} Buzzer mute bool, {2:0} Reserved
uint8_t SYS_STATUS_0;		//{7:0} System operating mode
uint8_t SYS_PERIPHERALS_0;	//{7:0} Reserved for maintenance of which peripherals are asserted/
							//		on line.


//Color preferences enumerated, 4 bit maximum:
typedef enum{white,red,yellow,orange,green,blue,violet,pink}color;

////////////////////////////////////////////////////////////////////////////////////////////////////
//								INTERRUPTS    											          //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* KEYPAD INTERRUPT FOR FSM */

/* GPS INTERRUPT FOR PARSING */
ISR(USARTRXC_vect){
	
	if(PRE_PARSING_STATUS == 1){
	//Step 0: Check GPS buffer for overflow condition:
	// if an overflow is detected, parse the buffer.
	if(GPS_BUFFER_INDEX >= 255){
		//Parse the buffer:
		GPS_parse_data();			//Will shut down, and reset the stream automatically.
	}
	else{
	//Step 1: Store byte in linear buffer, then increase index:
	GPS_BUFFER[GPS_BUFFER_INDEX] = GPS_receive_byte();
	GPS_BUFFER_INDEX++;
	}
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
	DDRA = 0x00;
	PORTA = 0x00;
	
	DDRB = 0x00;
	PORTB = 0x00;
	
	DDRC = 0x00;
	PORTC = 0x00;
	
	DDRD = 0x00;
	PORTD = 0x00;
	
	/* INTERRUPTS */	
	
	
	/* TIMERS/BUZZER */
	
	/* DISPLAY MODULES */
	
		
	//Dormant program loop for testing.
    while (1) 
    {
		_delay_us(0.5);
		PORTB &= ~(1<<0);

		
    }
}
