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
#define FOSC 8000000
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

#define parse_commas 10
////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER SYSTEM VARIABLES											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* SYSTEM SPECIFIC STRUCTURE */
	GPS_data SYS_GPS;				
					
/* MONTH POINTERS */
//const char * MONTH_TABLE[12] PROGMEM = {"January  ",
										//"February ",
										//"March    ",
										//"April    ",
										//"May      ",
										//"June     ",
										//"July     ",
										//"August   ",
										//"September",
										//"October  ",
										//"November ",
										//"December "	};		//Default string length is 9 bytes
								
/* LINEAR BUFFER */
//This is used to store data transactions from the UART port, whenever a '$' character
//is detected in the UDR0 register. After detection, serial storage of data will begin,
//until the checksum is evaluated, whereupon the system will flush the buffer and wait
//for another '$'
unsigned char GPS_BUFFER[GPS_BUFFER_MAX];			//A byte-address length buffer 
													//				{Max address: 0xFF, or 255}
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
void GPS_TX_PARSE_ERROR(void);
//10-20-2018
void GPS_request_update(void);				//Allows the GPS to obtain an update.
uint8_t GPS_parse_V3(void);					//Newest, and critically important.
											//Has a vital return type: {0 = FAIL, 1 = VALID}

/* FIRMWARE COMMANDS */
//Or at least the ones we care about:
// turn on only the second sentence (GPRMC)
const unsigned char FIRM_RMC[51] PROGMEM=			"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
const unsigned char FIRM_BAUD[18] PROGMEM=			"$PMTK251,9600*17\r\n";
const unsigned char FIRM_ECHO_1HZ[18] PROGMEM=		"$PMTK220,1000*1F\r\n";
const unsigned char FIRM_ECHO_10HZ[17] PROGMEM=		"$PMTK220,100*2F\r\n";
const unsigned char FIRM_FIX_1HZ[26] PROGMEM=		"$PMTK300,1000,0,0,0,0*1C\r\n";
const unsigned char FIRM_FIX_5HZ[25] PROGMEM=		"$PMTK300,200,0,0,0,0*2F\r\n";

/* BUILT-IN MESSAGES */
//These messages always contain their length as the first character:
const unsigned char MSG_DATA_ERROR[15] PROGMEM =			"DATA NOT VALID!";
const unsigned char MSG_REPORT_UTC[7] PROGMEM =				"[UTC] ";
const unsigned char MSG_GPRMC_NOT_RECEIVED[20] PROGMEM =	"GPRMC NOT AVAILABLE!";

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_parse_V3(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	This function is called at the end of an update request. The new parser does
					not automatically perform the math necessary to compress the SRAM space, but
					instead simply stores characters serially into their buffers. 
					
					',' characters are used to determine what sequence of the parse you are within,
					since it was discovered that the NMEA GPRMC sentences do not have a finite
					length. Data validity status must also be checked to determine what is going
					on.
					
					This function begins by disabling the stream, and ends by flushing the buffer,
					and reseting the interrupt-parser variable GPS_MESSAGE_READY = 0;
	
*/
uint8_t GPS_parse_V3(void){
	
	//Disable the stream:
	GPS_disable_stream();
	//Reset the MESSAGE_READY conditional variable:
	GPS_MESSAGE_READY = 0;
	
	//Enable interrupts for other systems:
	sei();
	
	//Begin the actual parsing:
	//When looping, the maximum comma count we are interested in for GPRMC NMEA is (10). 
	
	for(uint8_t comma = 0; comma < parse_commas; comma++){
		
		//Use an internal byte counter for SYS_GPS array placement.
		uint8_t byte = 0;
		uint8_t stop = 0;
		
		switch(comma){
			case 0:		//Message ID (GPRMC)	|	5	|	GPRMC			|
						//Place buffer index at the beginning of the GPRMC sentence:
						GPS_BUFFER_INDEX = 1;
						//Check to see if the next 5 bytes are GPRMC, or break the loop:
						if(	(GPS_BUFFER[GPS_BUFFER_INDEX	] == 'G')&&
							(GPS_BUFFER[GPS_BUFFER_INDEX + 1] == 'P')&&
							(GPS_BUFFER[GPS_BUFFER_INDEX + 2] == 'R')&&
							(GPS_BUFFER[GPS_BUFFER_INDEX + 3] == 'M')&&
							(GPS_BUFFER[GPS_BUFFER_INDEX + 4] == 'C') )
							{GPS_BUFFER_INDEX += 6; break;}
						//If the sentence wasn't detected, force-break the switch and loop:
						else{	SYS_GPS.IS_PROCESSING = 0;
								return 1;}
						
			case 1:		//UTC Time				|	10	|	hhmmss.sss		|
						//While you have not reached ',' store the subsequent bits in SYS.GPS:
						while(GPS_BUFFER[GPS_BUFFER_INDEX] != ','){
							if(byte < GPS_BYTES_ASCII_UTC_TIME){
								if(byte==2||byte==5) byte++;
								SYS_GPS.UTC_TIME_ASCII[byte] = GPS_BUFFER[GPS_BUFFER_INDEX];
								byte++;
							}
							GPS_BUFFER_INDEX++;
						}
						//The ',' character was detected, so jump over it.
						GPS_BUFFER_INDEX++;
						int8_t timeCorrection = atoi(SYS_GPS.UTC_TIME_ASCII) - 5;
						if(timeCorrection<0) timeCorrection+= 24;
						itoa(timeCorrection,SYS_GPS.UTC_TIME_ASCII+(timeCorrection>9?0:1),10);
						SYS_GPS.UTC_TIME_ASCII[2] = ':';
						//comma++;
						break;

			case 2:		//Data Status			|	1	|	A				|
						//This data is always available, and tells us whether or not any real data is present:
						if(GPS_BUFFER[GPS_BUFFER_INDEX] == 'A'){
							SYS_GPS.STATUS = 'A';
							GPS_BUFFER_INDEX += 2; 
							break;
						}
						//Force-terminate the parse. 
						//Send the error flag.
						else{	
							SYS_GPS.IS_PROCESSING = 0;
							SYS_GPS.STATUS = 'V';
							return 1;
						} 
							
			case 3:		//Latitude				|	9	|	ddmm.mmmm		|
						//While you have not reached ',' store the subsequent bits in SYS.GPS:
						while(GPS_BUFFER[GPS_BUFFER_INDEX] != ','){
							//Write directly into the system:
							if(GPS_BUFFER[GPS_BUFFER_INDEX] == '.') GPS_BUFFER_INDEX++;
							SYS_GPS.LATITUDE_ASCII[byte] = GPS_BUFFER[GPS_BUFFER_INDEX];
							//Increment buffer and byte counter:
							GPS_BUFFER_INDEX++;
							byte++;
						}
						//Finalize by increasing counts and hopping over the comma:
						GPS_BUFFER_INDEX++;
						
						break;

			case 4:		//N/S					|	1	|	N				|				
						SYS_GPS.NS = GPS_BUFFER[GPS_BUFFER_INDEX];
						// Add '-' character for southern bound latitude
						if(SYS_GPS.NS == 'S'){
							for(int i = GPS_BYTES_ASCII_LATITUDE-2; i > 0; i--) SYS_GPS.LATITUDE_ASCII[i] = SYS_GPS.LATITUDE_ASCII[i-1];
							SYS_GPS.LATITUDE_ASCII[0] = '-';
						}

						//Hop over the ","
						GPS_BUFFER_INDEX += 2;
						//Increment count:
							
						//Break:
						break;
						
						
			case 5:		//Longitude				|	10	|	12016.4438		|
						//While you have not reached ',' store the subsequent bits in SYS.GPS:
						while(GPS_BUFFER[GPS_BUFFER_INDEX] != ','){
							if(GPS_BUFFER[GPS_BUFFER_INDEX] == '.') GPS_BUFFER_INDEX++;
							//Write directly into the system:
							SYS_GPS.LONGITUDE_ASCII[byte] = GPS_BUFFER[GPS_BUFFER_INDEX];
							//Increment buffer and byte counter:
							GPS_BUFFER_INDEX++;
							byte++;
						}
						
						GPS_BUFFER_INDEX++;
						break;
						
			case 6:		//E/W					|	1	|	E				|
						SYS_GPS.EW = GPS_BUFFER[GPS_BUFFER_INDEX];
						if(SYS_GPS.EW == 'W'){
							for(int i = GPS_BYTES_ASCII_LONGITUDE-2; i > 0; i--) SYS_GPS.LONGITUDE_ASCII[i] = SYS_GPS.LONGITUDE_ASCII[i-1];
							SYS_GPS.LONGITUDE_ASCII[0] = '-';
						}
						GPS_BUFFER_INDEX += 2;
							
						break;
						
			case 7:		//Speed					|	4	|	0.03			|
						while(GPS_BUFFER[GPS_BUFFER_INDEX] != ','){
							if(GPS_BUFFER[GPS_BUFFER_INDEX] == '.') stop = 1;
							if(!stop){
								SYS_GPS.SPEED_ASCII[byte] = GPS_BUFFER[GPS_BUFFER_INDEX];
								byte++;
							}
							GPS_BUFFER_INDEX++;
						} 
						GPS_BUFFER_INDEX++;
						break;
						
			case 8:		//Course				|	4-5	|	165.03 OR 43.45	|
						while(GPS_BUFFER[GPS_BUFFER_INDEX] != ','){
							if(GPS_BUFFER[GPS_BUFFER_INDEX] == '.') stop = 1;
							if(!stop){
								SYS_GPS.COURSE_ASCII[byte] = GPS_BUFFER[GPS_BUFFER_INDEX];
								byte++;
							}
							GPS_BUFFER_INDEX++;
						}
						GPS_BUFFER_INDEX++;
						break;
		
			case 9:		//Date					|	6	|	ddmmyy			|
						//While you have not reached ',' store the subsequent bits in SYS.GPS:
						while(GPS_BUFFER[GPS_BUFFER_INDEX] != ','){
							if(byte < GPS_BYTES_ASCII_UTC_DATE){
								if(byte==2||byte==5) byte++;
								SYS_GPS.UTC_DATE_ASCII[byte] = GPS_BUFFER[GPS_BUFFER_INDEX];
								byte++;
							}
							GPS_BUFFER_INDEX++;
						}
		}
	}


	//After the for loop, figure out what variables need to be reset and whatnot.
	SYS_GPS.IS_PROCESSING = 0;
	GPS_MESSAGE_READY = 0;
	GPS_BUFFER_INDEX = 0;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_request_update(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Enables the receiver, and the interrupt that handles incoming data.
					Also maintains track of parser variables.
	
*/
void GPS_request_update(void){
	
	//Reset parser variables:
	GPS_MESSAGE_READY = 0;
	GPS_BUFFER_INDEX = 0;
	SYS_GPS.IS_PROCESSING = 1;
	//Enable receiver interrupt:
	UCSR0B |= (1 << RXCIE0);

	
	
}
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
	
	GPS_init_USART(MY_UBBR);
	
	//1. BAUD TO 9600
	for(uint8_t i = 0; i < 18; i++){
		GPS_USART_Transmit(pgm_read_byte(&FIRM_BAUD[i]));
	}
	
	//1. RMC ONLY!
	for(uint8_t i = 0; i < 51; i++){
		GPS_USART_Transmit(pgm_read_byte(&FIRM_RMC[i]));
			}
	
	//2. 10HZ data echoing:
	for(uint8_t i = 0; i < 17; i++){
		GPS_USART_Transmit(pgm_read_byte(&FIRM_ECHO_10HZ[i]));
			}
	
	////3. Fast position fix:
	//for(uint8_t i = 0; i < 25; i++){
		//GPS_USART_Transmit(pgm_read_byte(&FIRM_FIX_5HZ[i]));
	//}
			
	/* Set Default Strings */
	SYS_GPS.STATUS = 'V';
	strcpy(SYS_GPS.UTC_TIME_ASCII,"00:00:00");	strcpy(SYS_GPS.UTC_TIME_ASCII_LAST,"00:00:00");
	strcpy(SYS_GPS.UTC_DATE_ASCII,"00/00/00");
	strcpy(SYS_GPS.LATITUDE_ASCII,"00000000");		strcpy(SYS_GPS.LATITUDE_ASCII_LAST,"00000000");
	strcpy(SYS_GPS.LONGITUDE_ASCII,"000000000");	strcpy(SYS_GPS.LONGITUDE_ASCII_LAST,"000000000");
	strcpy(SYS_GPS.SPEED_ASCII,"0");
	strcpy(SYS_GPS.COURSE_ASCII,"000");
	SYS_GPS.NS = 'N';
	SYS_GPS.EW = 'E';
	SYS_GPS.IS_PROCESSING=0;
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
			UBRR0H = (unsigned char)(UBRR >> 8);
			UBRR0L = (unsigned char)UBRR;
	
			//Enable the transmitter and receiver bits:
			UCSR0B |= (1 << RXEN0)|(1 << TXEN0);
	
			//Set data frame format to accept 1 byte per frame, with 1 stop bit:
			UCSR0C = (1 << UCSZ01)|(1 << UCSZ00)|(1 << USBS0);
			
			
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void GPS_USART_Transmit(unsigned char data){
	
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_enable_stream(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function enables the receive-complete interrupt enable bit. This allows the
					system to look at the contents of the UDR0 register whenever a byte is received.
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
			UCSR0B |= (1 << RXCIE0);
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		void GPS_disable_stream(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function performs the opposite of its counterpart, listed above. When data
					enters the receiver and sets the interrupt flag, that data is ignored.
	
*/
void GPS_disable_stream(void){
	
	//Disable receiver interrupt:
	UCSR0B &= ~(1 << RXCIE0);
	
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
	while(!(UCSR0A & (1 << UDRE0)));
	
	//Load byte into the transmitter:
	UDR0 = data;

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
	while(!(UCSR0A & (1 << RXC0)));
	
	//Return the data to where it is desired:
	return UDR0;
	
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
	for(int i = 0; i < GPS_BYTES_ASCII_TOTAL; i++){
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
/*
void GPS_parse_data(void){
	
	//1. Shut down the stream.
//	GPS_disable_stream();
	
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
		//When the GPRMC is not detected, we need to let the serial monitor know.
		for(int i = 0; i < 20; i++){
			GPS_send_byte(MSG_GPRMC_NOT_RECEIVED[i]);
		}
		GPS_send_byte('\r');
		GPS_send_byte('\n');
		return;
	}
	
	//4. Unpack all of the data between '$' and '*'. Store it on the SYS_GPS object.
		//A. UTC data:
		for(int i = 0; i < GPS_BYTES_ASCII_UTC_TIME; i++){
			SYS_GPS.UTC_TIME_ASCII[i] = GPS_BUFFER[RMC_UTC + i];
		}
		SYS_GPS.UTC_H = (GPS_BUFFER[RMC_UTC] * 10)		+ (GPS_BUFFER[RMC_UTC + 1]);	
		SYS_GPS.UTC_M = ((GPS_BUFFER[RMC_UTC + 2] * 10) + (GPS_BUFFER[RMC_UTC + 3])); 
		SYS_GPS.UTC_S = ((GPS_BUFFER[RMC_UTC + 4] * 10) + (GPS_BUFFER[RMC_UTC + 5]));
		//B. STATUS:
		SYS_GPS.STATUS = GPS_BUFFER[RMC_STATUS];
// LOGICAL CHECK:
// If the data received is NOT valid, then the program will skip over the rest of the parse, and send out a
// "no fix" message:
if (SYS_GPS.STATUS == 'V'){
	//Send the error message containing the UTC data:
	GPS_TX_PARSE_ERROR();
	GPS_disable_stream();
	return;	
}
		//C. LATITUDE:
		//Buffer ASCII characters:
		for(uint8_t i = 0; i < GPS_BYTES_ASCII_LATITUDE; i++){
			SYS_GPS.LATITUDE_ASCII[i] = GPS_BUFFER[RMC_LAT + i];
		}
		//Compress to decimal:
		SYS_GPS.latitude_H =	(	(GPS_BUFFER[RMC_LAT]		* 1000)		+ 
									(GPS_BUFFER[RMC_LAT + 1]	* 100)		+
									(GPS_BUFFER[RMC_LAT + 2]	* 10)		+
									(GPS_BUFFER[RMC_LAT + 3]));
		SYS_GPS.latitude_L =	(	(GPS_BUFFER[RMC_LAT + 5]	* 1000)		+
									(GPS_BUFFER[RMC_LAT + 6]	* 100)		+
									(GPS_BUFFER[RMC_LAT + 7]	* 10)		+
									(GPS_BUFFER[RMC_LAT + 8]));
		SYS_GPS.NS = GPS_BUFFER[RMC_NS];
		//D. LONGITUDE
		//Buffer ASCII characters:
		for(uint8_t i = 0; i < GPS_BYTES_ASCII_LONGITUDE; i++){
		SYS_GPS.LONGITUDE_ASCII[i] = GPS_BUFFER[RMC_LON + i];
		}
		//Compress to decimal:
		SYS_GPS.longitude_H =	(	(GPS_BUFFER[RMC_LON]		* 10000)	+
									(GPS_BUFFER[RMC_LON + 1]	* 1000)		+
									(GPS_BUFFER[RMC_LON + 2]	* 100)		+
									(GPS_BUFFER[RMC_LON + 3]	* 10)		+
									(GPS_BUFFER[RMC_LON + 4]));
		SYS_GPS.longitude_L =	(	(GPS_BUFFER[RMC_LON + 6]	* 1000)		+
									(GPS_BUFFER[RMC_LON + 7]	* 100)		+
									(GPS_BUFFER[RMC_LON + 8]	* 10)		+
									(GPS_BUFFER[RMC_LON + 9]));
		SYS_GPS.EW = GPS_BUFFER[RMC_EW];
		//E. SPEED AND COURSE
		//Buffer ASCII characters:
		for(uint8_t i = 0; i < GPS_BYTES_ASCII_SPEED; i++){
			SYS_GPS.SPEED_ASCII[i] = GPS_BUFFER[RMC_SPEED + i];
		}
		//Compress to decimal:
		SYS_GPS.ground_speed_high = GPS_BUFFER[RMC_SPEED];
		SYS_GPS.ground_speed_low = ((GPS_BUFFER[RMC_SPEED + 2]		* 10)	+ 
									(GPS_BUFFER[RMC_SPEED + 3]));
		//Buffer ASCII characters:
		for(uint8_t i = 0; i < GPS_BYTES_ASCII_COURSE; i++){
			SYS_GPS.COURSE_ASCII[i] = GPS_BUFFER[RMC_COURSE + i];
		}
		//Compress to decimal:
		SYS_GPS.course_high =	(	(GPS_BUFFER[RMC_COURSE]			* 100)	+
									(GPS_BUFFER[RMC_COURSE + 1]		* 10)	+
									(GPS_BUFFER[RMC_COURSE + 2]));
		SYS_GPS.course_low = (		(GPS_BUFFER[RMC_COURSE + 4]		* 10)	+ 
									(GPS_BUFFER[RMC_COURSE + 5]));
		//F. DATE
		//Buffer ASCII characters:
		for(uint8_t i = 0; i < GPS_BYTES_ASCII_UTC_DATE; i++){
			SYS_GPS.UTC_DATE_ASCII[i] = GPS_BUFFER[RMC_DATE + i];
		}
		//Compress to decimal:
		SYS_GPS.UTC_DAY =	((GPS_BUFFER[RMC_DATE]		* 10)	+ (GPS_BUFFER[RMC_DATE + 1]));
		SYS_GPS.UTC_MONTH = ((GPS_BUFFER[RMC_DATE + 2]	* 10)	+ (GPS_BUFFER[RMC_DATE + 3]));
		SYS_GPS.UTC_YEAR =	((GPS_BUFFER[RMC_DATE + 4]	* 10)	+ (GPS_BUFFER[RMC_DATE + 5]));
	
	//Disable the stream until we want another update.
	GPS_disable_stream();
	//Flush the entire buffer:
	GPS_flush_buffer();
	
};
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void GPS_TX_PARSE_ERROR(void){
	//To get here, the invalid data character was detected.
	
	//Send a transmission containing the UTC time, as well as
	//a generic NO FIX message:
	for(uint8_t i = 0; i < 15; i++){
		GPS_send_byte(MSG_DATA_ERROR[i]);
	}
	
	//Send carriage return and linefeed:
	GPS_send_byte('\r');
	GPS_send_byte('\n');
	
	//Send the UTC time and date:
	for(uint8_t i = 0; i < 7; i++){
		GPS_send_byte(MSG_REPORT_UTC[i]);
	}
	GPS_send_byte(SYS_GPS.UTC_H);
	GPS_send_byte('.');
	GPS_send_byte(SYS_GPS.UTC_M);
	GPS_send_byte('.');
	GPS_send_byte(SYS_GPS.UTC_S);
	GPS_send_byte('\r');
	GPS_send_byte('\n');

}
*/