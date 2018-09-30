////////////////////////////////////////////////////////////////////////////////////////////////////
//									     Application Header										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef HEADER_APPLICATION_H
#define HEADER_APPLICATION_H
////////////////////////////////////////////////////////////////////////////////////////////////////
//								Application Data Structures and Libraries						  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "header_LCD.h"
#include <avr/io.h>

typedef enum  {LOADINGMENU,MAINMENU,SETTINGSMENU,DEBUGMENU} MenuType;
	 
////////////////////////////////////////////////////////////////////////////////////////////////////
//								    Application Public Functions								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

	 
Bool APP_generate_menu(MenuType menu);	 
#endif