////////////////////////////////////////////////////////////////////////////////////////////////////
//									      ILI9341 DRIVER										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	DESCRIPTION: Provides all-in-one drawing library for 240 x 320 LCD driven by the IL9341.
 *
 *	Public Functions:
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////
//									Libraries/Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <math.h>
#define F_CPU 16E6
#include "util/delay.h"
#include "header_LCD.h"
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void LCD_spi_init(void);
void LCD_spi_send(uint8_t data);
void LCD_writecommand8(uint8_t command);
void LCD_writedata8(uint8_t data);
void LCD_pushColor(Color color);
void LCD_setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_drawChar(int16_t x, int16_t y, char c, Color color, Color bg, uint8_t size);
void LCD_printChar(char c);
////////////////////////////////////////////////////////////////////////////////////////////////////
//											Macros												  //
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct{
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
	int16_t xoff;
	int16_t yoff;
	uint16_t space;
	Color color;
	Bool isDrawn;
} Grid;

typedef struct{
	uint16_t x0;
	uint16_t y0;
	int16_t rot;
	Color color;
	Bool isDrawn;
} Arrow;

typedef struct{
	uint16_t x;
	uint16_t y;
	uint8_t size;
	Color fg;
	Color bg;
} TextHandler;

Grid grid;
Arrow arrow;
TextHandler pencil = {0,0,1,WHITE,BLACK};

//volatile uint16_t textColor_fg;
//volatile uint16_t textColor_bg;
//volatile uint16_t textCursor_x;
//volatile uint16_t textCursor_y;
//volatile uint8_t textFPrecision;
//volatile uint8_t textSize;
////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void LCD_init_system ()
{
	/* Initialize Driver */		// ***
	LCD_spi_init();				// Initialize SPI		
	LCD_writecommand8(0x01);	// Perform Software Reset
	_delay_ms(10);				// Wait 10 ms
	LCD_writecommand8(0x28);	// Turn off Display
	
	/* Configure Power Settings */	// ***
	LCD_writecommand8(0xC0);		// Set VRH
	LCD_writedata8(0x26);			//  to 4.75V
	LCD_writecommand8(0xC1);		// Set AVDD, VGH, and VGL
	LCD_writedata8(0x11);			//  to VCl x 2, VCl x 7, and -VCl x 4 respectively
	LCD_writecommand8(0xC5);		// Set VCOM-
	LCD_writedata8(0x5C);			//  H to 5V
	LCD_writedata8(0x4C);			//  L to -0.6V
	LCD_writecommand8(0xC7);		// Set VCOM offset
	LCD_writedata8(0x94);			//  to level 44
	
	/* Configure Memory Settings */	// ***
	LCD_writecommand8(0x36);		// Set Memory Access Control
	LCD_writedata8(0x48);			//  to 0x48 (MY,MX,MV,ML,BGR,MH,0,0)
	LCD_writecommand8(0x3A);		// Set Pixel Format
	LCD_writedata8(0x55);			//  to 16bits/pixel (MCU Interface)
	
	/* Configure Frame Rate */		// ***
	LCD_writecommand8(0xB1);		// Set Normal Mode Frame Rate
	LCD_writedata8(0x00);			//  Division Ratio to 1 (fosc/1)
	LCD_writedata8(0x18);			//  Frame Rate to 70 Hz (default)
	
	/* Configure Gamma Control */	// *** 
	LCD_writecommand8(0xF2);	
	LCD_writedata8(0x08);	
	LCD_writecommand8(0x26);		// Select Gamma Curve
	LCD_writedata8(0x01);			//  number 1 (G2.2)
	LCD_writecommand8(0xE0);		/* Set Positive Gamma Correction */
	 LCD_writedata8(0x1F);
	 LCD_writedata8(0x1A);
	 LCD_writedata8(0x18);
	 LCD_writedata8(0x0A);
	 LCD_writedata8(0x0F);
	 LCD_writedata8(0x06);
	 LCD_writedata8(0x45);
	 LCD_writedata8(0x87);
	 LCD_writedata8(0x32);
	 LCD_writedata8(0x0A);
	 LCD_writedata8(0x07);
	 LCD_writedata8(0x02);
	 LCD_writedata8(0x07);
	 LCD_writedata8(0x05);
	 LCD_writedata8(0x00);
	LCD_writecommand8(0xE1);		/* Set Negative Gamma Correction */
	 LCD_writedata8(0x00);
	 LCD_writedata8(0x25);
	 LCD_writedata8(0x27);
	 LCD_writedata8(0x05);
	 LCD_writedata8(0x10);
	 LCD_writedata8(0x09);
	 LCD_writedata8(0x3A);
	 LCD_writedata8(0x78);
	 LCD_writedata8(0x4D);
	 LCD_writedata8(0x05);
	 LCD_writedata8(0x18);
	 LCD_writedata8(0x0D);
	 LCD_writedata8(0x38);
	 LCD_writedata8(0x3A);
	 LCD_writedata8(0x1F);
	
	/* Configure DDRAM Settings */	// ***
	LCD_writecommand8(0x2A);		// Set Column Address
	LCD_writedata8(0x00);			//  SCH to 0
	LCD_writedata8(0x00);			//  SCL to 0    (Start Column = 0)
	LCD_writedata8(0x00);			//  ECH to 0
	LCD_writedata8(0xEF);			//  ECL to 0xEF (End Column = 239)
	LCD_writecommand8(0x2B);		// Set Page Address
	LCD_writedata8(0x00);			//  SPH to 0
	LCD_writedata8(0x00);			//  SPL to 0    (Start Page = 0)
	LCD_writedata8(0x01);			//  EPH to 0x01
	LCD_writedata8(0x3F);			//	EPL to 0x3F (End Page = 319) 
	LCD_writecommand8(0xB7);		// Set Entry Mode Control
	LCD_writedata8(0x07);			//  to 0x07 (0,0,0,0,DSTB,GON,DTE,GAS)
	
	/* Configure Display Settings */	// ***
	LCD_writecommand8(0xB6);			// Set Display Function
	LCD_writedata8(0x0A);				//  Interval Scan Control to AGND
	LCD_writedata8(0x82);				//  to 0x82 (REV,GS,SS,SM,ISC[3:0])
	LCD_writedata8(0x27);				//  Number of Driving Lines to 320 lines
	LCD_writedata8(0x00);				//  PCDIV to 0 (fosc,ext = DOTCLK/(2*(0+1)))
	LCD_writecommand8(0x11);			// Turn Sleep Mode OFF
	_delay_ms(100);						// Wait 100 ms
	LCD_writecommand8(0x29);			// Turn Display ON
	_delay_ms(100);						// Wait 100 ms
	LCD_writecommand8(0x2C);			// Perform memory write
	LCD_writecommand8(0x36);			// Set Memory Access Control
	LCD_writedata8(0x20|0x08);			// Set Landscape // (0x20|0x08);
	grid.isDrawn = FALSE;				// Mark grid as UNDRAWN
	arrow.isDrawn = FALSE;				// Mark arrow as UNDRAWN
	
}


void LCD_drawPixel (uint16_t x, uint16_t y, Color color)
{
	/* Draw Pixel at (x,y) */	// ***
	LCD_setAddress(x, y, x, y);	// Select 1px-by-1px drawing zone at (x,y) (one pixel)
	LCD_pushColor(color);		// Send color data
}


void LCD_drawRect_filled (uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color)
{
	/* Draw Filled Rectangle (PIVOT = UPPERLEFT) */	// ***	
	LCD_setAddress(x, y, x+w-1, y+h-1);				// Select 'w'[px]-by-'h'[px] drawing zone from (x,y) to (x+w-1,y+h-1)
	
	for(y = 0; y < h; y++)							// For all rows,
		for(x = 0; x < w; x++)						//  For all columns,
			LCD_pushColor(color);					//	 Send color data
 
}


void LCD_drawRect_empty (uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color)
{
	/* Draw Empty Rectangle (PIVOT = UPPERLEFT) */	// ***
	LCD_drawRect_filled(x,y,w,1,color);				// Draw top border
	LCD_drawRect_filled(x,y+h-1,w,1,color);			// Draw bottom border
	LCD_drawRect_filled(x,y,1,h,color);				// Draw left border
	LCD_drawRect_filled(x+w-1,y,1,h,color);			// Draw right border
}


void LCD_drawCircle_filled(uint16_t x0, uint16_t y0, uint8_t radius, Color color)
{
	/* Draw Empty Circle (PIVOT = CENTER) */		// ***
	uint16_t rSquared = radius*radius;				// Calculate radius^2
	
	for (int y = -radius; y <= radius; y++)			// For all rows,
		for (int x = -radius; x <= radius; x++){	//  For all columns
			uint16_t dSquared = x*x + y*y;			//   Calculate distance^2 from pivot-coordinates using row and column iterations
		
			if (dSquared <= rSquared + radius)		//   If distance^2 from center is equal to or less than radius^2 + radius
				LCD_drawPixel(x0+x, y0+y, color);	//    Draw Pixel at point at displaced coordinates
		
		}
}


void LCD_drawCircle_empty(uint16_t x0, uint16_t y0, uint8_t radius, Color color)
{
	/* Draw Filled Circle (PIVOT = CENTER) */									// ***
	uint16_t rSquared = radius*radius;											// Calculate radius^2
	
	for (int y = -radius; y <= radius; y++)										// For all rows,
		for (int x = -radius; x <= radius; x++){								//  For all columns
			uint16_t dSquared = x*x + y*y;										//   Calculate distance^2 from pivot-coordinates using row and column iterations
			
			if (dSquared >= rSquared - radius && dSquared <= rSquared + radius)	//   If distance^2 from center is within threshold of radius^2 +/- radius
				LCD_drawPixel(x0+x, y0+y, color);								//    Draw Pixel at point at displaced coordinates
				
		}
}

void LCD_clear (Color color)
{
	/* Fill Screen with Color */						// ***
	LCD_drawRect_filled(0,0,TFTWIDTH,TFTHEIGHT,color);	// Draw screen-sized rectangle
}

void LCD_setText_cursor(uint16_t x, uint16_t y)
{
	/* Set cursor at desired location to print data */
	pencil.x = x;
	pencil.y = y;
}

void LCD_setText_size(uint8_t size)
{
	/* Set text size to 's' if between 1 and 8, otherwise 1 by default */
	pencil.size = (size > 0 && size <= 8) ? size : 1;
}

void LCD_setText_color(Color fg, Color bg)
{
	/* Set Text Foreground/Background Colors */
	pencil.fg = fg;
	pencil.bg = bg;
}

void LCD_setText_all(uint16_t x, uint16_t y,uint8_t size, Color fg, Color bg)
{
	/* Set all text parameters */
	LCD_setText_cursor(x,y);
	LCD_setText_size(size);
	LCD_setText_color(fg,bg);
}

void LCD_moveTextCursor(int16_t spaces, int16_t lines)
{
	/* Move Text Cursor */					// ***
	pencil.x += pencil.size * 6 * spaces;	// Move cursor in the x-direction
	pencil.y += pencil.size * 6 * lines;	// Move cursor in the y-direction
}

void LCD_print_str(char * str)
{
	/* Print all letters of 'str' */			// ***
	for(uint8_t i = 0; i < strlen(str); i++)	// For each letter in 'str'
		LCD_printChar(str[i]);					//  Print letter
}

void LCD_print_num(float num, uint8_t width, uint8_t prec)
{
	/* Print Number with given parameters */	// ***
	char str[20];								// Declare storage for new string
	dtostrf(num,width,prec,str);				// Generate string from 'num'
	LCD_print_str(str);							// Print string
}

void LCD_init_grid (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t space, int16_t xOff, int16_t yOff, Color color)
{
	/* Check if grid is already drawn */				// ***
	if(!grid.isDrawn)
	{
		/* Draw Grid (PIVOT = TOP LEFT) */				//  ***
		uint16_t lineIt = x + xOff;						//  Initialize line iterator to first vertical line (x-pivot-coordinate + xOff)
		
		while(lineIt < x + w){							//  While the line iterator is within the vertical-bounds of the grid,
			LCD_drawRect_filled(lineIt,y,1,h,color);	//   Draw a vertical line across the grid at x = lineIt and
			lineIt += space;							//   Iterate the line iterator to the next vertical line
		}
		
		lineIt = y + yOff;								//  Set line iterator to first horizontal line (y-pivot-coordinate + yOff)
		
		while(lineIt < y + h){							//  While the line iterator is within the horizontal-bounds of the grid,
			LCD_drawRect_filled(x,lineIt,w,1,color);	//   Draw a horizontal line across the grid at y = lineIt and
			lineIt += space;							//   Iterate the line iterator to the next horizontal line
		}
		
		/* Store Grid Parameters */
		grid.x = x;
		grid.y = y;
		grid.w = w;
		grid.h = h;
		grid.space = space;
		grid.xoff = xOff;
		grid.yoff = yOff;
		grid.color = color;
		grid.isDrawn = TRUE;
	}
}

void LCD_init_arrow(uint16_t x0, uint16_t y0, uint16_t rot, uint16_t fg)
{
	/* Check if arrow is already drawn */					// ***
	if(!arrow.isDrawn)
	{
		/* Determine Arrow Parameters */
		const uint32_t * bmpPtr = &arrow_BMP[(rot % 90) / 5][0];	
		uint8_t isMirrored = (rot >= 180);								// Determine whether BMP should be mirrored
		uint8_t isRotated = ((rot >= 90 && rot < 180) || (rot >= 270));	// Determine whether BMP should be rotated 90 degrees
		uint16_t x = x0 - ARROWSIZE / 2;								// Determine x-corner coordinate
		uint16_t y = y0 - ARROWSIZE / 2;								// Determine y-corner coordinate
		
		/* Draw Arrow (PIVOT = CENTER) */	
		for(int bmp_row = 0; bmp_row < ARROWSIZE; bmp_row++)
			for(int bmp_col = 0; bmp_col < ARROWSIZE; bmp_col++) {		
				uint8_t currArrowPixel;
					
				if(rot < 90)
					currArrowPixel = (pgm_read_dword(&bmpPtr[bmp_row]) >> (ARROWSIZE - 1 - bmp_col)) & 0x0001;	// regular
							
				else if(rot >= 90 && rot < 180)
					currArrowPixel = (pgm_read_dword(&bmpPtr[bmp_col]) >> (bmp_row)) & 0x0001;			// 90
							
				else if(rot >= 180 && rot < 270)
					currArrowPixel = (pgm_read_dword(&bmpPtr[ARROWSIZE - 1 - bmp_row]) >> (bmp_col)) & 0x0001;
						
				else if(rot >= 270 && rot < 360)
					currArrowPixel = (pgm_read_dword(&bmpPtr[ARROWSIZE - 1 - bmp_col]) >> (ARROWSIZE - 1 - bmp_row)) & 0x0001;
									
				if(currArrowPixel == 1)
					LCD_drawPixel(x + bmp_col, y + bmp_row, fg);
					
			}
		
		/* Save Arrow Parameters */
		arrow.x0 = x0;
		arrow.y0 = y0;
		arrow.rot = rot;
		arrow.color = fg;
		arrow.isDrawn = TRUE;
	}		
}

//bool LCD_shiftGrid (Vector2 dir)
//{
	//if(dir.x > 1 || dir.x < -1 || dir.y > 1 || dir.y < -1) return false;	// Return failure if a direction is invalid
	///* Initialize Parameters */
	//uint16_t colorCode[2] = {oldGrid.bg,oldGrid.fg};		// Color code for drawing color
	//uint8_t isReversed = (dir.x == -1) || (dir.y == -1);	// Flag for reversing drawing color
	//uint16_t horizLim = TFTWIDTH /oldGrid.space + 1;		// Number of possible horizontal lines
	//uint16_t vertLim = TFTHEIGHT/oldGrid.space + 1;			// Number of possible vertical lines
	//uint16_t offset_it;										// Grid Line Iterator
	//
	///* If shift has horizontal component */
	//if(dir.x != 0) {
		//offset_it = oldGrid.offset.x;															// Set iterator to first horizontal line
		//
		//for(int j = 0; j < horizLim; j++) {														// For all possible horizontal lines,
			//if(offset_it + dir.x < TFTWIDTH) { 													//  If line's pending offset is OB,
				//LCD_setAddress(offset_it - isReversed, 0, offset_it + !isReversed, TFTHEIGHT);	//   Set drawing bounds to changing columns			
				//for(int i = 0; i < TFTHEIGHT * 2; i++) {										//   For each pixel in bounds,
					//LCD_pushColor(colorCode[(i + isReversed) % 2]);								//    Draw colors based on direction
				//}
			//}
			//
			//else																				// Else (line's pending offset is TFTWIDTH or -1),
				//LCD_drawLine_vertical(offset_it, 0, TFTHEIGHT, oldGrid.bg); 					//  Simply erase line near bound
//
			//for(int i = 0; i < vertLim; i++)													// For all overwritten vertical lines,
				//LCD_drawPixel(offset_it, oldGrid.offset.y + i * oldGrid.space, oldGrid.fg);		//  Fill in overwritten pixels
			//
			////if(offset_it >= ARROWCORNERX && offset_it < ARROWCORNERX + ARROWSIZE){
				////LCD_drawArrow(oldArrow.rot, oldArrow.fg);
			////}
			//
			//offset_it += oldGrid.space;															// Iterate offset
		//}
		//
		///* Update Grid-X-Offset */
		//oldGrid.offset.x = (oldGrid.offset.x + dir.x) % oldGrid.space;	// Save X-Offset
		//if(oldGrid.offset.x < 0)										// If the new X-Offset is negative (-1),
			//oldGrid.offset.x = oldGrid.space - 1;						//  Set new X-Offset to the space - 1 
	//}
	//
	///* If shift has vertical component */
	//if(dir.y != 0) {
		//offset_it = oldGrid.offset.y;															// Set iterator to first vertical line
		//
		//for(int j = 0; j < vertLim; j++) {														// For all possible vertical lines
			//if(offset_it + dir.y < TFTHEIGHT) 													//  If line's pending offset is NOT OB,
				//for(int i = 0; i < TFTWIDTH; i++) {												//	 For all columns on LCD,
					//LCD_setAddress(i, offset_it - isReversed, i, offset_it + !isReversed);		//    Select 1px-by-2px drawing zone on changing column
					//LCD_pushColor(colorCode[isReversed]);										//	  Send primary color data
					//LCD_pushColor(colorCode[!isReversed]);										//	  Send secondary color data
				//}
			//
			//else																				// Else (line's pending offset is TFTWIDTH or -1),
				//LCD_drawLine_horizontal(0, offset_it, TFTWIDTH, oldGrid.bg);					//  Simply erase line near bound
//
			//for(int i = 0; i < horizLim; i++)													// For all overwritten vertical lines,
				//LCD_drawPixel(oldGrid.offset.x + i * oldGrid.space, offset_it, oldGrid.fg);		//  Fill in overwritten pixels
			//
			//offset_it += oldGrid.space;															// Iterate offset
		//}
		//
		///* Update Grid-Y-Offset */
		//oldGrid.offset.y = (oldGrid.offset.y + dir.y) % oldGrid.space;	// Save the Y-Offset
		//if(oldGrid.offset.y < 0)										// If the new Y-Offset is negative (-1),
			//oldGrid.offset.y = oldGrid.space - 1;						//  Set the new Y-Offset to space - 1
	//}
	//
	//return true;	// Return success
//}
//


























////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Private Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void LCD_spi_init()
{
	/* Set SPI Speed and Settings */	
	SPDDR |= (1<<1)|(1<<3)|(1<<4)|(1<<5)|(1<<7);	// Set CS(1), RST(3), D/C(4), MOSI(5), and SCK(7) as outputs
	SPCR = (1<<SPE)|(1<<MSTR)|(SPR0);				// Enable SPI - fclk/16
	SPPORT |= (1<<CS);								// Disable CS during startup
}

void LCD_spi_send(uint8_t data)
{
	/* Send SPI data */
	SPDR = data ;					// Load data into SPDR - initiates transmission
	while(!(SPSR & (1<<SPIF))) ;	// Wait till the transmission is finished
}


void LCD_writecommand8(uint8_t command)
{
	/* Write 8-bit Command */
	SPPORT &= ~((1<<DC)|(1<<CS));		// Enable chip select and set command-mode
	_delay_us(5);						// Wait
	LCD_spi_send(command);			// Send command
	SPPORT |= (1<<CS);					// Disable chip select
}


void LCD_writedata8(uint8_t data)
{
	/*Write 8-bit Data */
	SPPORT |=(1<<DC);		// Set data-mode
	_delay_us(1);			// Wait
	SPPORT &= ~(1<<CS);		// Enable chip select
	LCD_spi_send(data);	// Send data
	SPPORT |=(1<<CS);		// Disable chip select
}

void LCD_pushColor(uint16_t color)
{
	LCD_writedata8(color>>8);
	LCD_writedata8(color);
}

void LCD_setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	/* Set column address */
	LCD_writecommand8(0x2A);	// Send "Column-Address Set" command
	LCD_writedata8(x1>>8);		// Set start
	LCD_writedata8(x1);			// address to x1
	LCD_writedata8(x2>>8);		// Set end
	LCD_writedata8(x2);			// address to x2
	
	/* Set page address */
	LCD_writecommand8(0x2B);	// Send "Page-Address Set" command
	LCD_writedata8(y1>>8);		// Set start
	LCD_writedata8(y1);			// address to y1
	LCD_writedata8(y2>>8);		// Set end
	LCD_writedata8(y2);			// address to y2

	/* Update Frame Data */
	LCD_writecommand8(0x2C);	// Perform memory write
}

void LCD_drawChar(int16_t x, int16_t y, char c, Color fg, Color bg, uint8_t size)
{	
	/* Draw a Character with Given Text Parameters */									// ***
	for (int8_t line = 0; line < 6; line++){											// For every line of character 'c'
		uint8_t pattern;																//  Declare bit pattern variable
		
		if (line < 5)																	//  If the last line is NOT reached,
			pattern = pgm_read_byte(font + c*5 + line);									//   Load bit pattern at designated line  

		else																			//  Else,
			pattern = 0x0;																//   Load an empty bit pattern

		for (uint8_t bitNum = 0; bitNum < 8; bitNum++) {								//  For each bit in current pattern,
			Color bitColor = pattern & 0x01 ? fg : bg;									//   Record color depending on state of current bit in pattern
			
			if(size == 1)																//   If size of text = 1,
				LCD_drawPixel(x+line, y+bitNum, bitColor);								//	  Draw pixel using recorded bit color     
					
			else                                                                        //	 Else,
				LCD_drawRect_filled(x+line*size, y+bitNum*size, size, size, bitColor);	//	  Draw rectangle using recorded bit color
				
			pattern >>= 1;																//   Advance to next bit in pattern
		}	
	}
}

void LCD_printChar(char c)
{
	/* Draw 'c' */																// ***
	if (c == '\n') {															// If the character is '\n'
		pencil.y += pencil.size * 8;											//  Move pencil cursor down and 
		pencil.x = 0;															//  Move pencil cursor all the way to the left
	}
	
	else {																		// Else, 
		LCD_drawChar(pencil.x, pencil.y, c, pencil.fg, pencil.bg, pencil.size);	// Draw character using pencil
		pencil.x += pencil.size * 6;											// Move pencil cursor 6 px to the right
	}
}