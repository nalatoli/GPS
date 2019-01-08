#include "header_KEYPAD.h"
#include "header_APPLICATION.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Keypad Driver Objects									      //
////////////////////////////////////////////////////////////////////////////////////////////////////
int globalOption = 0;

/* Do (Basically) Nothing */
void null_tsk(){SFX_tone(100,200);}

/****************** MODIFIABLE ******************/

const Options optionsMAIN[] = {
	{APP_startMode_debug,	"Navigation Data"	},
	{APP_startMode_trace,	"Trace Mode"		},
	{SFX_toggle_enabled,	"Toggle Buzzer"		}
};

const Options optionsDEBUG[] = {
	{null_tsk,				"Save Coordinate"	},
	{APP_startMode_main,	"Exit"				}
};

const Options optionsTRACE[] = {
//	{null_tsk,				"Start"				},
	{null_tsk,				"Sleep"				},
	{null_tsk,				"Recover"			},
	{null_tsk,				"GPS"				},
	{APP_startMode_main,	"Exit"				}
};

/*************************************************/

////////////////////////////////////////////////////////////////////////////////////////////////////
//									 Keypad Public Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
	
void KEY_init()
{
	/* Configure INT Sense and Pin I/0 */
	DDRB &= ~(1<<2);							// Set PB2(INT2) as input
	DDRD &= ~((1<<2)|(1<<3));					// Set PD2(INT0) and PD3(INT1) as inputs
	PORTB |= (1<<2);
	PORTD |= (1<<2)|(1<<3);
	
	/* Set Sense Bits to Falling Edge */
	EICRA = (1<<ISC21)|(1<<ISC11)|(1<<ISC01);
}

void KEY_setState(uint8_t state)
{
	/* Update Interrupts: */
	EIFR = 0xFF;
	switch(state){
		case 0: EIMSK &= ~((1<<INT2)|(1<<INT1)|(1<<INT0));	return;
		case 1:	EIMSK |= (1<<INT2)|(1<<INT1)|(1<<INT0);		return;
	}
}

void KEY_execute()
{
	/* Execute Task Based on Screen and Option # */
	EIFR = 0xFF;
	sei();
	switch(screen){
		case MAINSCREEN:	optionsMAIN[globalOption].task();	return;
		case DEBUGSCREEN:	optionsDEBUG[globalOption].task();	return;
		case TRACESCREEN:	optionsTRACE[globalOption].task();	return;
	}
}

void KEY_scroll(int key)
{
	/* Set # of Options */
	EIFR = 0xFF;
	int maxOptions;
	switch(screen){
		case MAINSCREEN:	maxOptions = OPTION_LENGTH_MAIN;	break;
		case DEBUGSCREEN:	maxOptions = OPTION_LENGTH_NAV;		break;
		case TRACESCREEN:	maxOptions = OPTION_LENGTH_TRACE;	break;
		default: return;
	}
	
	/* Update Option # and Move Cursor */
	globalOption += key;
	if(globalOption >= maxOptions)	globalOption = 0;
	else if(globalOption < 0)		globalOption = maxOptions - 1;
	LCD_moveScreenCursor(globalOption);
}
