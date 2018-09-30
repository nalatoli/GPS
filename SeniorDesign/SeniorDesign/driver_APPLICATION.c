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

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Macros												  //
////////////////////////////////////////////////////////////////////////////////////////////////////


void APP_generate_menu_loading();
void APP_generate_menu_debug();
////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

Bool APP_generate_menu(MenuType menu)
{
	///* Generate 'menu' and return generation status*/
	switch(menu){
		case LOADINGMENU:
		APP_generate_menu_loading();
		return TRUE;
		
		case MAINMENU:
		return TRUE;
		
		case SETTINGSMENU:
		return TRUE;
		
		case DEBUGMENU:
		APP_generate_menu_debug();
		return TRUE;
	}
	return FALSE;
}

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
	/* Set Loading Screen Parameters */
	int16_t logo_xoff = 0;
	int16_t logo_yoff = -30;
	int16_t text_xoff = -7;
	int16_t text_yoff = 10;
	uint8_t text_size = 2;
	Color text_color = GREEN;
	Color screenColor = GREY;
		
	/* Set Logo Pivot Coordinates and Text Parameters */
	uint16_t x = TFTWIDTH / 2 - LOGOSIZE * (text_size + 2) / 2 + logo_xoff;
	uint16_t y = TFTHEIGHT / 2 - LOGOSIZE * (text_size + 2) / 2 + logo_yoff;
	LCD_setText_all(x + text_xoff, y + LOGOSIZE * (text_size + 2) + text_yoff, text_size, text_color, screenColor);
		
	/* Draw Loading Screen */
	LCD_clear(screenColor);
	LCD_drawLogo(x,y,text_size + 2);
	LCD_print_str("Power Couple");
	LCD_setText_size(text_size - 1);
	LCD_print_str("TM");
	//LCD_drawRect_empty(0,0,TFTWIDTH,TFTHEIGHT,RED);
	//LCD_drawRect_empty(1,1,TFTWIDTH-2,TFTHEIGHT-2,WHITE);
	//LCD_drawRect_empty(2,2,TFTWIDTH-4,TFTHEIGHT-4,RED);
}

void APP_generate_menu_debug()
{
	/* Set Debug Screen Parameters */
	Color screenColor = BLACK;
	uint16_t identifier_size = 3;
	uint16_t identifier_xoff = 10;
	uint16_t identifier_yoff = 10;
	Color identifier_color = GREEN;
	uint16_t text_size = 2;
	uint16_t text_yoff = 40;
	Color text_color = ORANGE;
	
	LCD_setText_all(identifier_xoff, identifier_yoff, identifier_size,identifier_color,screenColor);
	LCD_clear(screenColor);
	
	
	LCD_drawRect_empty(identifier_xoff - 2, identifier_yoff - 2, strlen("DEBUG") * 6 * identifier_size + 4, 8 * identifier_size + 4, identifier_color);
	LCD_print_str("DEBUG\n\n");
	
	LCD_setText_all(identifier_xoff, identifier_yoff + text_yoff, text_size, text_color, screenColor);
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
		
		

