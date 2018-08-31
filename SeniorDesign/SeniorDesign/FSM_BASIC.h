////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	TITLE:			Finite State Machine Basic
 *	AUTHOR:			Christopher DeFranco
 *
 *	DESCRIPTION:	The following software provides access to FSM-based solutions
 *					in an Embedded-C program.
 */ 
////////////////////////////////////////////////////////////////////////////////////////////////////
//							INPUT DEFINITIONS													  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#define i0		0
#define i1		1
#define i2		2
#define eol		3
////////////////////////////////////////////////////////////////////////////////////////////////////
//							SUBSYSTEM VARIABLES													  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* State enumeration type */
typedef enum {
	s0,
	s1,
	s2,
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
	unsigned char dummy = 0;
	dummy++;
};

/* FSM function prototype */
void FSM(state ps, unsigned char key);

////////////////////////////////////////////////////////////////////////////////////////////////////
//							TRANSITION TABLE													  //
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition[] s0_transitions = {
	
	{i0,		s0,			null_task},
	{i1,		s0,			null_task},
	{i2,		s0,			null_task},
	{eol,		s0,			null_task}
		
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition[] s1_transitions = {
	
	{iUP,		s1,			null_task},
	{iDOWN,		s1,			null_task},
	{iENTER,	s1,			null_task},
	{eol,		s1,			null_task}
		
};
////////////////////////////////////////////////////////////////////////////////////////////////////
const transition[] s2_transitions = {
	
	{iUP,		s2,			null_task},
	{iDOWN	,	s2,			null_task},
	{iENTER,	s2,			null_task},
	{eol,		s2,			null_task}
		
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
	
	ps_transitions_ptr[ps][i].tf_ptr();
	
	present_state = ps_transitions_ptr[ps][i].next_state;
}


