#include "header_LCD.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//										LCD Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void LCD_spi_init();
void LCD_spi_send(uint8_t data);
void LCD_writecommand8(uint8_t command);
void LCD_writedata8(uint8_t data);
void LCD_pushColor(Color color);
void LCD_setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_drawChar(int16_t x, int16_t y, char c, Color color, Color bg, uint8_t size);

////////////////////////////////////////////////////////////////////////////////////////////////////
//									    LCD Driver Objects									      //
////////////////////////////////////////////////////////////////////////////////////////////////////
TextHandler pencil = {0,0,0,1,WHITE,BLACK};
 ScreenType screen;
	
////////////////////////////////////////////////////////////////////////////////////////////////////
//									   LCD Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void LCD_init()
{	
	/* Initialize Driver */		// ***
	LCD_spi_init();				// Initialize SPI and perform hard reset		
	LCD_writecommand8(0x01);	// Perform soft Reset
//	_delay_ms(10);				// Wait 10 ms
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
	LCD_writedata8(0xE0|0x08);			//  Orientation to Landscape
	LCD_writecommand8(0x11);			// Turn Sleep Mode OFF
	_delay_ms(100);						// Wait 100 ms
	LCD_writecommand8(0x29);			// Turn Display ON
	_delay_ms(100);						// Wait 100 ms
	
	/* Initialize OID Object States */ 
	//grid.isDrawn = FALSE;				// Mark grid as UNDRAWN	
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
	LCD_setAddress(x, y, x+w-1, y+h-1);				// Select (x,y) to (x+w-1,y+h-1) drawing zone
	
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
	if(radius == 1) { LCD_drawPixel(x0,y0,color); return; }
	
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
	if(radius == 1) { LCD_drawPixel(x0,y0,color); return; }
	
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
	/* Declare Temporary Variables */
	uint16_t tmpWord, tmpOff_x, tmpOff_y, tmpColor;
	
	/* For Each Coordinate in BMP */
	for(int i = 0; i < LOGOPXCOUNT; i++)
	{
		/* Fill in Parameters Based on Code in BMP */						// ***
		tmpWord = pgm_read_word(logo_BMP + i);								// Read pixel code
		tmpOff_x = tmpWord & 0x1F;											// Extract x offset
		tmpOff_y = (tmpWord >> 5) & 0x1F;									// Extract y offset
		tmpColor = pgm_read_word(&logo_ColorCode[(tmpWord >> 10) & 0x1F]);	// Extract color
		
		/* Draw Pixel */
		if(size == 1)	LCD_drawPixel(x + tmpOff_x, y + tmpOff_y, tmpColor);
		else			LCD_drawRect_filled(x + tmpOff_x * size, y + tmpOff_y * size, size, size, tmpColor);
	}	
}

void LCD_drawArrow(uint16_t x, uint16_t y, int16_t rot, Color fg, Color bg)
{
	/* Return For Invalid Rotation */
	if(rot > 360) return;
	
	/* Adjust Rotation */
	while(rot < 0) rot += 360;
	
	/* Determine Correct Arrow BMP */
	const uint32_t * bmpPtr = &arrow_BMPs[(rot % 90) / 10][0];
	
	/* For Each Coordinate */
	for(int r = 0; r < ARROWSIZE; r++)
	for(int c = 0; c < ARROWSIZE; c++) 
	{
		/* Obtain The State of the Corresponding Pixel */
		uint8_t bitState;
		if     (rot < 90)	bitState = (pgm_read_dword(&bmpPtr[r]) >> (ARROWSIZE - 1 - c)) & 0x0001;	
		else if(rot < 180)	bitState = (pgm_read_dword(&bmpPtr[c]) >> r) & 0x0001;		
		else if(rot < 270)	bitState = (pgm_read_dword(&bmpPtr[ARROWSIZE - 1 - r]) >> c) & 0x0001;	
		else				bitState = (pgm_read_dword(&bmpPtr[ARROWSIZE - 1 - c]) >> (ARROWSIZE - 1 - r)) & 0x0001;
		
		/* Draw Pixel If State is High, Draw Pixel */
		LCD_drawPixel(x + c, y + r, bitState ? fg : bg);
	}
}

void LCD_drawImage(OutlineImage type, uint16_t x, uint16_t y, uint8_t size, Color color)
{	
	/* Declare Temporary Parameters */
	const uint8_t * bmpPtr;
	uint16_t h,w;
	
	/* Fill in Parameters Based on Image */
	switch(type){
		case CARDICON: bmpPtr = cardBMP;	h = CARDBMP_H;	w = CARDBMP_W / 8;	break;
		case GPSICON:  bmpPtr = gpsBMP;		h = GPSBMP_H;	w = GPSBMP_W / 8;	break;
		default: return;
	}
	
	/* For Each Row in BMP */
	for(int row = 0; row < h; row++)
	{
		/* For Each Byte in Row of BMP */
		for(int group = 0; group < w; group++)
		{
			/* Get Byte */
			uint8_t code = pgm_read_byte(bmpPtr);
			
			/* For Each Bit in Byte */
			for(int bit = 0; bit < 8; bit++)
			{
				/* If Bit is High */
				if((code >> (7-bit)) & 1)
				{
					/* Draw Pixel at Corresponding Coordinate */
					 if (size == 1)	LCD_drawPixel(x+group*8+bit,y+row,color);
					 else			LCD_drawRect_filled(x+(group*8+bit)*size,y+row*size,size,size,color);
				}
			}
			
			/* Increment to Next Byte */
			bmpPtr++;
		}
	}
}

void LCD_clearScreen_down (Color color)
{
	/* Clear Screen Downwards */
	LCD_drawRect_filled(0,0,TFTWIDTH,TFTHEIGHT,color);
}

void LCD_clearScreen_out (Color color)
{
	/* Clear Screen Outwards */
	for(int i = 0; i <= TFTHEIGHT / 2; i++)
		LCD_drawRect_empty(TFTHEIGHT/2-i, TFTHEIGHT/2-i, TFTWIDTH-TFTHEIGHT+2*i,2*i, color);
}

void LCD_clearScreen_in (Color color)
{	
	/* Clear Screen Inwards */
	for(int i = 0; i < TFTHEIGHT / 2; i++)
		LCD_drawRect_empty(i, i, TFTWIDTH-2*i,TFTHEIGHT-2*i, color);
}


void LCD_setText(uint16_t x, uint16_t y,uint8_t size, Color fg, Color bg)
{
	/* Set Cursor At Desired Location To Print Data */
	pencil.xorigin = x;
	pencil.x = x;
	pencil.y = y;
	
	/* Set text size to 's' if between 1 and 8, otherwise 1 by default */
	pencil.size = (size > 0 && size <= 8) ? size : 1;
	
	/* Set Text Foreground/Background Colors */
	pencil.fg = fg;
	pencil.bg = bg;
}

void LCD_moveCursor(int16_t spaces, int16_t lines)
{
	/* Move Text Cursor */					// ***
	pencil.x += pencil.size * 6 * spaces;	// Move cursor in the x-direction
	pencil.y += (pencil.size * 8) * lines; 	// Move cursor in the y-direction
}

void LCD_print_char(char c)
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

void LCD_print_str(char * str)
{
	/* Print all letters of 'str' */			// ***
	for(uint8_t i = 0; i < strlen(str); i++)	// For each letter in 'str'
		LCD_print_char(str[i]);					//  Print letter
}

void LCD_println_str(char * str)
{
	/* Print 'str' with newline */
	LCD_print_str(str);
	LCD_print_char('\n');
}

void LCD_print_str_len(char * str, uint8_t len)
{
	/* Print 'len' letters of 'str' */	// ***
	for(uint8_t i = 0; i < len; i++)	// For each letter in 'str'
		LCD_print_char(str[i]);			//  Print letter
}

void LCD_println_str_len(char * str, uint8_t len)
{
	/* Print 'str' with len with newline */
	LCD_print_str_len(str,len);
	LCD_print_char('\n');
}

void LCD_print_int(int num)
{
	/* Print 'num' */	// ***
	char str[6];		//  Declare storage for string
	itoa(num,str,10);	// Generate string from 'num'
	LCD_print_str(str);	// Print string
}

void LCD_println_int(int num)
{
	/* Print 'num' with newline */
	LCD_print_int(num);
	LCD_print_char('\n');
}

void LCD_generateScreen(ScreenType type)
{
	switch(type) {
		case MAINSCREEN:
		/* Print Header */
		LCD_clearScreen_in(MAINSCREEN_SCREENCOLOR);
		LCD_drawLogo(ALLSCREENS_LOGO_X,ALLSCREENS_LOGO_Y,ALLSCREENS_LOGO_SIZE);
		LCD_setIconState(CARDICON,0);
		LCD_setIconState(GPSICON,0);
		LCD_setText(MAINSCREEN_IDENTIFIER_XOFF, MAINSCREEN_IDENTIFIER_YOFF, MAINSCREEN_IDENTIFIER_SIZE,MAINSCREEN_IDENTIFIER_COLOR,MAINSCREEN_SCREENCOLOR);
		LCD_drawRect_empty(MAINSCREEN_IDENTIFIER_XOFF - MAINSCREEN_BORDEROFF, MAINSCREEN_IDENTIFIER_YOFF - MAINSCREEN_BORDEROFF, strlen("MAIN") * 6 * MAINSCREEN_IDENTIFIER_SIZE + MAINSCREEN_BORDEROFF * 2, 8 * MAINSCREEN_IDENTIFIER_SIZE + MAINSCREEN_BORDEROFF * 2, MAINSCREEN_IDENTIFIER_COLOR);
		LCD_print_str("MAIN\n\n");
		/* Print Options */
		LCD_setText(MAINSCREEN_OPTION_X,MAINSCREEN_OPTION_Y,MAINSCREEN_OPTION_SIZE,MAINSCREEN_OPTION_COLOR,MAINSCREEN_SCREENCOLOR);
	
		for(int i = 0; i < MAINSCREEN_OPTION_COUNT; i++){
			LCD_print_str("   ");	LCD_println_str(optionsMAIN[i].label);
		}
		pencil.x = MAINSCREEN_OPTION_X; pencil.y = MAINSCREEN_OPTION_Y;
		LCD_print_char('>');
		
		break;
		
		case DEBUGSCREEN:
		/* Print Header */
		LCD_clearScreen_in(DEBUGSCREEN_SCREENCOLOR);
		LCD_drawLogo(ALLSCREENS_LOGO_X,ALLSCREENS_LOGO_Y,ALLSCREENS_LOGO_SIZE);
		LCD_setIconState(CARDICON,0);
		LCD_setIconState(GPSICON,0);
		LCD_setText(DEBUGSCREEN_IDENTIFIER_XOFF, DEBUGSCREEN_IDENTIFIER_YOFF, DEBUGSCREEN_IDENTIFIER_SIZE,DEBUGSCREEN_IDENTIFIER_COLOR,DEBUGSCREEN_SCREENCOLOR);
		LCD_drawRect_empty(DEBUGSCREEN_IDENTIFIER_XOFF - DEBUGSCREEN_BORDEROFF, DEBUGSCREEN_IDENTIFIER_YOFF - DEBUGSCREEN_BORDEROFF, strlen("DEBUG") * 6 * DEBUGSCREEN_IDENTIFIER_SIZE + DEBUGSCREEN_BORDEROFF * 2, 8 * DEBUGSCREEN_IDENTIFIER_SIZE + DEBUGSCREEN_BORDEROFF * 2, DEBUGSCREEN_IDENTIFIER_COLOR);
		LCD_print_str("DEBUG\n\n");
		/* Print GPS Parameter List */
		pencil.size = DEBUGSCREEN_TEXT_SIZE;
		pencil.fg = DEBUGSCREEN_TEXT_COLOR;
		LCD_print_str("Time (UTC) :\n");
		LCD_print_str("Date       :\n");
		LCD_print_str("Data Status:\n");
		LCD_print_str("Latitude   :\n");
		LCD_print_str("Longitude  :\n");
		LCD_print_str("Speed      :\n");
		LCD_print_str("Course     :\n\n\n");
		/* Print Options */
		LCD_setText(DEBUGSCREEN_OPTION_X,DEBUGSCREEN_OPTION_Y,DEBUGSCREEN_OPTION_SIZE,DEBUGSCREEN_OPTION_COLOR,DEBUGSCREEN_SCREENCOLOR);
		
		for(int i = 0; i < DEBUGSCREEN_OPTION_COUNT; i++){
			LCD_print_str("   ");	LCD_println_str(optionsDEBUG[i].label);
		}
		pencil.x = DEBUGSCREEN_OPTION_X; pencil.y = DEBUGSCREEN_OPTION_Y;
		LCD_print_char('>');
		break;
		
		case TRACESCREEN:
		/* Draw Panes*/
		LCD_clearScreen_in(NAVSCREEN_SCREENCOLOR);
		LCD_drawLogo(ALLSCREENS_LOGO_X,ALLSCREENS_LOGO_Y,ALLSCREENS_LOGO_SIZE);
		LCD_setIconState(CARDICON,0);
		LCD_setIconState(GPSICON,0);
		LCD_drawRect_empty(NAVSCREEN_MAP_PANEX, NAVSCREEN_MAP_PANEY, NAVSCREEN_MAP_PANEW, NAVSCREEN_MAP_PANEH, NAVSCREEN_MAP_PANECOLOR);
		LCD_drawRect_empty(NAVSCREEN_DIRA_PANEX,NAVSCREEN_DIRA_PANEY,NAVSCREEN_DIRA_PANEW,NAVSCREEN_DIRA_PANEH,NAVSCREEN_DIRA_PANECOLOR);
		LCD_drawRect_empty(NAVSCREEN_DIRB_PANEX,NAVSCREEN_DIRB_PANEY,NAVSCREEN_DIRB_PANEW,NAVSCREEN_DIRB_PANEH,NAVSCREEN_DIRB_PANECOLOR);
		LCD_drawRect_empty(NAVSCREEN_UTC_PANEX, NAVSCREEN_UTC_PANEY, NAVSCREEN_UTC_PANEW, NAVSCREEN_UTC_PANEH, NAVSCREEN_UTC_PANECOLOR);
		LCD_drawRect_empty(NAVSCREEN_INFO_PANEX,NAVSCREEN_INFO_PANEY,NAVSCREEN_INFO_PANEW,NAVSCREEN_INFO_PANEH,NAVSCREEN_INFO_PANECOLOR);
		/* Print Text */
		LCD_setText(NAVSCREEN_DIRA_TEXTX,NAVSCREEN_DIRA_TEXTY,NAVSCREEN_DIRA_SIZE,NAVSCREEN_DIRA_PANECOLOR,NAVSCREEN_SCREENCOLOR);
		LCD_print_str("Now: ");
		LCD_setText(NAVSCREEN_DIRB_TEXTX,NAVSCREEN_DIRB_TEXTY,NAVSCREEN_DIRB_SIZE,NAVSCREEN_DIRB_PANECOLOR,NAVSCREEN_SCREENCOLOR);
		LCD_print_str("Last:");
		LCD_setText(NAVSCREEN_UTC_TEXTX,TFTHEIGHT-10,1,BLUE,NAVSCREEN_SCREENCOLOR);
		LCD_print_str("TRACEMODE");
		LCD_setText(NAVSCREEN_INFO_TEXTX,NAVSCREEN_INFO_TEXTY,2,NAVSCREEN_INFO_PANECOLOR,NAVSCREEN_SCREENCOLOR);
		LCD_print_str("Options:");
		LCD_setText(NAVSCREEN_OPTION_X,NAVSCREEN_OPTION_Y,NAVSCREEN_OPTION_SIZE,NAVSCREEN_OPTION_COLOR,NAVSCREEN_SCREENCOLOR);
		
		for(int i = 0; i < NAVSCREEN_OPTION_COUNT; i++){
			LCD_print_str("   ");	LCD_println_str(optionsTRACE[i].label);
		}
		pencil.x = NAVSCREEN_OPTION_X; pencil.y = NAVSCREEN_OPTION_Y;
		LCD_print_char('>');
	}
	
	screen = type;
	globalOption = 0;
}

void LCD_moveScreenCursor(uint8_t optionNumber)
{
	/* Initialize Max Count Parameter */
	uint8_t optionCount;
	
	/* Set Text and Max Count Parameters */
	switch(screen) {
		case MAINSCREEN:
		LCD_setText(MAINSCREEN_OPTION_X,MAINSCREEN_OPTION_Y,MAINSCREEN_OPTION_SIZE,MAINSCREEN_OPTION_COLOR,MAINSCREEN_SCREENCOLOR);
		optionCount = MAINSCREEN_OPTION_COUNT;
		break;
		
		case DEBUGSCREEN:
		LCD_setText(DEBUGSCREEN_OPTION_X,DEBUGSCREEN_OPTION_Y,DEBUGSCREEN_OPTION_SIZE,DEBUGSCREEN_OPTION_COLOR,DEBUGSCREEN_SCREENCOLOR);
		optionCount = DEBUGSCREEN_OPTION_COUNT;
		break;
		
		case TRACESCREEN:
		LCD_setText(NAVSCREEN_OPTION_X,NAVSCREEN_OPTION_Y,NAVSCREEN_OPTION_SIZE,NAVSCREEN_OPTION_COLOR,NAVSCREEN_SCREENCOLOR);
		optionCount = NAVSCREEN_OPTION_COUNT;
		break;
		
		default: return;
	}

	/* Redraw Pointer */
	for(int i = 0; i < optionCount; i++){
		if(i == optionNumber)	LCD_print_str(">\n");
		else					LCD_print_str(" \n");
	}
}

void LCD_setIconState(OutlineImage type, uint8_t state)
{
	/* Set State of Specified Icon */
	switch(type){
		case CARDICON: LCD_drawImage(CARDICON,ALLSCREENS_CARD_X, ALLSCREENS_CARD_Y, ALLSCREENS_CARD_SIZE, state ? ALLSCREENS_CARD_COLORON : ALLSCREENS_CARD_COLOROFF);	break;
		case GPSICON:  LCD_drawImage(GPSICON, ALLSCREENS_GPS_X,  ALLSCREENS_GPS_Y,  ALLSCREENS_GPS_SIZE,  state ? ALLSCREENS_GPS_COLORON : ALLSCREENS_GPS_COLOROFF);	break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   LCD Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
void LCD_spi_init()
{
	/* Set SPI Speed and Settings */	
	SPDDR |= (1<<LDC)|(1<<LCS)|(1<<5)|(1<<7);
	SPPORT |= (1<<LCS);					// Disable CS and RST during startup
	SPCR0 |= (1<<SPE0)|(1<<MSTR0);						// Enable SPI - fclk/4
	SPSR0 |= (1<<SPI2X0);								// Double SPI speed
}

void LCD_spi_send(uint8_t data)
{
	/* Send SPI data */
	SPDR0 = data ;					// Load data into SPDR - initiates transmission
	while(!(SPSR0 & (1<<SPIF0))) ;	// Wait till the transmission is finished
}

void LCD_writecommand8(uint8_t command)
{
	/* Write 8-bit Command */
	SPPORT &= ~((1<<LDC)|(1<<LCS));	// Enable chip select and set command-mode
	LCD_spi_send(command);			// Send command
	SPPORT |= (1<<LCS);				// Disable chip select
	SPPORT |= (1<<LDC);				// EXPLICITELY ENABLE DATA [SD LCD CIVIL WAR]
}

void LCD_writedata8(uint8_t data)
{
	/*Write 8-bit Data */
	SPPORT |=(1<<LDC);		// Set data-mode
	SPPORT &= ~(1<<LCS);	// Enable chip select
	LCD_spi_send(data);		// Send data
	SPPORT |=(1<<LCS);		// Disable chip select
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