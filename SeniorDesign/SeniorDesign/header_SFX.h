////////////////////////////////////////////////////////////////////////////////////////////////////
//											SFX HEADER											  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef HEADER_SFX_H
#define HEADER_SFX_H
////////////////////////////////////////////////////////////////////////////////////////////////////
//										   SFX Libraries										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#define F_CPU 8E6
#include "util/delay.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   SFX Type Definitions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
	Type Definition: Note (Data Structure)
	Description:
		Defines frequency and duration of a note.
		
			freq: frequency of note
			dur:  duration of note
			
		A 'Note' object can be used standalone or an array of them can be passed to the 'play'.
		driver function for a song to play.
		
***************************************************************************************************/
typedef struct{
	uint16_t freq;	// Frequency of note
	uint16_t dur;	// Duration of note
	
} Note;

////////////////////////////////////////////////////////////////////////////////////////////////////
//										SFX Public Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
	Function: init
		- Initializes Sound FX System. The processes executed are:
		
			1. Set Pin Direction of Buzzer
			2. Clear Buzzer Pin
			3. Enables Timers 0/1/2 CTC Modes
			
***************************************************************************************************/
void SFX_init ();

/***************************************************************************************************
	Function: tone_start
		- Begins playing sound with 'freq' frequency [Hz]
		- USE 'tone_end' function to end tone
			
***************************************************************************************************/
void SFX_tone_start (uint16_t freq);

/***************************************************************************************************
	Function: tone_end
		- Stops playing sound started by 'tone_start'
		- Does NOTHING if no 'tone_start' was executed beforehand
			
***************************************************************************************************/
void SFX_tone_end ();

/***************************************************************************************************
	Function: tone
		- Plays sound with 'freq' frequency [Hz] and 'dur' duration [ms]
		- This tone is NOT INTERRUPTABLE!
			
***************************************************************************************************/
void SFX_tone (uint16_t freq, uint16_t dur);

/***************************************************************************************************
	Function: tone_i
		! GLOBAL INTERRUPTS MUST BE ENABLED MANUALLY
		- Plays sound with 'freq' frequency [Hz] and 'dur' duration [ms].
		- This tone is INTERRUPTABLE! As long as Timer 1/2 is unaffected, tone will play
		  as normal while further code is executed.
			
***************************************************************************************************/
void SFX_tone_i (uint16_t freq, uint16_t dur);

/***************************************************************************************************
	Function: play (NOT COMPLETE)
		! GLOBAL INTERRUPTS MUST BE ENABLED MANUALLY
		- Plays the first 'n' notes in 'song' (a user-defined 'Note' array)
		- The notes, as well as the succession of notes, is INTERRUPTABLE
			
***************************************************************************************************/
void SFX_play(Note * song, uint16_t n);

/***************************************************************************************************
	Function: delay
		- Waits 'dur' before executing further code
		
***************************************************************************************************/
void SFX_delay(uint16_t dur);

/***************************************************************************************************
	Function: toggle_enabled
		- Toggles buzzer enable
		
***************************************************************************************************/
void SFX_toggle_enabled();

////////////////////////////////////////////////////////////////////////////////////////////////////
//										 SFX Public MACROS										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Control MACROS */
extern uint16_t iTONETIME;
#define BUZZER_PIN 4
#define TIMER1_N 1
#define TIMER2_NE6 64E6 	

/* Musical Pitch Frequencies */
#define FREQ_NO   0	// No sound will play for this frequency
#define FREQ_B0  31
#define FREQ_C1  33
#define FREQ_CS1 35
#define FREQ_D1  37
#define FREQ_DS1 39
#define FREQ_E1  41
#define FREQ_F1  44
#define FREQ_FS1 46
#define FREQ_G1  49
#define FREQ_GS1 52
#define FREQ_A1  55
#define FREQ_AS1 58
#define FREQ_B1  62
#define FREQ_C2  65
#define FREQ_CS2 69
#define FREQ_D2  73
#define FREQ_DS2 78
#define FREQ_E2  82
#define FREQ_F2  87
#define FREQ_FS2 93
#define FREQ_G2  98
#define FREQ_GS2 104
#define FREQ_A2  110
#define FREQ_AS2 117
#define FREQ_B2  123
#define FREQ_C3  131
#define FREQ_CS3 139
#define FREQ_D3  147
#define FREQ_DS3 156
#define FREQ_E3  165
#define FREQ_F3  175
#define FREQ_FS3 185
#define FREQ_G3  196
#define FREQ_GS3 208
#define FREQ_A3  220
#define FREQ_AS3 233
#define FREQ_B3  247
#define FREQ_C4  262
#define FREQ_CS4 277
#define FREQ_D4  294
#define FREQ_DS4 311
#define FREQ_E4  330
#define FREQ_F4  349
#define FREQ_FS4 370
#define FREQ_G4  392
#define FREQ_GS4 415
#define FREQ_A4  440
#define FREQ_AS4 466
#define FREQ_B4  494
#define FREQ_C5  523
#define FREQ_CS5 554
#define FREQ_D5  587
#define FREQ_DS5 622
#define FREQ_E5  659
#define FREQ_F5  698
#define FREQ_FS5 740
#define FREQ_G5  784
#define FREQ_GS5 831
#define FREQ_A5  880
#define FREQ_AS5 932
#define FREQ_B5  988
#define FREQ_C6  1047
#define FREQ_CS6 1109
#define FREQ_D6  1175
#define FREQ_DS6 1245
#define FREQ_E6  1319
#define FREQ_F6  1397
#define FREQ_FS6 1480
#define FREQ_G6  1568
#define FREQ_GS6 1661
#define FREQ_A6  1760
#define FREQ_AS6 1865
#define FREQ_B6  1976
#define FREQ_C7  2093
#define FREQ_CS7 2217
#define FREQ_D7  2349
#define FREQ_DS7 2489
#define FREQ_E7  2637
#define FREQ_F7  2794
#define FREQ_FS7 2960
#define FREQ_G7  3136
#define FREQ_GS7 3322
#define FREQ_A7  3520
#define FREQ_AS7 3729
#define FREQ_B7  3951
#define FREQ_C8  4186
#define FREQ_CS8 4435
#define FREQ_D8  4699
#define FREQ_DS8 4978

#endif 