////////////////////////////////////////////////////////////////////////////////////////////////////
//									          LCD Header										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef HEADER_LCD_H
#define HEADER_LCD_H
////////////////////////////////////////////////////////////////////////////////////////////////////
//									LCD Data Structures and Libraries							  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/pgmspace.h>
typedef enum {FALSE,TRUE}	Bool;
typedef uint16_t			Color;
typedef struct{
	int16_t x;
	int16_t y;
	} Vector2;
	
typedef struct{
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
	int16_t xoff;
	int16_t yoff;
	int16_t space;
	Color fg;
	Color bg;
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
////////////////////////////////////////////////////////////////////////////////////////////////////
//									      LCD Public Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/* System Initialization */
void LCD_init_system ();

/* Drawing */
void LCD_drawPixel (uint16_t x, uint16_t y, Color color);
void LCD_drawRect_filled (uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color);
void LCD_drawRect_empty (uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color);
void LCD_drawCircle_filled(uint16_t x0, uint16_t y0, uint8_t radius, Color color);
void LCD_drawCircle_empty(uint16_t x0, uint16_t y0, uint8_t radius, Color color);
void LCD_clear (Color color);

/* Text */
void LCD_setText_cursor(uint16_t x, uint16_t y);
void LCD_setText_size(uint8_t size);
void LCD_setText_color(Color fg, Color bg);
void LCD_setText_all(uint16_t x, uint16_t y, uint8_t size, Color fg, Color bg);
void LCD_moveTextCursor(int16_t spaces, int16_t lines);
void LCD_print_str(char * str);
void LCD_print_num(float num, uint8_t width, uint8_t prec);

/* One-Instance Drawing */
void LCD_init_grid (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t space, int16_t xoff, int16_t yoff, Color fg, Color bg);
void LCD_init_arrow(uint16_t x0, uint16_t y0, uint16_t rot, uint16_t fg);
Grid LCD_get_grid();
Arrow LCD_get_arrow();
void LCD_shiftGrid(Vector2 dir);
void LCD_zoomGridIn(uint16_t x, uint16_t y);
void LCD_zoomGridOut(uint16_t x, uint16_t y);
//void LCD_moveArrow(Vector2 dir);
//void LCD_rotateArrow(int16_t rot);




void LCD_pushColor(uint16_t);
void LCD_setAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
void LCD_writecommand8(uint8_t);
void LCD_writedata8(uint8_t);
////////////////////////////////////////////////////////////////////////////////////////////////////
//									       LCD Public MACROS									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Direction MACROS */
static const Vector2 UP = {0,1};
static const Vector2 DOWN = {0,-1};
static const Vector2 LEFT = {-1,0};
static const Vector2 RIGHT = {1,0};

/* Control MACROS */
#define SPPORT PORTB
#define SPDDR DDRB
#define SPPIN PINB
#define CS 3
#define DC 4
#define TFTHEIGHT 240 
#define TFTWIDTH 320
#define ARROWSIZE 29
#define ARROWRADIUS ARROWSIZE / 2

/* Color MACROS */
#define BLACK       0x0000      
#define NAVY        0x000F      
#define DARKGREEN   0x03E0      
#define DARKCYAN    0x03EF      
#define MAROON      0x7800      
#define PURPLE      0x780F      
#define OLIVE       0x7BE0      
#define LIGHTGREY   0xC618      
#define DARKGREY    0x7BEF      
#define BLUE        0x001F      
#define GREEN       0x07E0      
#define CYAN        0x07FF      
#define RED         0xF800     
#define MAGENTA     0xF81F      
#define YELLOW      0xFFE0      
#define WHITE       0xFFFF      
#define ORANGE      0xFD20      
#define GREENYELLOW 0xAFE5     
#define PINK        0xF81F

static const uint32_t arrow_BMPs[18][29] PROGMEM = {
	{0,0,0,0,0x00800000,0x00f00000,0x00fc0000,0x007f8000,0x007fe000,0x007ff800,0x003fff00,0x003fffc0,0x003ffff0,0x001ffffe,0x001fffff,0x001ffffe,0x003ffff0,0x003fffc0,0x003fff00,0x007ff800,0x007fe000,0x007f8000,0x00fc0000,0x00f00000,0x00800000,0,0,0,0},
	{0,0,0,0,0x00800000,0x00f00000,0x00fc0000,0x007fa000,0x007ff800,0x007fff00,0x003fffc0,0x003ffff6,0x003fffff,0x001fffff,0x000ffffc,0x000ffff8,0x001fffe0,0x001fff80,0x001ffc00,0x003ff000,0x003fc000,0x003fc000,0x007e0000,0x00780000,0x00400000,0,0,0,0},
	{0,0,0,0,0,0x01000000,0x01f80000,0x01ff4000,0x00fff800,0x007fffc0,0x007ffff6,0x003fffff,0x003ffffe,0x003ffffc,0x001ffff8,0x000fffe0,0x000fff80,0x001fff00,0x001ffc00,0x001ff800,0x001fe000,0x001fe000,0x001f0000,0x003c0000,0x00380000,0x00200000,0,0,0},
	{0,0,0,0,0,0,0x02400000,0x01fb4000,0x01fff600,0x00fffff6,0x00ffffff,0x003ffffe,0x003ffff8,0x003ffff8,0x001fffe0,0x000fffc0,0x000fff80,0x001ffe00,0x000ffe00,0x000ff800,0x001fe000,0x001fe000,0x000e8000,0x000e0000,0x001e0000,0x00100000,0,0,0},
	{0,0,0,0,0,0,0,0x02d20000,0x03ffffac,0x03ffffff,0x00fffffe,0x00fffff8,0x00bffff8,0x003fffe0,0x003fffe0,0x000fff40,0x000fff00,0x000ffe00,0x000ff800,0x000ff800,0x000ff000,0x000fe000,0x000f8000,0x000f8000,0x000f0000,0x000c0000,0,0,0},
	{0,0,0,0,0,0,0,0x00000524,0x05affffe,0x03fffffe,0x03fffff8,0x01fffff0,0x00ffffe0,0x003fffe0,0x003fff80,0x002fff80,0x000ffe00,0x000ffe00,0x000ffc00,0x000ff800,0x000ff800,0x000ff000,0x0007c000,0x00078000,0x00078000,0x00070000,0x00020000,0,0},
	{0,0,0,0,0,0,0x00000008,0x000007dc,0x000d7ffc,0x05fffff8,0x07fffff0,0x03ffffe0,0x00ffffe0,0x00ffffc0,0x003fffc0,0x003fff00,0x000fff00,0x0007fe00,0x0007fc00,0x0007f800,0x0007f800,0x0007f000,0x0007e000,0x0003c000,0x0003c000,0x00038000,0x00030000,0x00010000,0},
	{0,0,0,0,0,0x0000000c,0x0000027c,0x00001bfc,0x0005fff0,0x003ffff0,0x09ffffe0,0x07ffffe0,0x03ffffc0,0x00ffffc0,0x00ffff80,0x003fff00,0x001fff00,0x0007fe00,0x0007fe00,0x0007fc00,0x0007fc00,0x0003f000,0x0003f000,0x0003e000,0x0001e000,0x0001c000,0x00018000,0x00008000,0},
	{0,0,0,0,0x00000010,0x00000038,0x000003f8,0x00001ff0,0x0002ffe0,0x001bffe0,0x015fffc0,0x03ffffc0,0x0fffff80,0x07ffff80,0x00ffff00,0x00bfff00,0x003ffe00,0x000ffe00,0x0003fc00,0x0003fc00,0x0003fc00,0x0001f800,0x0001f800,0x0001f000,0x0000f000,0x0000e000,0x0000c000,0x00004000,0},
	{0,0,0,0,0x00000070,0x00000170,0x00001fe0,0x00003fe0,0x0000bfe0,0x000fffc0,0x001fffc0,0x01ffffc0,0x03ffffc0,0x0fffff80,0x05ffff00,0x00ffff00,0x003ffe00,0x0017fe00,0x0003fe00,0x0003fe00,0x0003fc00,0x0001f800,0x0000f800,0x0000f800,0x00007800,0x00007000,0x00006000,0x00002000,0},
	{0,0,0,0x00000060,0x000000f0,0x00000be0,0x00002fc0,0x00003fc0,0x0000ffc0,0x0003ffc0,0x001fff80,0x007fff80,0x01ffff80,0x03ffff00,0x0fffff00,0x07ffff00,0x00ffff00,0x001fff00,0x0003fe00,0x0003fe00,0x0001fc00,0x0001fc00,0x0000fc00,0x0000fc00,0x00003c00,0x00003800,0x00003000,0x00001000,0},
	{0,0,0x000000a0,0x000000e0,0x000003c0,0x00000fc0,0x00003fc0,0x00007f80,0x00017f80,0x0005ff80,0x0017ff80,0x001fff80,0x007fff00,0x01ffff00,0x03ffff00,0x0bffff00,0x07ffff00,0x00ffff00,0x001fff00,0x0001fe00,0x0001fe00,0x0000fe00,0x00007c00,0x00007c00,0x00001c00,0x00001800,0x00000800,0,0},
	{0,0x00000040,0x000001c0,0x000003c0,0x00000780,0x00001f80,0x00007f80,0x00007f80,0x0001ff80,0x0003ff80,0x000fff80,0x001fff00,0x003fff00,0x007fff00,0x01ffff00,0x03ffff00,0x0bffff00,0x07ffff00,0x0057ff00,0x0003fe00,0x0000fe00,0x0000fe00,0x00003e00,0x00003e00,0x00000e00,0x00000e00,0x00000600,0,0},
	{0,0x00000300,0x00000380,0x00000700,0x00001f00,0x00003f80,0x0000bf00,0x0000ff00,0x0000ff80,0x0003ff80,0x000fff80,0x001fff00,0x003fff00,0x003fff00,0x007fff00,0x01ffff00,0x03ffff00,0x07ffff00,0x02ffff00,0x000dff00,0x0000ff00,0x0000df00,0x00001f00,0x00001f00,0x00000f00,0x00000700,0x00000100,0,0},
	{0x00000200,0x00000600,0x00000700,0x00001e00,0x00001e00,0x00007f00,0x0000ff00,0x0001ff00,0x0001ff00,0x0003ff00,0x0007ff00,0x000fff00,0x001fff00,0x003fff00,0x003fff00,0x00ffff00,0x01ffff00,0x01ffff80,0x03ffff00,0x03fdff00,0x00007f80,0x00007f80,0x00001f80,0x00001f80,0x00000380,0x00000380,0,0,0},
	{0x00000400,0x00000e00,0x00000e00,0x00003e00,0x00003e00,0x00007e00,0x0000fe00,0x0001fe00,0x0001fe00,0x0005ff00,0x0007ff00,0x000fff00,0x000fff00,0x003fff00,0x003fff80,0x007fff80,0x00ffff80,0x01ffff80,0x01ffff80,0x01ffff80,0x03327f80,0x00003f80,0x000007c0,0x00000780,0x00000180,0x00000040,0,0,0},
	{0x00000800,0x00001c00,0x00003c00,0x00007c00,0x00007c00,0x0000fc00,0x0000fe00,0x0001fe00,0x0003fe00,0x0003fe00,0x0007fe00,0x000fff00,0x000fff00,0x003fff00,0x003fff80,0x003fff80,0x007fff80,0x007fff80,0x00ffff80,0x01ffffc0,0x01fe7fc0,0x03803fc0,0x000007c0,0x000001c0,0x000000e0,0,0,0,0},
	{0x00003000,0x00003800,0x00007800,0x0000f800,0x0000f800,0x0001f800,0x0001fc00,0x0003fc00,0x0003fe00,0x0003fe00,0x0007fe00,0x0007ff00,0x000fff00,0x000fff80,0x003fff80,0x003fff80,0x003fff80,0x007fff80,0x007fffc0,0x00ffffc0,0x00ff3fe0,0x00f81fe0,0x01c003e0,0x00000070,0,0,0,0,0}
};


static const unsigned char font[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
	0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
	0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
	0x18, 0x3C, 0x7E, 0x3C, 0x18,
	0x1C, 0x57, 0x7D, 0x57, 0x1C,
	0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
	0x00, 0x18, 0x3C, 0x18, 0x00,
	0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
	0x00, 0x18, 0x24, 0x18, 0x00,
	0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
	0x30, 0x48, 0x3A, 0x06, 0x0E,
	0x26, 0x29, 0x79, 0x29, 0x26,
	0x40, 0x7F, 0x05, 0x05, 0x07,
	0x40, 0x7F, 0x05, 0x25, 0x3F,
	0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
	0x7F, 0x3E, 0x1C, 0x1C, 0x08,
	0x08, 0x1C, 0x1C, 0x3E, 0x7F,
	0x14, 0x22, 0x7F, 0x22, 0x14,
	0x5F, 0x5F, 0x00, 0x5F, 0x5F,
	0x06, 0x09, 0x7F, 0x01, 0x7F,
	0x00, 0x66, 0x89, 0x95, 0x6A,
	0x60, 0x60, 0x60, 0x60, 0x60,
	0x94, 0xA2, 0xFF, 0xA2, 0x94,
	0x08, 0x04, 0x7E, 0x04, 0x08,
	0x10, 0x20, 0x7E, 0x20, 0x10,
	0x08, 0x08, 0x2A, 0x1C, 0x08,
	0x08, 0x1C, 0x2A, 0x08, 0x08,
	0x1E, 0x10, 0x10, 0x10, 0x10,
	0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
	0x30, 0x38, 0x3E, 0x38, 0x30,
	0x06, 0x0E, 0x3E, 0x0E, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x5F, 0x00, 0x00,
	0x00, 0x07, 0x00, 0x07, 0x00,
	0x14, 0x7F, 0x14, 0x7F, 0x14,
	0x24, 0x2A, 0x7F, 0x2A, 0x12,
	0x23, 0x13, 0x08, 0x64, 0x62,
	0x36, 0x49, 0x56, 0x20, 0x50,
	0x00, 0x08, 0x07, 0x03, 0x00,
	0x00, 0x1C, 0x22, 0x41, 0x00,
	0x00, 0x41, 0x22, 0x1C, 0x00,
	0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
	0x08, 0x08, 0x3E, 0x08, 0x08,
	0x00, 0x80, 0x70, 0x30, 0x00,
	0x08, 0x08, 0x08, 0x08, 0x08,
	0x00, 0x00, 0x60, 0x60, 0x00,
	0x20, 0x10, 0x08, 0x04, 0x02,
	0x3E, 0x51, 0x49, 0x45, 0x3E,
	0x00, 0x42, 0x7F, 0x40, 0x00,
	0x72, 0x49, 0x49, 0x49, 0x46,
	0x21, 0x41, 0x49, 0x4D, 0x33,
	0x18, 0x14, 0x12, 0x7F, 0x10,
	0x27, 0x45, 0x45, 0x45, 0x39,
	0x3C, 0x4A, 0x49, 0x49, 0x31,
	0x41, 0x21, 0x11, 0x09, 0x07,
	0x36, 0x49, 0x49, 0x49, 0x36,
	0x46, 0x49, 0x49, 0x29, 0x1E,
	0x00, 0x00, 0x14, 0x00, 0x00,
	0x00, 0x40, 0x34, 0x00, 0x00,
	0x00, 0x08, 0x14, 0x22, 0x41,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x00, 0x41, 0x22, 0x14, 0x08,
	0x02, 0x01, 0x59, 0x09, 0x06,
	0x3E, 0x41, 0x5D, 0x59, 0x4E,
	0x7C, 0x12, 0x11, 0x12, 0x7C,
	0x7F, 0x49, 0x49, 0x49, 0x36,
	0x3E, 0x41, 0x41, 0x41, 0x22,
	0x7F, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x49, 0x49, 0x49, 0x41,
	0x7F, 0x09, 0x09, 0x09, 0x01,
	0x3E, 0x41, 0x41, 0x51, 0x73,
	0x7F, 0x08, 0x08, 0x08, 0x7F, //H
	0x00, 0x41, 0x7F, 0x41, 0x00,
	0x20, 0x40, 0x41, 0x3F, 0x01,
	0x7F, 0x08, 0x14, 0x22, 0x41,
	0x7F, 0x40, 0x40, 0x40, 0x40,
	0x7F, 0x02, 0x1C, 0x02, 0x7F,
	0x7F, 0x04, 0x08, 0x10, 0x7F,
	0x3E, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x09, 0x09, 0x09, 0x06,
	0x3E, 0x41, 0x51, 0x21, 0x5E,
	0x7F, 0x09, 0x19, 0x29, 0x46,
	0x26, 0x49, 0x49, 0x49, 0x32,
	0x03, 0x01, 0x7F, 0x01, 0x03,
	0x3F, 0x40, 0x40, 0x40, 0x3F,
	0x1F, 0x20, 0x40, 0x20, 0x1F,
	0x3F, 0x40, 0x38, 0x40, 0x3F,
	0x63, 0x14, 0x08, 0x14, 0x63,
	0x03, 0x04, 0x78, 0x04, 0x03,
	0x61, 0x59, 0x49, 0x4D, 0x43,
	0x00, 0x7F, 0x41, 0x41, 0x41,
	0x02, 0x04, 0x08, 0x10, 0x20,
	0x00, 0x41, 0x41, 0x41, 0x7F,
	0x04, 0x02, 0x01, 0x02, 0x04,
	0x40, 0x40, 0x40, 0x40, 0x40,
	0x00, 0x03, 0x07, 0x08, 0x00,
	0x20, 0x54, 0x54, 0x78, 0x40,
	0x7F, 0x28, 0x44, 0x44, 0x38,
	0x38, 0x44, 0x44, 0x44, 0x28,
	0x38, 0x44, 0x44, 0x28, 0x7F,
	0x38, 0x54, 0x54, 0x54, 0x18,
	0x00, 0x08, 0x7E, 0x09, 0x02,
	0x18, 0xA4, 0xA4, 0x9C, 0x78,
	0x7F, 0x08, 0x04, 0x04, 0x78,
	0x00, 0x44, 0x7D, 0x40, 0x00,
	0x20, 0x40, 0x40, 0x3D, 0x00,
	0x7F, 0x10, 0x28, 0x44, 0x00,
	0x00, 0x41, 0x7F, 0x40, 0x00,
	0x7C, 0x04, 0x78, 0x04, 0x78,
	0x7C, 0x08, 0x04, 0x04, 0x78,
	0x38, 0x44, 0x44, 0x44, 0x38,
	0xFC, 0x18, 0x24, 0x24, 0x18,
	0x18, 0x24, 0x24, 0x18, 0xFC,
	0x7C, 0x08, 0x04, 0x04, 0x08,
	0x48, 0x54, 0x54, 0x54, 0x24,
	0x04, 0x04, 0x3F, 0x44, 0x24,
	0x3C, 0x40, 0x40, 0x20, 0x7C,
	0x1C, 0x20, 0x40, 0x20, 0x1C,
	0x3C, 0x40, 0x30, 0x40, 0x3C,
	0x44, 0x28, 0x10, 0x28, 0x44,
	0x4C, 0x90, 0x90, 0x90, 0x7C,
	0x44, 0x64, 0x54, 0x4C, 0x44,
	0x00, 0x08, 0x36, 0x41, 0x00,
	0x00, 0x00, 0x77, 0x00, 0x00,
	0x00, 0x41, 0x36, 0x08, 0x00,
	0x02, 0x01, 0x02, 0x04, 0x02,
	0x3C, 0x26, 0x23, 0x26, 0x3C,
	0x1E, 0xA1, 0xA1, 0x61, 0x12,
	0x3A, 0x40, 0x40, 0x20, 0x7A,
	0x38, 0x54, 0x54, 0x55, 0x59,
	0x21, 0x55, 0x55, 0x79, 0x41,
	0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
	0x21, 0x55, 0x54, 0x78, 0x40,
	0x20, 0x54, 0x55, 0x79, 0x40,
	0x0C, 0x1E, 0x52, 0x72, 0x12,
	0x39, 0x55, 0x55, 0x55, 0x59,
	0x39, 0x54, 0x54, 0x54, 0x59,
	0x39, 0x55, 0x54, 0x54, 0x58,
	0x00, 0x00, 0x45, 0x7C, 0x41,
	0x00, 0x02, 0x45, 0x7D, 0x42,
	0x00, 0x01, 0x45, 0x7C, 0x40,
	0x7D, 0x12, 0x11, 0x12, 0x7D, // A-umlaut
	0xF0, 0x28, 0x25, 0x28, 0xF0,
	0x7C, 0x54, 0x55, 0x45, 0x00,
	0x20, 0x54, 0x54, 0x7C, 0x54,
	0x7C, 0x0A, 0x09, 0x7F, 0x49,
	0x32, 0x49, 0x49, 0x49, 0x32,
	0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
	0x32, 0x4A, 0x48, 0x48, 0x30,
	0x3A, 0x41, 0x41, 0x21, 0x7A,
	0x3A, 0x42, 0x40, 0x20, 0x78,
	0x00, 0x9D, 0xA0, 0xA0, 0x7D,
	0x3D, 0x42, 0x42, 0x42, 0x3D, // O-umlaut
	0x3D, 0x40, 0x40, 0x40, 0x3D,
	0x3C, 0x24, 0xFF, 0x24, 0x24,
	0x48, 0x7E, 0x49, 0x43, 0x66,
	0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
	0xFF, 0x09, 0x29, 0xF6, 0x20,
	0xC0, 0x88, 0x7E, 0x09, 0x03,
	0x20, 0x54, 0x54, 0x79, 0x41,
	0x00, 0x00, 0x44, 0x7D, 0x41,
	0x30, 0x48, 0x48, 0x4A, 0x32,
	0x38, 0x40, 0x40, 0x22, 0x7A,
	0x00, 0x7A, 0x0A, 0x0A, 0x72,
	0x7D, 0x0D, 0x19, 0x31, 0x7D,
	0x26, 0x29, 0x29, 0x2F, 0x28,
	0x26, 0x29, 0x29, 0x29, 0x26,
	0x30, 0x48, 0x4D, 0x40, 0x20,
	0x38, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x38,
	0x2F, 0x10, 0xC8, 0xAC, 0xBA,
	0x2F, 0x10, 0x28, 0x34, 0xFA,
	0x00, 0x00, 0x7B, 0x00, 0x00,
	0x08, 0x14, 0x2A, 0x14, 0x22,
	0x22, 0x14, 0x2A, 0x14, 0x08,
	0xAA, 0x00, 0x55, 0x00, 0xAA,
	0xAA, 0x55, 0xAA, 0x55, 0xAA,
	0x00, 0x00, 0x00, 0xFF, 0x00,
	0x10, 0x10, 0x10, 0xFF, 0x00,
	0x14, 0x14, 0x14, 0xFF, 0x00,
	0x10, 0x10, 0xFF, 0x00, 0xFF,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x14, 0x14, 0x14, 0xFC, 0x00,
	0x14, 0x14, 0xF7, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x14, 0x14, 0xF4, 0x04, 0xFC,
	0x14, 0x14, 0x17, 0x10, 0x1F,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0x1F, 0x00,
	0x10, 0x10, 0x10, 0xF0, 0x00,
	0x00, 0x00, 0x00, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0xF0, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0xFF, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x14,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x00, 0x00, 0x1F, 0x10, 0x17,
	0x00, 0x00, 0xFC, 0x04, 0xF4,
	0x14, 0x14, 0x17, 0x10, 0x17,
	0x14, 0x14, 0xF4, 0x04, 0xF4,
	0x00, 0x00, 0xFF, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x14, 0x14, 0xF7, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x17, 0x14,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0xF4, 0x14,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x00, 0x00, 0x1F, 0x10, 0x1F,
	0x00, 0x00, 0x00, 0x1F, 0x14,
	0x00, 0x00, 0x00, 0xFC, 0x14,
	0x00, 0x00, 0xF0, 0x10, 0xF0,
	0x10, 0x10, 0xFF, 0x10, 0xFF,
	0x14, 0x14, 0x14, 0xFF, 0x14,
	0x10, 0x10, 0x10, 0x1F, 0x00,
	0x00, 0x00, 0x00, 0xF0, 0x10,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0xFF,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0x38, 0x44, 0x44, 0x38, 0x44,
	0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
	0x7E, 0x02, 0x02, 0x06, 0x06,
	0x02, 0x7E, 0x02, 0x7E, 0x02,
	0x63, 0x55, 0x49, 0x41, 0x63,
	0x38, 0x44, 0x44, 0x3C, 0x04,
	0x40, 0x7E, 0x20, 0x1E, 0x20,
	0x06, 0x02, 0x7E, 0x02, 0x02,
	0x99, 0xA5, 0xE7, 0xA5, 0x99,
	0x1C, 0x2A, 0x49, 0x2A, 0x1C,
	0x4C, 0x72, 0x01, 0x72, 0x4C,
	0x30, 0x4A, 0x4D, 0x4D, 0x30,
	0x30, 0x48, 0x78, 0x48, 0x30,
	0xBC, 0x62, 0x5A, 0x46, 0x3D,
	0x3E, 0x49, 0x49, 0x49, 0x00,
	0x7E, 0x01, 0x01, 0x01, 0x7E,
	0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
	0x44, 0x44, 0x5F, 0x44, 0x44,
	0x40, 0x51, 0x4A, 0x44, 0x40,
	0x40, 0x44, 0x4A, 0x51, 0x40,
	0x00, 0x00, 0xFF, 0x01, 0x03,
	0xE0, 0x80, 0xFF, 0x00, 0x00,
	0x08, 0x08, 0x6B, 0x6B, 0x08,
	0x36, 0x12, 0x36, 0x24, 0x36,
	0x06, 0x0F, 0x09, 0x0F, 0x06,
	0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x10, 0x10, 0x00,
	0x30, 0x40, 0xFF, 0x01, 0x01,
	0x00, 0x1F, 0x01, 0x01, 0x1E,
	0x00, 0x19, 0x1D, 0x17, 0x12,
	0x00, 0x3C, 0x3C, 0x3C, 0x3C,
	0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

