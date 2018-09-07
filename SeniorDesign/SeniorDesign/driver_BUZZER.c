////////////////////////////////////////////////////////////////////////////////////////////////////
//										BUZZER DRIVER											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	DESCRIPTION: Provides buzzer functions for audio feedback. The driver utilizes the Timer 1
 *				 Counter Compare Match and Peripheral Pin to produce low-overhead sounds.
 *				
 *	AUTHOR: Nor
 *	
 *	FUNCTION SET:
 *	buzzer_init(void)
 *		- Sets BUZZER_PIN as output
 *		- Configures Timer 1 for Fast-PWM
 *		- Sets initial volume of Buzzer
 *		! 'volume': {0 - 100} [%] (100% = MAX VOLUME)
 *
 *	buzzer_delay_ms (uint32_t duration)
 *		- Delays for 'duration' [ms]
 *		! 'duration': {0 - 2^16 - 1}
 *	
 *	tone (uint16_t frequency, uint16_t duration)
 *		- Generates tone at 'frequency' [Hz] for 'duration' [ms]	
 *		! Can be interrupted!
 *			- Interrupt during 'tone' will extend duration of tone for time of interrupt
 *		! 'frequency': {4 - 193798} [Hz] (F_CPU = 16 MHz, N = 64)
 *		! 'duration': {0 - 2^16 - 1} [ms]
 *
 *	tone_START (uint16_t frequency)
 *		- Starts tone generation at 'frequency' [Hz]
 *		o Can be called in succession to change frequency dynamically
 *		! 'frequency': {4 - 193798} [Hz] (F_CPU = 16 MHz, N = 64)
 *		! 'duration': {0 - 2^16 - 1} [ms]
 *
 *	tone_END (void) 
 *		- Ends tone generation
 *		o Does nothing if no tone was being generated
 *
 *	play (Note *song, uint8_t song_len)
 *		- Plays song
 *		- 'Note' array is created and passed for another function using the 'Note' struct
 *		- Note durations are cutoff by 25 ms for clarity 
 *		o Calls tone(), so is subject to its limitations	
 *		
 */
////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Libraries/Macros									      //
//////////////////////////////////////////////////////////////////////////////////////////////////// 
#include <avr/io.h>			// Standard I/O Library
#define  F_CPU 16E6			// System Clock Frequency [Hz]
#define  BUZZER_PIN 4		// Pin # of Buzzer
#include "util/delay.h"		// Contains '_delay_us' function
#include "header_BUZZER.h"	// Contains 'Note' definition
////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Function prototypes								      //
////////////////////////////////////////////////////////////////////////////////////////////////////
void init_buzzer(void);
void delay_ms(uint16_t duration);
void tone(uint16_t frequency, uint16_t duration);
void tone_START(uint16_t frequency);
void tone_END (void);
void play(Note *song, uint8_t song_len, uint8_t song_cut);

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Functions											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void init_buzzer(){	
	// Initialize Buzzer
	DDRD |= (1 << BUZZER_PIN);			// Set buzzer pin (PD4) direction
	PORTD &= ~(1 << BUZZER_PIN);		// Clear buzzer output
	TCCR1B = (1 << WGM12)|(1 << CS11);	// Enable "CTC" mode and set prescaler = 64	 	
}

// Delay
void delay_ms (uint16_t duration) {
	// Scale duration
	uint32_t count = (uint32_t)duration * 1000;	// Scale duration
		
	// Wait until 'count' is 0 
	while(count > 0) {
		 _delay_us(1);
		 count--;
	}	
}
	
//Play Tone at 'frequency' [Hz] for 'duration' [ms]
void tone (uint16_t frequency, uint16_t duration){	
	// Play tone for specified duration
	OCR1A = (F_CPU / 128) / frequency - 1;					// Set Period
	TCNT1 = 0;												// Reset Timer	
	TCCR1A |= (1 << COM1B0);								// Enable Toggle
	delay_ms(duration);										// Wait	
	TCCR1A &= ~(1 << COM1B0);								// Disable Toggle	
					
}

//Play Tone at 'frequency' [Hz]
void tone_START (uint16_t frequency){
	// Play tone for specified duration
	OCR1A = (F_CPU / 128) / frequency - 1;	// Set Period
	TCCR1A |= (1 << COM1B0);				// Enable Toggle
	
}

// Disable Toggle
void tone_END (){ TCCR1A &= ~(1 << COM1B1); }

// Play Notes in given Note array
void play(Note *song, uint8_t song_len, uint8_t song_cut){
	
	// For each note in 'song', call tone()
	for(uint8_t i = 0; i < song_len; i++){
		
		if(song[i].freq != FREQ_NO){					// If frequency is valid,
			tone(song[i].freq, song[i].dur - song_cut);	//  Call tone() with cutoff
			delay_ms(song_cut);							//  compensate cutoff
		}
				
		else { delay_ms(song[i].dur); }				// Else, simply wait for duration
	}
}





