#include <avr/io.h>
#define F_CPU 16E6
#include "util/delay.h"
#include "header_LCD.h"
#include "header_BUZZER.h"
#include <string.h>





int main(void)
{	
	///* Initialize Zoom-Test Parameters */
	//Vector2 cursorStart = {0,170};	// Initialize text cursor start position
	//char * strXOff  = "XOff  = ";	// Initialize string for "XOff  = "
	//char * strYOff  = "YOff  = ";	// Initialize string for "YOff  = "
	//char * strSpace = "Space = ";	// Initialize string for "Space = "
	//Vector2 debugPoint = {100,55};	// Initialize debugPoint coordinates
		//
	///* Initialize LCD */
	//LCD_init_system();													// Init LCD System
	//LCD_clear(GREENYELLOW);												// Clear entire screen
	//LCD_init_grid(5,5,160,160,60,10,10,BLACK,WHITE);					// Draw Grid
	//LCD_drawCircle_filled(debugPoint.x,debugPoint.y,3,RED);				// Draw zoom-point debug circle
	//LCD_setText_all(cursorStart.x,cursorStart.y,2,BLACK, GREENYELLOW);	// Set Text Parameters
	//
	///* Print Debug Information */
	//LCD_print_str(strXOff);								// Print "XOff  = "
	//LCD_print_num(LCD_get_grid().xoff,3,0);				// Print initial xoff
	//LCD_moveTextCursor(-(strlen(strXOff) + 3), 1);		// Move text cursor below 'X' in "XOff  = "
	//LCD_print_str(strYOff);								// Print "YOff  = "
	//LCD_print_num(LCD_get_grid().yoff,3,0);				// Print initial yoff
	//LCD_moveTextCursor(-(strlen(strYOff) + 3), 1);		// Move text cursor below 'Y' in :YOff  = "
	//LCD_print_str(strSpace);							// Print "Space = "
	//LCD_print_num(LCD_get_grid().space,3,0);				// Print initial space
	//LCD_setText_cursor(cursorStart.x,cursorStart.y);	// Move text cursor to 'X' in "XOff  = "
	//LCD_moveTextCursor(strlen(strXOff), 0);				// Move text cursor 2 spaces after '=' in "XOff  = "
//
	//DDRB |= 0x02;
//
	//while(1){
		//while(PINB & (1 << 2));
			///* Perform Zoom Test */	
			//_delay_ms(100);										// Wait 1s								
			//LCD_zoomGridOut(debugPoint.x, debugPoint.y);				// Perform zoom w.r.t. zoom-point
////			LCD_shiftGrid(RIGHT);
			//LCD_drawCircle_filled(debugPoint.x,debugPoint.y,3,RED);	// Draw zoom-point debug circle
			//LCD_print_num(LCD_get_grid().xoff,3,0);					// Print new xoff
			//LCD_moveTextCursor(-3,1);								// Setup cursor to print new yoff
			//LCD_print_num(LCD_get_grid().yoff,3,0);					// Print new yoff
			//LCD_moveTextCursor(-3,1);								// Setup cursor to print new space
			//LCD_print_num(LCD_get_grid().space,3,0);				// Print new space
			//LCD_moveTextCursor(-3,-2);								// Setup cursor to print new xoff
		//
	//}
	
	
	LCD_init_system();
	
	LCD_setText_all(10,10,3,YELLOW,BLACK);
	LCD_clear(GREEN);
	
	LCD_init_grid(10,10,200,200,20,15,15,WHITE,BLACK);
	
	Vector2 point = {150,150};
		
	while(1){
		LCD_drawCircle_filled(point.x, point.y, 5, RED);
		LCD_shiftGrid(RIGHT);
//		_delay_ms(100);
		
	}
	
	
	
	
	return 0;
}




