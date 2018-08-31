#include <avr/io.h>
#define F_CPU 16E6
#include "util/delay.h"
#include "header_LCD.h"
#include "header_BUZZER.h"
#include <string.h>





int main(void) {

	LCD_init_system();	// Init LCD System
	LCD_clear(BLUE);	// Clear entire screen
	
	LCD_init_grid(20,20,200,200,23,10,10,YELLOW);
	LCD_init_arrow(150,150,355,RED);
	


	return 0;
}


