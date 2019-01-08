#include "header_APPLICATION.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//											    Main    										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************	
	- Starts by initializing the MCU and peripheral systems and loads the main menu:
			
		1. MCU - Microchip ATmega16A
			- Communicates with all peripherals and processes peripheral and user-input
			  information
			
		2. LCD - HiLetgo 2.2" SPI TFT LCD Module
			- Displays graphical-user interface. The module communicates with the MCU via a 
			  SPI
			
		3. DISK - 8GB Sandisk MicroSD Card w/ Adafruit MicroSD Card Breakout
			- Stores extended application-based information. The module communicates with the 
			  MCU via SPI
				
		4. GPS - Adafruit Ultimate GPS Breakout
			- Transmits GPS information to MCU via USART
				
	- After initialization is complete, the MCU enters standby mode, waiting for interrupts
		
***************************************************************************************************/

Vector2 tmp = {0,0};

int main(void)
{	
	//APP_loadProgram();
	
	while(1){DDRA ^=(1<<0);}
	

	
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//											 Interrupts   										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************
	INTn - External Button Interrupts
	- Executes upon pressing button # 'n'
	- Updates FSM with user-input, triggering selection updates:
	
	n | Function | Description
	--|----------|--------------------------------------
	0 | UP       | Cycles upwards in a list of options
	1 | DOWN     | Cycles downwards in a list of options
	2 | EXECUTE  | Executes highlighted option
	
***************************************************************************************************/
ISR(INT0_vect)
{
	/* Enter DOWN into System */
	SFX_tone(FREQ_C6,100);
	KEY_scroll(KEY_DOWN);
}

ISR(INT1_vect)
{
	/* Enter EXECUTE into System */
	SFX_tone(FREQ_E6,100);
	KEY_execute();
}

ISR(INT2_vect)
{
	/* Enter UP into System */
	SFX_tone(FREQ_G6,100);
	KEY_scroll(KEY_UP);
}

/***************************************************************************************************
	TIMERn - Timer Interrupts
	- Executes every 1 ms
	- Calls corresponding function
	
	n | Function | Description
	--|----------|------------------------------------
	0 | UPDATE   | Controls frequency of master update
	2 | ITONE    | Controls state of passive SFX
	
	USARTRXC - USART Receive-Data Interrupt
	- Executes on request
	- Processes/buffers GPS information from GPS module
	
***************************************************************************************************/
ISR(TIMER0_COMPA_vect)
{
	/* Increment Towards Master Update */	// ***
	static uint32_t count = 0;				// Initialize static count to 0
	if(++count > MASTERUPDATETIME){			// Increment count / if has reached master update time,
		APP_update_MASTER();				//  Execute master update
		count = 0;							//  Reset count
	}
}

ISR(TIMER2_COMPB_vect)
{
	/* Increment Towards Tone Completion */
	static uint32_t count = 0;
	if(++count > iTONETIME){				// Increment count / if has reached itone end time,
		TIMSK2 &= ~(1<<OCIE2B);				//  Disable Timer 2 interrupt
		SFX_tone_end();						//  Disable tone
		count = 0;							//  Reset Count
	}
}

/***************************************************************************************************
	USARTRXC - USART Receive-Data Interrupt
	- Executes on request
	- Processes/buffers GPS information from GPS module
	
***************************************************************************************************/	
ISR(USART0_RX_vect)
{	
	/* Buffer USART Data */
	char vector_data = GPS_receive_byte();
	
	/* Perform Data Analysis */								// ***
	if(vector_data == '$'){									// If character is '$' (Sequence-Start Indicator),
		GPS_MESSAGE_READY = 1;								//  Set flag to being filling buffer,
		GPS_BUFFER[0] = '$';								//  Insert '$' into first buffer element and
		GPS_BUFFER_INDEX = 1;								//  Set buffer index to 1 (index 0 = '$')
	}
	
	else if((vector_data == '*') && (GPS_MESSAGE_READY)){	// Else, if character is '*' (Sequence-Terminating Indicator) and buffer is flagged to fill,
		GPS_parse_V3();										//  Load SYS_GPS with updated information
	}
	
	else if(GPS_MESSAGE_READY){								// Else (no indicators detected), if buffer is flagged to fill,
		GPS_BUFFER[GPS_BUFFER_INDEX] = vector_data;			//  Insert data into buffer and
		GPS_BUFFER_INDEX++;									//  Increment buffer index
	}
}
