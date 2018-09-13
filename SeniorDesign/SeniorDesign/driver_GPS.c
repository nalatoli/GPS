////////////////////////////////////////////////////////////////////////////////////////////////////
//								GPS/USART/UART DRIVER											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	The purpose of this module is to provide basic commands for GPS handling from the MCU, through
	the UART communication protocol. Several notes are included for the software engineer/anyone 
	who reads this document below:
	
		[NOTES]
		-When using the ATmega16A's USART subsystem, you must always read status bits from the 
			receive buffer first, before reading any data. Reads must only happen ONCE to keep
			the buffer order congruent.
			
		-GPS Data transfer bit rate:	9600 Baud (NMEA 0183)
		
		-Control and Status register summaries:
		
			(UCSR0A)	Contains bits that allow error-detection, 2X speed operation, and multi-
						processor mode operation.
			(UCSR0B)	Contains generic [RXEN0/TXEN0] bits to enable communication directions.
						Also contains the interrupt vector enable bits that are triggered during
						various operations in the system. They are: [RXCIE0,TXCIE0,UDRIE0]
			(UCSR0C)	Contains character size information, which determines how many bits of
						a frame are data bits, as opposed to other operational bits. Also contains
						very important bits for determining whether or not the USART module is 
						operating in "Synchronous" or "Asynchronous" (00) modes [UMSEL00, UMSEL01].
		
		-The UDR0(RX) receive register is buffered (FIFO) for 3 characters maximum.
			For our application we must read this data in as soon as possible, to prevent data from
			being lost.
			
		[ASSERTING GPS SLAVE]
		-The GPS module has an enable strobe, that is used to allow the system to function normally.
		when driven low, the module will deactivate and lose its fix. We don't want this to happen
		after the initiation of runtime, so we'll constantly power the EN pin.
		
 */ 
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#define F_CPU 16000000UL
#define FOSC 16000000
#include "util/delay.h"
#include "header_FUNCTIONS.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//								BAUD RATE CONFIGURATION											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#define BAUD 9600
#define MY_UBBR FOSC/16/BAUD-1
////////////////////////////////////////////////////////////////////////////////////////////////////
//								GPS FIRMWARE COMMAND SEQUENCES									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"
#define PMTK_SET_BAUD_9600 "$PMTK251,9600*17\r\n"
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F\r\n"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F\r\n"
#define PMTK_API_SET_FIX_CTL_1HZ  "$PMTK300,1000,0,0,0,0*1C\r\n"
#define PMTK_API_SET_FIX_CTL_5HZ  "$PMTK300,200,0,0,0,0*2F\r\n"

////////////////////////////////////////////////////////////////////////////////////////////////////
//								BUFFER LOCATION DEFINITIONS  									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
//This is used to maintain the offsets for various segments of data for the parser to look at.
#define RMC_UTC			7
#define RMC_STATUS		18
#define RMC_LAT			20
#define RMC_NS			30
#define RMC_LON			32
#define RMC_EW			43
#define RMC_SPEED		45
#define RMC_COURSE		50
#define RMC_DATE		57
#define RMC_MAGVAR		64
#define RMC_MODE		69
#define RMC_CHECKSUM	71

////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER SYSTEM VARIABLES											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* SYSTEM SPECIFIC STRUCTURE */
	GPS_data SYS_GPS;				
					
/* MONTH POINTERS */
const char *MONTH_TABLE[] = {	"January  ",
								"February ",
								"March    ",
								"April    ",
								"May      ",
								"June     ",
								"July     ",
								"August   ",
								"September",
								"October  ",
								"November ",
								"December "	};		//Default string length is 9 bytes
								
/* LINEAR BUFFER */
//This is used to store data transactions from the UART port, whenever a '$' character
//is detected in the UDR0 register. After detection, serial storage of data will begin,
//until the checksum is evaluated, whereupon the system will flush the buffer and wait
//for another '$'
unsigned char GPS_BUFFER[125];			//A byte-address length buffer (256 bytes) 
										//							   {Max address: 0xFF, or 255}
unsigned char GPS_BUFFER_INDEX = 0;
unsigned char GPS_MESSAGE_READY = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER FUNCTION SET/PROTOTYPES									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*		--------------------
		Necessary functions:
		--------------------
		-GPS_init_USART			:	Set the framing/baud rate for the system, with asynchronous
									operation (9600 Baud). Un-assert other colliding peripherals, 
									and assert the GPS unit.
									Ideally can be terminated with interrupts, albeit unnecessary.
		-GPS_parse_buffer		:	This is triggered at the end of a transmission from the GPS
									device, specifically when the control characters are
									detected. Depending on what kind of NMEAS signature was
									registered, different segments of data will be updated in the
									MCU RAM. Afterwards, this function should shut down the
									receiver temporarily to flush the buffer, and re-enable it
									afterwards.
		-USART_flush_buffer		:	Empties the contents of the linear buffer, and also resets the
									position of the buffer index to the beginning.
		-GPS_begin_trace		:	Alters the system state to the "TRACING" mode. This forces the
									system to serially deposit coordinate data into the MicroSD
									card after generating a header that can be easily interpreted
									when extracting the data. It is critical to keep track in both
									SRAM and EEPROM of how many bytes have been written to the
									MicroSD card. Alternatively, store this information in a known
									partition of the MicroSD card itself. Perhaps consider running
									a formatting algorithm on startup that looks for a memory card,
									and checks to see if the signature exists on it. If the card
									is devoid of the signature, write it in from the system. The
									reason this kind of caution must be taken, is due to the 
									potential hazard of recovery after a sudden shutdown. The 
									acquisition of data in the TRACING mode is automatically
									achieved in the 'GPS_parse_buffer' function, which has a
									logically gated check, that depends on the system state status.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER FUNCTIONS												  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* PROTOTYPES */
void GPS_init_USART(uint16_t UBRR);
void GPS_USART_Transmit(unsigned char data);
void GPS_configure_firmware(void);
void GPS_enable_stream(void);
void GPS_disable_stream(void);
void GPS_send_byte(char data);
char GPS_receive_byte(void);
void GPS_flush_buffer(void);
void GPS_parse_data(void);

/* FIRMWARE COMMANDS */
//Or at least the ones we care about:
// turn on only the second sentence (GPRMC)
const unsigned char FIRM_RMC[51] =			"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
const unsigned char FIRM_BAUD[18] =			"$PMTK251,9600*17\r\n";
const unsigned char FIRM_ECHO_1HZ[18] =		"$PMTK220,1000*1F\r\n";
const unsigned char FIRM_ECHO_10HZ[17] =	"$PMTK220,100*2F\r\n";
const unsigned char FIRM_FIX_1HZ[26] =		"$PMTK300,1000,0,0,0,0*1C\r\n";
const unsigned char FIRM_FIX_5HZ[25] =		"$PMTK300,200,0,0,0,0*2F\r\n";
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		GPS_configure_firmware(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	This function assumes that the system USART baud rate was already configured
					to operate at the appropriate baud rate. Ideally, the stream should be closed
					when invoking this function. Therefore for proper initialization at the
					outset of chip startup, the process should follow:
					
					1.	System recovery
					2.	GPS_init_USART(9600);
					3.	GPS_configure_firmware();
					4.	GPS_enable_stream();
	
*/
void GPS_configure_firmware(void){

	//We want our system to receive sentences as fast as possible, but only of RMC type:
	
	//1. RMC ONLY!
	for(uint8_t i = 0; i < 51; i++){
		GPS_USART_Transmit(FIRM_RMC[i]);
			}
	
	//2. 10HZ data echoing:
	for(uint8_t i = 0; i < 17; i++){
		GPS_USART_Transmit(FIRM_ECHO_10HZ[i]);
			}
	
	//3. Fast position fix:
	for(uint8_t i = 0; i < 25; i++){
		GPS_USART_Transmit(FIRM_FIX_5HZ[i]);
			}
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		GPS_init_USART(uint16_t UBRR);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function sets up the USART topology on the ATmega to produce settings congruent
					with interfacing the GPS patch module. Accepts the UBRR value as an input.
					However, this is not necessary for our application.
	
*/
void GPS_init_USART(uint16_t UBRR){
	
	//Set baud rate for 9600 bits per second:
	// [TYPICAL PARAMETER: 103 @ 16MHz system clock]
	UBRRH = (unsigned char)(UBRR >> 8);
	UBRRL = (unsigned char)UBRR;
	
	//Enable the transmitter and receiver bits:
	UCSRB = (1 << RXEN)|(1 << TXEN);
	
	//Set data frame format to accept 1 byte per frame, with 1 stop bit:
	UCSRC = (1 << URSEL)|(1 << UCSZ1)|(1 << UCSZ0)|(1 << USBS);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GPS_USART_Transmit(unsigned char data){
	
	while(!(UCSRA & (1 << UDRE)));
	
	UDR = data;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_enable_stream(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function enables the receive-complete interrupt enable bit. This allows the
					system to look at the contents of the UDR register whenever a byte is received.
					If you want to start searching for NMEA sentences, this function must be 
					invoked. Afterwards, the interrupt handler attached will check several 
					conditions relevant to parsing, which should happen seamlessly.
	
*/
void GPS_enable_stream(void){
	
	//Flush the buffer each time, such that there is a fresh start:
	GPS_flush_buffer();
	
	//Also make sure that the parsing flag is reset as well to prevent erroneous behavior:
	GPS_MESSAGE_READY = 0;
	
	//Set receiver-interrupt enable bit, while maintaining register contents:
	UCSRB |= (1 << RXCIE);
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_disable_stream(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function performs the opposite of its counterpart, listed above. When data
					enters the receiver and sets the interrupt flag, that data is ignored.
	
*/
void GPS_disable_stream(void){
	
	//Clear receiver-interrupt enable bit, while maintaining register contents:
	UCSRB = UCSRB & (!(1 << RXCIE));
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_send_byte(unsigned char data);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function is used during the firmware update process between the ATmega and the
					patch GPS module. This function is invoked as many times as are necessary to
					deplete the transmitter buffer.
	
*/
void GPS_send_byte(char data){
	
	//Wait for the system to be ready to transmit more data:
	while(!(UCSRA & (1 << UDRE)));
	
	//Load byte into the transmitter:
	UDR = data;

}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		unsigned char GPS_receive_byte(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function is used to read from the receiver, which is critical for data
					acquisition in our system.
	
*/
char GPS_receive_byte(void){
	
	//Check to see if data has been received:
	while(!(UCSRA & (1 << RXC)));
	
	//Return the data to where it is desired:
	return UDR;
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_flush_buffer(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Systematically clears out the data contained in the USART receive buffer, 
					then sets the value of the head pointer to 0;
	
*/
void GPS_flush_buffer(void){
	
	//Clear out data
	for(int i = 0; i < 120; i++){
		GPS_BUFFER[i] = 0;
	}
	
	//Set pointer to zero:
	GPS_BUFFER_INDEX = 0;				//Buffer is at start position.
	
	//Clear the pre-parsing flag:
	GPS_MESSAGE_READY = 0;				//Buffer is closed off.
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_parse_data(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	After the linear USART buffer is filled with data, this function will be called.
					The function should halt the operation of the USART receiver, and look byte-by-
					byte through the linear buffer to extract the relevant NMEA output sentences.
					After the completion of this parsing, the receiver will be enabled again, which
					will clear the buffer and its reception flag, and the cycle will repeat.
			
*/
void GPS_parse_data(void){
	
	//1. Shut down the stream.
	//GPS_disable_stream();
	
	//2. Reset the MESSAGE_RECEIVED flag
	GPS_MESSAGE_READY = 0;
	
	//3. Look for the sentence type, and confirm RMC:
	//	-The first character should be '$'
	//	-Therefore, GPS_BUFFER[1] is the first sentence-indicating character.
	if( (GPS_BUFFER[1] == 'G')&&
		(GPS_BUFFER[2] == 'P')&&
		(GPS_BUFFER[3] == 'R')&&
		(GPS_BUFFER[4] == 'M')&&
		(GPS_BUFFER[5] == 'C')	){
			//Continue normally!
		}
	else{
		//Enter an interminable loop for debugging:
		//while(1);		//YOU FAILED!
	}
	
	//4. Unpack all of the data between '$' and '*'. Store it on the SYS_GPS object.
		//A. UTC data:
		SYS_GPS.UTC_H = (GPS_BUFFER[RMC_UTC] * 10)		+ (GPS_BUFFER[RMC_UTC + 1]);	
		SYS_GPS.UTC_M = ((GPS_BUFFER[RMC_UTC + 2] * 10) + (GPS_BUFFER[RMC_UTC + 3])); 
		SYS_GPS.UTC_S = ((GPS_BUFFER[RMC_UTC + 4] * 10) + (GPS_BUFFER[RMC_UTC + 5]));
		//B. STATUS:
		SYS_GPS.STATUS = GPS_BUFFER[RMC_STATUS];
		//C. LATITUDE:
		SYS_GPS.latitude_H =	(	(GPS_BUFFER[RMC_LAT] * 1000) + 
									(GPS_BUFFER[RMC_LAT + 1] * 100)+
									(GPS_BUFFER[RMC_LAT + 2] * 10)+
									(GPS_BUFFER[RMC_LAT + 3])	);
		SYS_GPS.latitude_L =	(	(GPS_BUFFER[RMC_LAT + 5] * 1000) +
									(GPS_BUFFER[RMC_LAT + 6] * 100)+
									(GPS_BUFFER[RMC_LAT + 7] * 10)+
									(GPS_BUFFER[RMC_LAT + 8])	);
		SYS_GPS.NS = GPS_BUFFER[RMC_NS];
		//D. LONGITUDE
		SYS_GPS.longitude_H =	(	(GPS_BUFFER[RMC_LON] * 10000)+
									(GPS_BUFFER[RMC_LON + 1] * 1000)+
									(GPS_BUFFER[RMC_LON + 2] * 100)+
									(GPS_BUFFER[RMC_LON + 3] * 10)+
									(GPS_BUFFER[RMC_LON + 4])	);
		SYS_GPS.longitude_L =	(	(GPS_BUFFER[RMC_LON + 6] * 1000)+
									(GPS_BUFFER[RMC_LON + 7] * 100)+
									(GPS_BUFFER[RMC_LON + 8] * 10)+
									(GPS_BUFFER[RMC_LON + 9])	);
		SYS_GPS.EW = GPS_BUFFER[RMC_EW];
		//E. SPEED AND COURSE
		SYS_GPS.ground_speed_high = GPS_BUFFER[RMC_SPEED];
		SYS_GPS.ground_speed_low = ((GPS_BUFFER[RMC_SPEED + 2] * 10) + (GPS_BUFFER[RMC_SPEED + 3]));
		SYS_GPS.course_high =	(	(GPS_BUFFER[RMC_COURSE] * 100)+
									(GPS_BUFFER[RMC_COURSE + 1] * 10)+
									(GPS_BUFFER[RMC_COURSE + 2])	);
		SYS_GPS.course_low = ((GPS_BUFFER[RMC_COURSE + 4] * 10) + (GPS_BUFFER[RMC_COURSE + 5]));
		//F. DATE
		SYS_GPS.UTC_DAY =	((GPS_BUFFER[RMC_DATE] * 10) + (GPS_BUFFER[RMC_DATE + 1]));
		SYS_GPS.UTC_MONTH = ((GPS_BUFFER[RMC_DATE + 2] * 10) + (GPS_BUFFER[RMC_DATE + 3]));
		SYS_GPS.UTC_YEAR =	((GPS_BUFFER[RMC_DATE + 4] * 10) + (GPS_BUFFER[RMC_DATE + 5]));
	
	//?. Enable stream, thereby flushing the buffer of all its contents.
	GPS_enable_stream();
	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
