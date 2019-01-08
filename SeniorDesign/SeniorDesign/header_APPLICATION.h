////////////////////////////////////////////////////////////////////////////////////////////////////
//									     Application Header										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef HEADER_APPLICATION_H
#define HEADER_APPLICATION_H
////////////////////////////////////////////////////////////////////////////////////////////////////
//										Application Libraries									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "header_KEYPAD.h"
#include "header_LCD.h"
#include "header_SFX.h"
#include "header_FUNCTIONS.h"
#include "header_DISK.h"

#include <avr/io.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//									 Application Type Definitions								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
	Enumeration: DataType
	Description:
		Organizes all types of data encountered in application memory, including: 
			
			SIGNATURE    - Signature         - 8-bit MCU and Card linkage
			M_TRACE      - Trace             - Contains a bitmap of routers/string of nodes
			M_SINGULAR   - Single Coordinate - Similar to nodes, but unbound to quadrants
			D_ROUTER     - Router            - Map to quadrant-starting/continuing nodes in trace
			D_NORMALNODE - Normal Node       - Contains relative position/invisible UTC data
			D_SUPERNODE  - Super Node        - Contains relative position/visible UTC data
			D_ORIGINNODE - Origin Node       - Contains absolute position/visible UTC data
			D_REFNODE    - Reference Node    - Contains absolute position/invisible UTC data
			
***************************************************************************************************/
typedef enum {
	SIGNATURE,
	M_TRACE,
	M_SINGULAR,
	D_ROUTER,
	D_NORMALNODE,
	D_SUPERNODE,
	D_ORIGINNODE,
	D_REFNODE
	
} DataType;

typedef enum {
	NONE,
	TRACING,
	RETRACING,
	DEBUGGING
} ModeType;

typedef enum {
	NORTH,
	NORTHEAST,
	EAST,
	SOUTHEAST,
	SOUTH,
	SOUTHWEST,
	WEST,
	NORTHWEST	
} Direction;

typedef struct {
	uint8_t isDGPSon;
	ModeType mode;
	uint16_t entryCount;
	uint32_t liveSector;
} SettingHandler;

typedef struct {
	Vector2 ref;
	Vector2 pos;
	Vector2 sup;
	Vector2 quad;
	uint32_t startSector;
} TraceHandler; 

////////////////////////////////////////////////////////////////////////////////////////////////////
//								    Application Public Functions								  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Core Functions */
void APP_loadProgram_fast();
void APP_loadProgram();
void APP_startMode_debug();
void APP_startMode_main();

/* Debug GPS Functions */
void APP_DGPS_setState(uint8_t state);
void APP_DGPS_setTime(uint8_t h, uint8_t m, uint8_t s);
void APP_DGPS_setDate(uint8_t d, uint8_t m, uint8_t y);
void APP_DGPS_setLocation(int16_t lat, int16_t lon);
void APP_DGPS_setCourse(uint16_t course);

/* Testing Functions (Eventually Become Private) */
uint8_t APP_formatCard();
void APP_startMode_trace();
void APP_update_trace();
void APP_update_debug();
uint8_t APP_write_manifest(DataType type);
uint8_t APP_write_node(DataType type);
uint8_t APP_write_router();
void APP_reDrawMapPane();

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Application Public MACROS									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
/* Control Parameters */
extern SettingHandler settings;
#define MASTERUPDATETIME 100
#define TIMER0_NE6 64E6
#define D2PX 1
#define MAPXBOUND (NAVSCREEN_MAP_PANEW / 2)
#define MAPYBOUND (NAVSCREEN_MAP_PANEH / 2)

/* Trace Parameters */
#define NODECOLOR_NORMAL WHITE
#define NODECOLOR_SUPER  GREEN
#define NODECOLOR_USER   RED
#define NODESIZE		 4
#define NODESIZE_S		 2
#define QUAD_COLCOUNT    32
#define QUAD_ROWCOUNT    32

/* Set Loading Screen Parameters */
#define LOADSCREEN_SCREENCOLOR BLACK
#define LOADSCREEN_LOGO_XOFF 5
#define LOADSCREEN_LOGO_YOFF 5
#define LOADSCREEN_LOGO_SIZE 3
#define LOADSCREEN_TEXT_XOFF 20
#define LOADSCREEN_TEXT_SIZE 2
#define LOADSCREEN_TEXT_YOFF (LOADSCREEN_LOGO_YOFF + LOADSCREEN_LOGO_SIZE * LOGOSIZE + LOADSCREEN_LOGO_SIZE)
#define LOADSCREEN_TEXT_COLOR WHITE
#define LOADSCREEN_COMPANY_COLOR GREEN

/* Signature Parameters */
#define SIG_SECTOR			0
#define SIG_BLOCKLEN		1
#define SIG_TOTAL_SIZE		(1 + 1)
/* Manifest Parameters */
#define MAN_SECTOR			(0 + SIG_SECTOR + SIG_BLOCKLEN)
#define MAN_BLOCKLEN		255
#define MAN_TYPE_OFF		0
#define MAN_TYPE_SIZE		(1 + 1)
#define MAN_START_OFF		(MAN_TYPE_OFF + MAN_TYPE_SIZE)
#define MAN_START_SIZE		(8 + 1)
#define MAN_END_OFF			(MAN_START_OFF + MAN_START_SIZE)
#define MAN_END_SIZE		(8 + 1)
#define MAN_TOTAL_SIZE		(MAN_TYPE_SIZE + MAN_START_SIZE + MAN_END_SIZE)
/* Database Parameters */
#define DAT_SECTOR			(0 + MAN_SECTOR + MAN_BLOCKLEN)
#define DAT_TYPE_OFF		0
#define DAT_TYPE_SIZE		(1 + 1)
#define DAT_EID_OFF			(DAT_TYPE_OFF + DAT_TYPE_SIZE)
#define DAT_EID_SIZE		(3 + 1)
/* Node Specific Parameters */
#define DAT_X_OFF			(DAT_EID_OFF + DAT_EID_SIZE)
#define DAT_X_SIZE			GPS_BYTES_ASCII_LONGITUDE
#define DAT_Y_OFF			(DAT_X_OFF + DAT_X_SIZE)
#define DAT_Y_SIZE			GPS_BYTES_ASCII_LATITUDE
#define DAT_TIME_OFF		(DAT_Y_OFF + DAT_Y_SIZE)
#define DAT_TIME_SIZE		GPS_BYTES_ASCII_UTC_TIME
#define DAT_DATE_OFF		(DAT_TIME_OFF + DAT_TIME_SIZE)
#define DAT_DATE_SIZE		GPS_BYTES_ASCII_UTC_DATE
#define DAT_QUADC_OFF		(DAT_DATE_OFF + DAT_DATE_SIZE)
#define DAT_QUADC_SIZE		(3 + 1)
#define DAT_QUADR_OFF		(DAT_QUADC_OFF + DAT_QUADC_SIZE)
#define DAT_QUADR_SIZE		(3 + 1)
#define DAT_NODE_SIZE		(DAT_TYPE_SIZE + DAT_EID_SIZE  + DAT_X_SIZE + DAT_Y_SIZE + DAT_TIME_SIZE + DAT_DATE_SIZE + DAT_QUADC_SIZE + DAT_QUADR_SIZE)
/* Router Specific Parameters */
#define DAT_ADDRN_OFF		(DAT_EID_OFF + DAT_EID_SIZE)
#define DAT_ADDRN_SIZE		(8 + 1)
#define DAT_ROUTER_SIZE		(DAT_TYPE_SIZE + DAT_EID_SIZE + DAT_ADDRN_SIZE * 8)

#endif