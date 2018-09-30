////////////////////////////////////////////////////////////////////////////////////////////////////
//								LCD Libraries/Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <math.h>
#include "header_LCD.h"
#define F_CPU 16E6
#include "util/delay.h"

void LCD_spi_init(void);
void LCD_spi_send(uint8_t data);
void LCD_writecommand8(uint8_t command);
void LCD_writedata8(uint8_t data);
void LCD_pushColor(Color color);
void LCD_setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_drawChar(int16_t x, int16_t y, char c, Color color, Color bg, uint8_t size);
void LCD_printChar(char c);
void LCD_gridSmart_drawVL(uint16_t x);
void LCD_gridSmart_drawHL(uint16_t y);
void LCD_gridSmart_eraseVL(uint16_t x);
void LCD_gridSmart_eraseHL(uint16_t y);
void LCD_gridSmart_addOff(int8_t x, int8_t y);
////////////////////////////////////////////////////////////////////////////////////////////////////
//									    LCD Driver Objects									      //
////////////////////////////////////////////////////////////////////////////////////////////////////
Grid grid;
Arrow arrow;
TextHandler pencil = {0,0,0,1,WHITE,BLACK};

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   LCD Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void LCD_init_system ()
{
	/* Initialize Driver */		// ***
	LCD_spi_init();				// Initialize SPI		
	LCD_writecommand8(0x01);	// Perform Software Reset
	_delay_ms(10);				// Wait 10 ms
	LCD_writecommand8(0x28);	// Turn off Display
	
	/* Configure Power Settings */	// ***
	LCD_writecommand8(0xC0);		// Set GVDD
	LCD_writedata8(0x26);			//  to 4.75V
	LCD_writecommand8(0xC1);		// Set AVDD, VGH, and VGL
	LCD_writedata8(0x11);			//  to VCl x 2, VCl x 7, and -VCl x 4 respectively
	LCD_writecommand8(0xC5);		// Set VCOM
	LCD_writedata8(0x5C);			//  H to 5V
	LCD_writedata8(0x4C);			//  L to -0.6V
	LCD_writecommand8(0xC7);		// Set VCOM offset
	LCD_writedata8(0x94);			//  shift to +/-44
	
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
	
	/* Configure Screen Settings */	// ***
	LCD_writecommand8(0xB6);			// Set Display Function
	LCD_writedata8(0x0A);				//  Interval Scan Control to AGND
	LCD_writedata8(0x82);				//  to 0x82 (REV,GS,SS,SM,ISC[3:0])
	LCD_writedata8(0x27);				//  Number of Driving Lines to 320 lines
	LCD_writedata8(0x00);				//  PCDIV to 0 (fosc,ext = DOTCLK/(2*(0+1)))
	LCD_writecommand8(0x36);			// Set Memory Access Control
	LCD_writedata8(0x20|0x08);			//  Orientation to Landscape
	LCD_writecommand8(0x11);			// Turn Sleep Mode OFF
	_delay_ms(100);						// Wait 100 ms
	LCD_writecommand8(0x29);			// Turn Display ON
	_delay_ms(100);						// Wait 100 ms
	
	/* Initialize OID Object States */ 
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

void LCD_drawLogo(uint16_t x, uint16_t y, uint16_t size)
{
	/* Draw Logo */
	uint16_t tmpWord, tmpOff_x, tmpOff_y, tmpColor;													// Declare temporary variables
	
	for(int i = 0; i < LOGOPXCOUNT; i++) {															// For each pixel in logo,
		tmpWord = pgm_read_word(logo_BMP + i);														//  Read pixel code
		tmpOff_x = tmpWord & 0x1F;															//  Extract x offset
		tmpOff_y = (tmpWord >> 5) & 0x1F;														//  Extract y offset
		tmpColor = logo_ColorCode[(tmpWord >> 10) & 0x1F];											//  Extract color
		
		if(size == 1)																				// If size = 1,
			LCD_drawPixel(x + tmpOff_x, y + tmpOff_y, tmpColor);									//  Draw corresponding pixel 
			
		else																						// Else (size > 1),
			LCD_drawRect_filled(x + tmpOff_x * size, y + tmpOff_y * size, size, size, tmpColor);	//  Draw properly offset rectangle 
			
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
	pencil.xorigin = x;
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

void LCD_moveCursor(int16_t spaces, int16_t lines)
{
	/* Move Text Cursor */					// ***
	pencil.x += pencil.size * 6 * spaces;	// Move cursor in the x-direction
	pencil.y += (pencil.size * 8) * lines; 	// Move cursor in the y-direction
}

void LCD_clearLine()
{
	/* Clear Current Line of Text */
	LCD_drawRect_filled(pencil.x, pencil.y, TFTWIDTH - pencil.x, 8 * pencil.size, pencil.bg);
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

void LCD_init_grid (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t space, int16_t xOff, int16_t yOff, Color fg, Color bg)
{
	/* Return if Grid is Already Drawn */
	if(grid.isDrawn)
		return;
		
	/* Draw Grid (PIVOT = TOP LEFT) */			// ***
	LCD_drawRect_filled(x,y,w,h,bg);			//  Fill grid background color
	uint16_t lineIt = x + xOff;					//  Initialize line iterator to first vertical line (x-pivot-coordinate + xOff)
		
	while(lineIt < x + w){						//  While the line iterator is within the horizontal-bounds of the grid,
		LCD_drawRect_filled(lineIt,y,1,h,fg);	//   Draw a vertical line across the grid at x = lineIt and
		lineIt += space;						//   Iterate the line iterator to the next vertical line
	}
		
	lineIt = y + yOff;							//  Set line iterator to first horizontal line (y-pivot-coordinate + yOff)
		
	while(lineIt < y + h){						//  While the line iterator is within the vertical-bounds of the grid,
		LCD_drawRect_filled(x,lineIt,w,1,fg);	//   Draw a horizontal line across the grid at y = lineIt and
		lineIt += space;						//   Iterate the line iterator to the next horizontal line
	}
		
	/* Store Grid Parameters */
	grid.x = x;
	grid.y = y;
	grid.w = w;
	grid.h = h;
	grid.space = space;
	grid.xoff = xOff;
	grid.yoff = yOff;
	grid.fg = fg;
	grid.bg = bg;
	grid.isDrawn = TRUE;
	
}

void LCD_init_arrow(uint16_t x0, uint16_t y0, uint16_t rot, uint16_t fg)
{
	/* Return if Arrow is Already Drawn or Rotation is Invalid */
	if(arrow.isDrawn || rot >= 360 || rot % 5 != 0)
		return;
		
	/* Determine Arrow Parameters */								// ***
	const uint32_t * bmpPtr = &arrow_BMPs[(rot % 90) / 5][0];		// Determine specific arrow BMP to use	
	uint16_t xCor = x0 - ARROWSIZE / 2;								// Determine x-corner coordinate
	uint16_t yCor = y0 - ARROWSIZE / 2;								// Determine y-corner coordinate
		
	/* Draw Arrow (PIVOT = CENTER) */																		// ***	
	for(int y = 0; y < ARROWSIZE; y++)																		// For each row in bounds,
		for(int x = 0; x < ARROWSIZE; x++) {																//  For each column in bounds,		
			uint8_t bitState;																				//   Declare variable for bit state
					
			if(rot < 90)																					//   If 0 <= rot < 90,
				bitState = (pgm_read_dword(&bmpPtr[y]) >> (ARROWSIZE - 1 - x)) & 0x0001;					//	  Read bit at (x,y)
							
			else if(rot >= 90 && rot < 180)																	//   Else, if 90 <= rot < 180,
				bitState = (pgm_read_dword(&bmpPtr[x]) >> y) & 0x0001;										//    Read bit at (-y,x)
							
			else if(rot >= 180 && rot < 270)																//   Else, if 180 <= rot < 270
				bitState = (pgm_read_dword(&bmpPtr[ARROWSIZE - 1 - y]) >> x) & 0x0001;						//	  Read bit at (-x,-y)
						
			else                                                                                            //	 Else (270 <= rot < 360),
				bitState = (pgm_read_dword(&bmpPtr[ARROWSIZE - 1 - x]) >> (ARROWSIZE - 1 - y)) & 0x0001;	//    Read bit at (y,-x)
									
			if(bitState == 1)																				//   If bit read is '1'
				LCD_drawPixel(xCor + x, yCor + y, fg);														//    Draw a pixel at (xCor + x, y + yCor)
					
		}
		
	/* Save Arrow Parameters */
	arrow.x0 = x0;
	arrow.y0 = y0;
	arrow.rot = rot;
	arrow.color = fg;
	arrow.isDrawn = TRUE;

}

Grid LCD_get_grid(){ return grid; }
	
Arrow LCD_get_arrow(){ return arrow; }

void LCD_shiftGrid(Vector2 dir)
{
	/* Return if Grid is NOT Drawn */
	if(!grid.isDrawn)
		return;
		
	/* Shift Grid Horizontally*/																//  ***
	if(dir.x != 0){																				//  If 'dir' has horizontal component,
		Bool isReversed = (dir.x == -1);														//   Flag whether direction is positive (RIGHT) or negative (LEFT)
		uint16_t lineIt = grid.x + grid.xoff;													//   Initialize line iterator to first vertical line				
			
		while(lineIt <= grid.x + grid.w){														//   While the line iterator is within the horizontal-bounds of the grid,
				
			if ((lineIt == grid.x + grid.w && !isReversed) || (lineIt == grid.x && isReversed))	//    If the line iterator is at about to extend out of bounds,
				LCD_gridSmart_eraseVL(lineIt);													//     Smart erase vertical line at line iterator			
				
			else{																				//    Else,
				LCD_gridSmart_drawVL(lineIt + dir.x);											//     Smart draw vertical line at new location
				LCD_gridSmart_eraseVL(lineIt);													//     Smart erase old vertical line					
			}
				
			lineIt += grid.space;																//    Iterate the line iterator				
		}
				
		LCD_gridSmart_addOff(dir.x,0);														//   Update XOff
					
	}
		
	/* Shift Grid Vertically */																	//  ***
	if(dir.y != 0){																				//  If 'dir' has vertical component,
		Bool isReversed = (dir.y == -1);														//   Flag whether direction is positive (UP) or negative (DOWN)
		uint16_t lineIt = grid.y + grid.yoff;													//   Initialize line iterator to first horizontal line
			
		while(lineIt <= grid.y + grid.h){														//   While the line iterator is within the vertical-bounds of the grid,
				
			if ((lineIt == grid.y + grid.h && isReversed) || (lineIt == grid.x && !isReversed))	//    If the line iterator is at about to extend out of bounds,
				LCD_gridSmart_eraseHL(lineIt);													//     Smart erase horizontal line at line iterator
				
			else{																				//    Else,
				LCD_gridSmart_drawHL(lineIt - dir.y);;											//     Smart draw horizontal line at new location
				LCD_gridSmart_eraseHL(lineIt);													//	   Smart erase old horizontal line
						
			}
				
			lineIt += grid.space;																//    Iterate the line iterator
		}
					
		LCD_gridSmart_addOff(0,-dir.y);														//   Update YOff
			
	}		
}

void LCD_zoomGridIn(uint16_t x, uint16_t y)
{
	/* Return if Zoom-Point is NOT Within Bounds of Grid */
	if(x < grid.x || x > grid.x + grid.h || y < grid.y || y > grid.y + grid.w)
		return;
	
	/* Initialize Parameters */														// ***
	int16_t nV = ((x - grid.x + grid.space - grid.xoff) / grid.space) * -2 + 1 ;	// Initialize nV to shift magnitude+direction of first vertical line
	int16_t nH = ((y - grid.y + grid.space - grid.yoff) / grid.space) * -2 + 1;		// Initialize nH to shift magnitude+direction of first horizontal line
	
	/* Shift Vertical Lines */					// ***
	int16_t lineIt = grid.x + grid.xoff;		// Initialize lineIt to first vertical line				
	int16_t count = nV;							// Initialize count to nV			
			
	while(lineIt < grid.x + grid.w){			// While the line iterator is within the horizontal-bounds of the grid,		
		LCD_gridSmart_drawVL(lineIt + count);	//  Smart draw vertical line 'count' away from current vertical line
		LCD_gridSmart_eraseVL(lineIt);			//  Smart erase current vertical line
		lineIt += grid.space;					//  Move on to next vertical line
		count += 2;								//  Increase count by 2
		
	}
	
	/* Update Parameters */			// ***
	grid.space += 2;				// Update space
	LCD_gridSmart_addOff(nV, 0);	// Update XOff	
	
	/* Shift Horizontal Lines */				// ***
	lineIt = grid.y + grid.yoff;				// Set lineIt to first horizontal line
	count = nH;									// Set count to nH
		
	while(lineIt < grid.y + grid.h){			// While the line iterator is within the vertical-bounds of the grid,	
		LCD_gridSmart_drawHL(lineIt + count);	//  Draw horizontal line 'count' away from current horizontal line	
		LCD_gridSmart_eraseHL(lineIt);			//  Smart erase current horizontal line
		lineIt += grid.space - 2;				//  Move on to next horizontal line
		count += 2;								//  Increase count by 2
		
	}
	
	/* Update YOff */	
	LCD_gridSmart_addOff(0,nH);
	
}

void LCD_zoomGridOut(uint16_t x, uint16_t y)
{
	/* Return if Zoom-Point is NOT Within Bounds of Grid */
	if(x < grid.x || x > grid.x + grid.h || y < grid.y || y > grid.y + grid.w)
		return;
	
	/* Initialize Parameters */														// ***
	int16_t nV = ((x - grid.x + grid.space - grid.xoff) / grid.space) * 2 + 1;		// Initialize nV to shift magnitude+direction of ghost vertical line
	int16_t nH = ((y - grid.y + grid.space - grid.yoff) / grid.space) * 2 + 1;		// Initialize nH to shift magnitude+direction of ghost horizontal line
	
	/* Shift Vertical Lines */							// ***
	int16_t lineIt = grid.x + grid.xoff - grid.space;	// Initialize lineIt to ghost vertical line
	int16_t count = nV;									// Initialize count to nV
	
	while(lineIt < grid.x + grid.w){			// While the line iterator is within the horizontal-bounds of the grid,
		LCD_gridSmart_drawVL(lineIt + count);	//  Smart draw vertical line 'count' away from current vertical line
		LCD_gridSmart_eraseVL(lineIt);			//  Smart erase current vertical line
		lineIt += grid.space;					//  Move on to next vertical line
		count -= 2;								//  Increase count by 2
		_delay_ms(1000);
	}
	
	/* Update Parameters */							// ***
	int16_t ghostOff = grid.xoff - grid.space + nV;	// Calculate y-coordinate of shifted ghost line
	
	if(ghostOff > 0)								// If ghost offset is within grid bounds,
		grid.xoff = ghostOff;						//  Set xoff to ghost offset
		
	else{											// Else,
		grid.space -= 2;							//  Update space
		LCD_gridSmart_addOff(nV > 1 ? (nV - 2): -1, 0);	//  Update XOff
	}
		
	/* Shift Horizontal Lines */				// ***
	lineIt = grid.y + grid.yoff - grid.space;	// Set lineIt to first horizontal line
	count = nH;									// Set count to nH
	
	while(lineIt < grid.y + grid.h){			// While the line iterator is within the vertical-bounds of the grid,
		LCD_gridSmart_drawHL(lineIt + count);	//  Draw horizontal line 'count' away from current horizontal line
		LCD_gridSmart_eraseHL(lineIt);			//  Smart erase current horizontal line
		lineIt += grid.space;				//  Move on to next horizontal line
		count -= 2;								//  Increase count by 2
		_delay_ms(1000);
	}
	
	///* Update YOff */								// ***
	//ghostOff = grid.yoff - grid.space + nH;			// Calculate y-coordinate of shifted ghost line
		//
	//if(ghostOff > 0)								// If ghost offset is within grid bounds,
	//grid.yoff = ghostOff;							//  Set xoff to ghost offset
		//
	//else{											// Else,
		//LCD_gridSmart_addOff(0, nH > 1 ? nV : -1);	//  Update YOff
	//}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   LCD Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void LCD_spi_init()
{
	/* Set SPI Speed and Settings */	
	SPDDR |= (1<<1)|(1<<4)|(1<<5)|(1<<7);	// Set CS(1), D/C(4), MOSI(5), and SCK(7) as outputs
	SPCR = (1<<SPE)|(1<<MSTR);				// Enable SPI - fclk/4
	SPPORT |= (1<<CS);						// Disable CS during startup
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
	LCD_spi_send(command);			// Send command
	SPPORT |= (1<<CS);					// Disable chip select
}


void LCD_writedata8(uint8_t data)
{
	/*Write 8-bit Data */
	SPPORT |=(1<<DC);		// Set data-mode
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
		pencil.x = pencil.xorigin;												//  Move pencil cursor back to xoriging
	}
	
	else {																		// Else, 
		LCD_drawChar(pencil.x, pencil.y, c, pencil.fg, pencil.bg, pencil.size);	// Draw character using pencil
		pencil.x += pencil.size * 6;											// Move pencil cursor 6 px to the right
	}
}

void LCD_gridSmart_drawVL(uint16_t x)
{
	if(!grid.isDrawn || x < grid.x || x >= grid.x + grid.w)
		return;
		
	LCD_drawRect_filled(x, grid.y, 1, grid.h, grid.fg);
}

void LCD_gridSmart_drawHL(uint16_t y)
{
	if(!grid.isDrawn || y < grid.y || y >= grid.y + grid.h)
		return;
	
	LCD_drawRect_filled(grid.x, y, grid.w, 1, grid.fg);
}

void LCD_gridSmart_eraseVL(uint16_t x)
{
	if(!grid.isDrawn || x < grid.x || x >= grid.x + grid.w)
		return;
		
	LCD_setAddress(x, grid.y, x, grid.y + grid.h);
	
	for(int i = 0; i < grid.h; i++)
		LCD_pushColor(i % grid.space != grid.yoff ? grid.bg : grid.fg);
		
}

void LCD_gridSmart_eraseHL(uint16_t y)
{
	LCD_setAddress(grid.x, y, grid.x + grid.w, y);
	
	for(int i = 0; i < grid.w; i++)
		LCD_pushColor(i % grid.space != grid.xoff ? grid.bg : grid.fg);
	
}

void LCD_gridSmart_addOff(int8_t x, int8_t y)
{
	if(!grid.isDrawn)
		return;
		
	if(x != 0){
		grid.xoff += x;
	
		while(grid.xoff < 0)
			grid.xoff += grid.space;
			
		while(grid.xoff > grid.space)
			grid.xoff -= grid.space;
		
	}
	
	if(y != 0){
		grid.yoff += y;
			
		while(grid.yoff < 0)
			grid.yoff += grid.space;
		
		while(grid.yoff > grid.space)
			grid.yoff -= grid.space;
			
	}		
}