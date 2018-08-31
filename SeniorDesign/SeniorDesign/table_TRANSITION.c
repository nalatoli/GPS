////////////////////////////////////////////////////////////////////////////////////////////////////
//							FINITE STATE MACHINE TRANSITION TABLE								  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This document contains all of the responses, and transition structure arrays necessary to
	use a finite state machine implementation of the system.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include "header_MACROS.h"
#include "header_FUNCTIONS.h"
#define F_CPU 16000000UL
#include "util/delay.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//							SYSTEM DEFINITIONS													  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#define iENTER	0
#define iUP		1
#define iDOWN	2
#define eol		3
////////////////////////////////////////////////////////////////////////////////////////////////////
//							SUBSYSTEM VARIABLES													  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* State enumeration type */
typedef enum {
	s0,
	s1,
	s2,
	sSETTINGS,
	sOPTIONS,
	sTRACING
} state;

/* Global state register */
state present_state;

/* Task function pointer definitions */
typedef void (*task_fn_ptr) ();

/* Individual state transition structures */
typedef struct{
	unsigned char keyval;
	state next_state;
	task_fn_ptr task_pointer;
} transition;

/* System debug, default task */
void null_task(void){
	_delay_ms(1);
};

/* FSM function prototype */
void FSM(state ps, unsigned char key);

////////////////////////////////////////////////////////////////////////////////////////////////////
//							TRANSITION TABLE													  //
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition sRESET_transitions[] = {
	
	{iUP,		s0,			null_task},
	{iDOWN,		s0,			null_task},
	{iENTER,	s0,			null_task},
	{eol,		s0,			null_task}
		
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition sTITLE_transitions[] = {
	
	{iUP,		s1,			null_task},
	{iDOWN,		s1,			null_task},
	{iENTER,	s1,			null_task},
	{eol,		s1,			null_task}
		
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition sMENU_transitions[] = {
	
	{iUP,		s2,			null_task},
	{iDOWN	,	s2,			null_task},
	{iENTER,	s2,			null_task},
	{eol,		s2,			null_task}
		
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition sSETTINGS_transitions[] = {
	
	{iUP,		s2,			null_task},
	{iDOWN	,	s2,			null_task},
	{iENTER,	s2,			null_task},
	{eol,		s2,			null_task}
	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition sOPTIONS_transitions[] = {
	
	{iUP,		s2,			null_task},			//Increment user setting choice
	{iDOWN	,	s2,			null_task},			//Decrement user setting choice
	{iENTER,	s2,			null_task},			//Funnel into proper setting
	{eol,		s2,			null_task}			//Universal null task, no effect
	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition sTRACING_transitions[] = {
	
	{iENTER,	sTRACING,		null_task},
	{iUP,		sTRACING,		null_task},
	{iDOWN	,	sTRACING,		null_task},
	{eol,		sTRACING,		null_task}
	
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Pointers to subtables */
const transition *ps_transitions_ptr [6] = {
	sRESET_transitions,
	sTITLE_transitions,
	sMENU_transitions,
	sSETTINGS_transitions,
	sOPTIONS_transitions,
	sTRACING_transitions,
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//							STATE PARSING ALGORITHM						`						  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void FSM(state ps, unsigned char key){
	int i;
	for(i = 0; (ps_transitions_ptr[ps][i].keyval != key) &&
	(ps_transitions_ptr[ps][i].keyval != eol); i++);
	
	ps_transitions_ptr[ps][i].task_pointer();
	
	present_state = ps_transitions_ptr[ps][i].next_state;
}


