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
#include "util/delay.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//								GPS FIRMWARE COMMAND SEQUENCES									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MTK_EXAMPLE 500
////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER SYSTEM VARIABLES											  //
////////////////////////////////////////////////////////////////////////////////////////////////////

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
	unsigned char validity;
									
	//Latitude and longitude in degrees
	uint16_t latitude_H;
	uint16_t latitude_L;	//Below the decimal point
	uint8_t NS;				//Indicates your placement relative to the equator.
	uint32_t longitude_H;
	uint16_t longitude_L;	//Below the decimal point
	uint8_t EW;				//Indicates you placement relative to the prime meridian.
	
	//Ground speed (1*10^-2 knots)
	uint8_t ground_speed;
	
	//Course over ground (azimuth angle from GPS North) {DECIMATED}
	uint16_t course_high;	//Indicates the 360 degrees above the decimal point.
	uint8_t  couse_low;		//Indicates the precision value below the decimal point.

	}GPS_data;
	
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
unsigned char GPS_BUFFER [256];			//A byte-address length buffer (256 bytes) 
										//							   {Max address: 0xFF, or 255}
unsigned char GPS_BUFFER_INDEX = 0;
unsigned char PRE_PARSING_STATUS = 0;	//Used to indicate whether or not we can currently receive
										//a valid string of data.
unsigned char PARSING_PHASE[2] = {0,0};	//The array related to the parsing phase is broken up into
										//two bytes:
										//PARSING_PHASE[0] : Contains the larger phase count
										//PARSING_PHASE[1] : Contains a pace counter for sub-phases

////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER FUNCTION SET/PROTOTYPES									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*		--------------------
		Necessary functions:
		--------------------
		-GPS_init_USART			:	Set the framing/baud rate for the system, with asynchronous
									operation (9600 Baud). Un-assert other colliding peripherals, 
									and assert the GPS unit.
		-GPS_select_datum		:	Chooses one of the data packing formats listed in the GPS
									data sheet. For more information, go there. Consider using
									enumeration encoding for the 3-character parameters to this
									function.
		-GPS_satellite_acquire	:	Does necessary work to engage the GPS module to prompt 
									satellite acquisition. Also presents wait message to UI. 
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
		-store_trace			:
		-terminate_trace		:
		
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
//								DRIVER FUNCTIONS												  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* PROTOTYPES */
void GPS_init_USART(uint16_t UBRR);
void GPS_configure_firmware(void);
void GPS_enable_stream(void);
void GPS_disable_stream(void);
void GPS_send_byte(unsigned char data);
char GPS_receive_byte(void);
void GPS_flush_buffer(void);
void GPS_parse_data(char datum);
	//Sub-functions necessary for the parsing algorithm:
	void parse_GPRMC(void);
/* FIRMWARE COMMANDS */
//Or at least the ones we care about:
// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define PMTK_SET_BAUD_9600 "$PMTK251,9600*17"
#define PMTK_API_SET_FIX_CTL_1HZ  "$PMTK300,1000,0,0,0,0*1C"
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"

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
	UCSRC = (0 << UMSEL)|(1 << UCSZ1)|(1 << UCSZ0)|(0 << USBS);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	FUNCTION:		GPS_configure_firmware(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function initializes the USART system to operate at 9600 baud for the GPS.
					Then, the transmitter is enabled, and a few commands are sent over to the
					device to configure its firmware for specified parameters. Afterwards,
					the device is un-asserted, and everything returns to normal to prevent
					collision.
	
*/
void GPS_configure_firmware(void){
	//Enable USART
	GPS_init_USART();

	//Make sure the receiver is closed, but load linear buffers to send out the firmware commands
	//serially:
	
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
	PRE_PARSING_STATUS = 0;
	
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
void GPS_send_byte(unsigned char data){
	
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
	for(int i = 0; i <= 255; i++){
		GPS_BUFFER[i] = 0;
	}
	
	//Set pointer to zero:
	GPS_BUFFER_INDEX = 0;				//Buffer is at start position.
	
	//Clear the pre-parsing flag:
	PRE_PARSING_STATUS = 0;				//Buffer is closed off.
	
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
void GPS_parse_data(char datum){
	
	//Reset parsing counter variables:
	/* RIGHT NOW IM USING THE GLOBAL ONES */
	PARSING_PHASE[0] = 0;
	PARSING_PHASE[1] = 0;
	
	//Disable USART receiver:
	GPS_disable_stream();
	
	//***NOTE:*** May need to add an extra logical layer here like "while(GPS_BUFFER_INDEX < 255)" to allow multiple
	//cycles.
	
	uint8_t sentence_found = 0;						//Sentence detected flag.
	//Start by looking for the $ character:
	while(sentence_found == 0){
		
		//If you found the beginning of a sentence, then break:
		if(GPS_BUFFER[GPS_BUFFER_INDEX] == '$'){
						sentence_found = 1; 
						break;
												}

		//If you didn't detect the '$', then you'd better keep looking:
		GPS_BUFFER_INDEX++;
		
		//If for some reason no sentence is detected (weird), then break the loop entirely,
		//maintaining the sentence_found = 0 flag, which should cause the function to terminate
		//in a desirable fashion:
		if(GPS_BUFFER_INDEX == 255){break;}
			
	}
	
	//After searching for a sentence, depending on the outcome, you will either want to terminate
	//the function and reset the buffer entirely... Or if the sentence was detected, now you're interested
	//in determining what type of sentence was sent:
	if((sentence_found = 1)){
		
		//Normal program operation:
		char sentence_type[5];
		//Get the next five bytes from the buffer, and figure out what sentence is selected:
			//Shift into next character position:
			GPS_BUFFER_INDEX++;
			//Grab the next five bytes:
			for(int i = 0; i < 5; i++){
				sentence_type[i] = GPS_BUFFER[GPS_BUFFER_INDEX];
				GPS_BUFFER_INDEX++;
			}
			//Now figure out if the sentence matches "GPRMC":
			if((	(sentence_type[0] == 'G') &&
					(sentence_type[1] == 'P') &&
					(sentence_type[2] == 'R') &&
					(sentence_type[3] == 'M') &&
					(sentence_type[4] == 'C')		)){
						
						//At this point, the test succeeded.
						//Time to parse the following data:
						
							//1. Check and see if the remaining buffer length is long enough to
							//	support a full sentence:
							#define GPRMC_LENGTH_BYTES 75
							if((255 - GPS_BUFFER_INDEX) < GPRMC_LENGTH_BYTES){
								//Not enough space remains! The parsing will fail. Break out now!
							}
							else{
								//There's enough space, so just continue normally:
								
								//2. Invoke the void parse_GPRMC(void) function:
								// *** You may consider making a branching function for other sentence types ***
								parse_GPRMC();
							}
														}
			else{
				
				//This indicates that the sentence is not of the "GPRMC" type,
				//therefore we don't care about it (right now).
				//If we wanted to use other sentence types, then this would be
				//the go-to area to add additional screening.
	
				}
	
	} 
	
	//Two things could have happened to get to this point:
	//	1.	The IF statement was false, and it's time to bail.
	//	2.	The IF statement was true, and the program operated normally.
	//In any event, this is where the function resets the buffer, re-enables the USART receiver,
	//and terminates.
	
	//Enable receiver and flush buffer:
	GPS_enable_stream();
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	SUB-FUNCTION:	void parse_GPRMC(void);
	AUTHOR:			Christopher DeFranco
	
	DESCRIPTION:	Function begins at the first ',' of the sentence. This function assumes the
					buffer is loaded at the proper index.
	
*/
void parse_GPRMC(void){
	/* LOOK AHEAD OFFSETS */
	//These definitions specify how far away a particular segment of data is from the
	//buffer index:
	#define to_UTC 0
	#define to_Status 11	//Lands on the actual character
	#define to_Latitude 13
	#define to_NS_indicator 23
	#define to_Longitude 25
	#define to_EW_indicator 36
	#define to_Speed 38
	#define to_Course 43
	#define to_Date 50
	#define to_MagVar 57
	#define to_Mode 62
	#define to_Checksum 63	//Lands on '*'
	/* SUB-DATUM LENGTHS */
	//These are used to specify the length in bytes of a particular value that is
	//contained within a typical RMC output sentence.
	#define len_UTC 10
	#define len_Status 1
	#define len_Latitude 9
	#define len_NS 1
	#define len_Longitude 10
	#define len_EW 1
	#define len_Speed 4
	#define len_Course 6
	#define len_date 6
	#define len_MagVar 6
	#define len_Mode 1
	#define len_Checksum 3
	
	//Advance one byte in the buffer (skips over ','):
	GPS_BUFFER_INDEX++;
	
	/* EVALUATE CHECKSUM */
	//Skip ahead, and look for a checksum:
	//To do this, you must XOR every byte between '$' and '*' with each subsequent byte.
	uint8_t checksum = 0;
	for(char i = (GPS_BUFFER_INDEX - 6); i < (GPS_BUFFER_INDEX + to_Checksum); i++){
		checksum ^= i;	
	}
	//After this evaluation, check the flag, and see what happened in the emulator:
	if(checksum != 0){
		//Checksum is bad, so we want to exit.
		checksum = '!';								//Indicator for debugging.
	}
	else{
		//Checksum is good, so we can mark it as such.
		checksum = '*';								//Ditto.
	}
	
	/* BEGIN PARSING */
	//Temporary buffer, has a maximum allocated length of 10.
	char data_temp[10];
		
		//0: Obtain UTC data:
			//Get raw data:
			for(int i = 0; i < 10; i++){
				//Place 10 bytes into a temporary buffer:
				data_temp[i] = GPS_BUFFER[GPS_BUFFER_INDEX];
				GPS_BUFFER_INDEX++;
			}
			//Condition raw data:
				//HOUR:
				SYS_GPS.UTC_H = ((data_temp[0] * 10) + (data_temp[1]));
				//MINUTE:
				SYS_GPS.UTC_M = ((data_temp[2] * 10) + (data_temp[3]));
				//SECOND:
				SYS_GPS.UTC_S = ((data_temp[4] * 10) + (data_temp[5]));
				//We do not care about the point precision, although it may be added at a later date.
		//1: Obtain data valid status:
			//Grab the data:
			SYS_GPS.validity = GPS_BUFFER[GPS_BUFFER_INDEX+1];
			//Put buffer index at first character of next data segment:
			GPS_BUFFER_INDEX = GPS_BUFFER_INDEX + 3;
		//2: Obtain latitude:
			//Loop through buffer to obtain data:
			for(int i = 0; i < 9; i++){
				//Grab data:
				data_temp[i] = GPS_BUFFER[GPS_BUFFER_INDEX];
				//Increase buffer:
				GPS_BUFFER_INDEX++;			//At the end of this the next character will be the next ','
			}
			//Condition the raw data:
			SYS_GPS.latitude_H = (	((short)data_temp[0] * 1000)	+ 
									((short)data_temp[1] * 100)		+ 
									((short)data_temp[2] * 10)		+ 
									 (short)data_temp[3]);
									 //Skip over decimal point:
			SYS_GPS.latitude_L = (	((short)data_temp[5] * 1000)	+
									((short)data_temp[6] * 100)		+
									((short)data_temp[7] * 10)		+
									 (short)data_temp[8]);
		//3: Obtain NORTH/SOUTH character:
			//Grab the indicator character:
			SYS_GPS.NS = GPS_BUFFER[GPS_BUFFER_INDEX + 1];
			GPS_BUFFER_INDEX = GPS_BUFFER_INDEX + 3;		//Now you're looking at the first byte of
															//longitude.
		//4. Obtain longitude:
			//Load temporary buffer:
			for(int i = 0; i < 10; i++){
				//Grab data:
				data_temp[i] = GPS_BUFFER[GPS_BUFFER_INDEX];
				//Increment:
				GPS_BUFFER_INDEX++;
			}
			//Condition that data:
			SYS_GPS.longitude_H = (	((uint32_t)data_temp[0] * 10000)	+
									((uint32_t)data_temp[1] * 1000)		+
									((uint32_t)data_temp[2] * 100)		+
									((uint32_t)data_temp[3] * 10)		+
									 (uint32_t)data_temp[4]		);
			SYS_GPS.longitude_L = (	((uint16_t)data_temp[6] * 1000)		+
									((uint16_t)data_temp[7] * 100)		+
									((uint16_t)data_temp[8] * 10)		+
									 (uint16_t)data_temp[9]		);
		//5. Obtain EAST/WEST character:
			//Fetch the character:
			SYS_GPS.EW = GPS_BUFFER[GPS_BUFFER_INDEX+1];
			GPS_BUFFER_INDEX = GPS_BUFFER_INDEX + 3;
		//6. TO BE CONTINUED...
}
////////////////////////////////////////////////////////////////////////////////////////////////////