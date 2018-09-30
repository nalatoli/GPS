/* Full Application Software Driver */

/*		
	Required Functions:
	
	- void [Initialize APP] ()
		- Initializes all systems
	
	- bool [Display Menu] ([menu identifier])
		- Displays user-options on LCD, along with a default cursor. Text is application defined, 
		  but aesthetics (color, font size, brightness, contrast, etc) can be user-configured 
		  (by navigating to proper menu). Each menu is stored by application memory.
			- bool - returns true if menu has successfully displayed
			- menu identifier - application defined menu to display
		  
	- bool [Move Menu Cursor] ([direction])
		- Move menu cursor across menu options.
			- bool - returns true if cursor has moved
			- direction - direction in which cursor traverses
			
	- bool [Start trace] ()
		- Enters "Trace Mode", displaying corresponding initial screen and enabling dynamic
		  drawing updates (global flag in application driver)
			- bool - returns true if trace mode is entered successfully
		
	- bool [End Trace] ()
		- "Trace Mode". The screen is locked in place, until another display command changes screen.
			- bool - returns true if trace mode is exited successfully	
		
*/


////////////////////////////////////////////////////////////////////////////////////////////////////
//									     Application DRIVER										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////
//									Libraries/Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "header_APPLICATION.h"
#include <string.h>

void APP_generate_menu_loading();
void APP_generate_menu_main();
void APP_generate_menu_settings();
void APP_generate_menu_debug();
////////////////////////////////////////////////////////////////////////////////////////////////////
//											Macros												  //
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

void APP_initSystems()
{
	/* Initialize All Modules */
	// init GPS
	LCD_init_system();
	init_buzzer();
	// init SD Card
	// Enable EEPROM
}

//void APP_loadProgram()
//{
	///* Start Up */
	//APP_generate_menu_main();
	////APP_update_menu_loading_printStatusString();
	//
//}

Bool APP_generate_menu(MenuType menu)
{
	///* Generate 'menu' and return generation status*/
	switch(menu){
		case LOADINGMENU:
		APP_generate_menu_loading();
		return TRUE;
		
		case MAINMENU:
		APP_generate_menu_main();
		return TRUE;
		
		case SETTINGSMENU:
		APP_generate_menu_settings();
		return TRUE;
		
		case DEBUGMENU:
		APP_generate_menu_debug();
		return TRUE;
	}
	return FALSE;
}

void APP_update_menu_debug_updateGPSParameter(GPSDataType type)
{
	/* Display Updated Parameter on Debug Menu */
	LCD_setText_all(DEBUGSCREEN_START_X,DEBUGSCREEN_START_Y,DEBUGSCREEN_TEXT_SIZE,DEBUGSCREEN_TEXT_COLOR,DEBUGSCREEN_SCREENCOLOR);
	LCD_moveCursor(0,type);
	LCD_clearLine();
	
	switch(type) {
		case TIME:
			for(int i = 0; i < GPS_BYTES_ASCII_UTC_TIME; i++){

				LCD_printChar(SYS_GPS.UTC_TIME_ASCII[i]);
				if(i%2==1)
					LCD_printChar(':');

			}
			
			return;	
				
		case DATE:
			for(int i = 0; i < GPS_BYTES_ASCII_UTC_DATE; i++){
				LCD_printChar(SYS_GPS.UTC_DATE_ASCII[i]);
				if(i%2==1)
					LCD_printChar('/');
					
			}
			
			return;
		
		case STATUS:
			LCD_printChar(SYS_GPS.STATUS);
			return;
			
		case LATITUDE:
			for(int i = 0; i < GPS_BYTES_ASCII_LATITUDE; i++){
				LCD_printChar(SYS_GPS.LATITUDE_ASCII[i]);
			}
			
			return;

		case LONGITUDE:
			for(int i = 0; i < GPS_BYTES_ASCII_LONGITUDE; i++){
				LCD_printChar(SYS_GPS.LONGITUDE_ASCII[i]);
			}
		
			return;
			
		case N_S:
			LCD_printChar(SYS_GPS.NS);
			return;
			
		case E_W:
			LCD_printChar(SYS_GPS.EW);
			return;
			
		case SPEED:
			for(int i = 0; i < GPS_BYTES_ASCII_SPEED; i++){
				LCD_printChar(SYS_GPS.SPEED_ASCII[i]);
			}
			
			return;
			
		case COURSE:
			for(int i = 0; i < GPS_BYTES_ASCII_COURSE; i++){
				LCD_printChar(SYS_GPS.COURSE_ASCII[i]);
			}
			
			return;
		}	
}

void APP_update_menu_debug_updateAll()
{
	for(int i = 0; i < 9; i++){
		APP_update_menu_debug_updateGPSParameter(i);
	}
}

//void APP_update_menu_loading_printStatusString(char * str)
//{
	///* Display Status String on Debug Menu */
	//LCD_setText_all(LOADSCREEN_STATUSTEXT_XOFF,LOADSCREEN_STATUSTEXT_YOFF,LOADSCREEN_STATUSTEXT_SIZE,LOADSCREEN_STATUSTEXT_COLOR,LOADSCREEN_SCREENCOLOR);
	//LCD_clearLine();
	//LCD_print_str(str);
	//
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Private Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void APP_generate_menu_main()
{
	/* Draw Menu Identifier */
}

void APP_generate_menu_settings()
{	
}

void APP_generate_menu_loading()
{
	/* Set Logo Pivot Coordinates and Text Parameters */
	uint16_t x = TFTWIDTH / 2 - LOGOSIZE * (LOADSCREEN_TEXT_SIZE + 2) / 2 + LOADSCREEN_LOGO_XOFF;
	uint16_t y = TFTHEIGHT / 2 - LOGOSIZE * (LOADSCREEN_TEXT_SIZE + 2) / 2 + LOADSCREEN_LOGO_YOFF;
	LCD_setText_all(x + LOADSCREEN_TEXT_XOFF, y + LOGOSIZE * (LOADSCREEN_TEXT_SIZE + 2) + LOADSCREEN_TEXT_YOFF, LOADSCREEN_TEXT_SIZE, LOADSCREEN_TEXT_COLOR, LOADSCREEN_SCREENCOLOR);
		
	/* Draw Loading Screen */
	LCD_clear(LOADSCREEN_SCREENCOLOR);
	LCD_drawLogo(x,y,LOADSCREEN_TEXT_SIZE + 2);
	LCD_print_str("Power Couple");
	LCD_setText_size(LOADSCREEN_TEXT_SIZE - 1);
	LCD_print_str("TM");
}

void APP_generate_menu_debug()
{
	/* Set Text Parameters */
	LCD_setText_all(DEBUGSCREEN_IDENTIFIER_XOFF, DEBUGSCREEN_IDENTIFIER_YOFF, DEBUGSCREEN_IDENTIFIER_SIZE,DEBUGSCREEN_IDENTIFIER_COLOR,DEBUGSCREEN_SCREENCOLOR);
	
	/* Print Identifier with Border */
	LCD_clear(DEBUGSCREEN_SCREENCOLOR);
	LCD_drawRect_empty(DEBUGSCREEN_IDENTIFIER_XOFF - DEBUGSCREEN_BORDEROFF, DEBUGSCREEN_IDENTIFIER_YOFF - DEBUGSCREEN_BORDEROFF, strlen("DEBUG") * 6 * DEBUGSCREEN_IDENTIFIER_SIZE + DEBUGSCREEN_BORDEROFF * 2, 8 * DEBUGSCREEN_IDENTIFIER_SIZE + DEBUGSCREEN_BORDEROFF * 2, DEBUGSCREEN_IDENTIFIER_COLOR);
	LCD_print_str("DEBUG\n\n");
	
	/* Print Debug Parameters */
	LCD_setText_size(DEBUGSCREEN_TEXT_SIZE);
	LCD_setText_color(DEBUGSCREEN_TEXT_COLOR,DEBUGSCREEN_SCREENCOLOR);
	LCD_print_str("Time (UTC) :\n");
	LCD_print_str("Date       :\n");
	LCD_print_str("Data Status:\n");
	LCD_print_str("Latitude   :\n");
	LCD_print_str("Longitude  :\n");
	LCD_print_str("N/S        :\n");
	LCD_print_str("E/W        :\n");
	LCD_print_str("Speed      :\n");
	LCD_print_str("Course     :\n");
}
		
		

