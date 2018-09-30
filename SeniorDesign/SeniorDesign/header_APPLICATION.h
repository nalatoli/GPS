////////////////////////////////////////////////////////////////////////////////////////////////////
//									     Application Header										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef HEADER_APPLICATION_H
#define HEADER_APPLICATION_H
////////////////////////////////////////////////////////////////////////////////////////////////////
//								Application Data Structures and Libraries						  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "header_LCD.h"
#include "header_FUNCTIONS.h"
#include <avr/io.h>

typedef enum {LOADINGMENU,MAINMENU,SETTINGSMENU,DEBUGMENU} MenuType;
typedef enum {TIME,DATE,STATUS,LATITUDE,LONGITUDE,N_S,E_W,SPEED,COURSE} GPSDataType;
	 
////////////////////////////////////////////////////////////////////////////////////////////////////
//								    Application Public Functions								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

void APP_initSystems();
void APP_loadProgram();
Bool APP_generate_menu(MenuType menu);
void APP_update_menu_debug_updateGPSParameter(GPSDataType type);
void APP_update_menu_debug_updateAll();
void APP_update_menu_loading_showStatusString(char * str);

////////////////////////////////////////////////////////////////////////////////////////////////////
//									       LCD Public MACROS									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Set Loading Screen Parameters */
#define LOADSCREEN_LOGO_XOFF 0
#define LOADSCREEN_LOGO_YOFF -30
#define LOADSCREEN_TEXT_XOFF -7
#define LOADSCREEN_TEXT_YOFF 10
#define LOADSCREEN_TEXT_SIZE 2
#define LOADSCREEN_TEXT_COLOR GREEN
#define LOADSCREEN_SCREENCOLOR GREY
#define LOADSCREEN_STATUSTEXT_SIZE 2
#define LOADSCREEN_STATUSTEXT_COLOR WHITE
#define LOADSCREEN_STATUSTEXT_XOFF -7
#define LOADSCREEN_STATUSTEXT_YOFF (10 + LOADSCREEN_TEXT_SIZE)

/* Set Debug Screen Parameters */
#define DEBUGSCREEN_SCREENCOLOR BLACK
#define DEBUGSCREEN_IDENTIFIER_SIZE 3
#define DEBUGSCREEN_IDENTIFIER_XOFF 10
#define DEBUGSCREEN_IDENTIFIER_YOFF 10
#define DEBUGSCREEN_IDENTIFIER_COLOR GREEN
#define DEBUGSCREEN_BORDEROFF 2
#define DEBUGSCREEN_TEXT_SIZE 2
#define DEBUGSCREEN_TEXT_COLOR ORANGE
#define DEBUGSCREEN_START_X (DEBUGSCREEN_IDENTIFIER_XOFF + (12 * 6 * DEBUGSCREEN_TEXT_SIZE))
#define DEBUGSCREEN_START_Y (DEBUGSCREEN_IDENTIFIER_YOFF + (16 * DEBUGSCREEN_IDENTIFIER_SIZE))

/* Set Main Screen Parameters */
#define MAINSCREEN_SCREENCOLOR BLACK
#define MAINSCREEN_IDENTIFIER_SIZE 3
#define MAINSCREEN_IDENTIFIER_XOFF 10
#define MAINSCREEN_IDENTIFIER_YOFF 10
#define MAINSCREEN_IDENTIFIER_COLOR WHITE
#define MAINSCREEN_BORDEROFF 2
#define MAINSCREEN_TEXT_SIZE 2
#define MAINSCREEN_TEXT_COLOR WHITE

#endif