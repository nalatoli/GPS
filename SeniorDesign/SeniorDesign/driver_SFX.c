#include "header_SFX.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//									     SFX Driver Object									      //
////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t iTONETIME;
////////////////////////////////////////////////////////////////////////////////////////////////////
//									    SFX Public Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void SFX_init()
{	
	/* Initialize SFX */				// ***
	DDRD |= (1 << BUZZER_PIN);			// Set buzzer pin (PD4) direction
	OCR2B = 1000 / 8;					// Timer 2: CTC Period = 1 ms
	TCCR2A = (1<<WGM21);				// Timer 2: Mode = "CTC"
	TCCR2B = (1<<CS22);					// Timer 2: N = 64
	TCCR1B = (1<<WGM12)|(1<<CS10);		// Timer 1: N = 1, Mode = "CTC"
	ASSR = ~(1<<AS2);					// Explicitly disable asynchronous Timer 2 source	
}

void SFX_tone_start (uint16_t freq)
{
	/* Play Tone Indefinitely */				// ***
	OCR1A = F_CPU / 2 / freq -1;				// Set frequency
	TCCR1A |= (1<<COM1B0);						// Enable buzzer
}

void SFX_tone_end ()
{
	/* Stop Any Tones (Diable Buzzer) */
	TCCR1A &= ~(1<<COM1B0);
}

void SFX_tone (uint16_t freq, uint16_t dur)
{	
	/* Play Tone */	// ***	
	SFX_tone_start(freq);				// Start tone
	SFX_delay(dur);						// Delay
	SFX_tone_end();						// End tone			
}

void SFX_tone_i (uint16_t freq, uint16_t dur)
{
	/* Play INTERRUPTIBLE Tone */	// ***
	iTONETIME = dur;				// Set global itone duration
	TCNT2 = 0;						// Clear Timer 2
	TIFR2 |= (1<<OCF2B);				// Clear Timer 2 interrupt flag
	TIMSK2 |= (1<<OCIE2B);			// Enable Timer 2 interrupt
	SFX_tone_start(freq);			// Start tone
}

void SFX_play(Note *song, uint16_t n)
{
	
}

void SFX_delay(uint16_t dur)
{
	// Wait until 'dur' is 0
	while(dur-- > 0)
	_delay_ms(1);
}

void SFX_toggle_enabled(){	

	/* Changes Port Direction Of Buzzer Pin */
	_delay_ms(100);
	uint8_t state = DDRD & (1<<BUZZER_PIN);
	if(state){	for(int f = 400; f > 200; f -= 40) SFX_tone(f,40); DDRD &= ~(1 << BUZZER_PIN);}
	else{		DDRD |= (1 << BUZZER_PIN);	for(int f = 200; f < 400; f += 40) SFX_tone(f,40);  }
}


