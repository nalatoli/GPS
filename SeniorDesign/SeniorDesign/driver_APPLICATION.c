#include "header_APPLICATION.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//									         APP DRIVER										      //
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////
//									    APP Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t APP_course2rot();
int16_t APP_lastSuper2rot();
void APP_update_MASTER();
void APP_DGPS_incTime();
void APP_setUpdateState(uint8_t state);
////////////////////////////////////////////////////////////////////////////////////////////////////
//										APP Driver Objects										  //
////////////////////////////////////////////////////////////////////////////////////////////////////
SettingHandler settings;
TraceHandler trace;
////////////////////////////////////////////////////////////////////////////////////////////////////
//									   APP Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//										  Core Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

void APP_loadProgram_fast()
{
	/* Initialize Systems */
	DISK_init();
	LCD_init();
	SFX_init();				
	GPS_configure_firmware();
	KEY_init();					
	APP_formatCard();
	
	/* Configure Update Settings */				// ***
	OCR0A = 1000 / 8;							// Timer 0: CTC Period = 1 ms
	TCCR0A = (1<<WGM01);						// Timer 0: Mode = "CTC"
	TCCR0B = (1<<CS01)|(1<<CS00);				// Timer 0: N = 64
	TIMSK0 |= (1<<OCIE0A);						// Enable Timer 0 CMPA interrupt
	APP_startMode_main();
	sei();
}

void APP_loadProgram()
{
	/* Initialize LCD */
	KEY_init();					// Initialize Keys
	LCD_init();
	
	/* Set Up Initialization Test */
	LCD_clearScreen_in(LOADSCREEN_SCREENCOLOR);
	LCD_setText(LOADSCREEN_TEXT_XOFF,LOADSCREEN_TEXT_YOFF,LOADSCREEN_TEXT_SIZE,LOADSCREEN_COMPANY_COLOR,LOADSCREEN_SCREENCOLOR);
	
	/* Print Brand */
	LCD_drawLogo(LOADSCREEN_LOGO_XOFF,LOADSCREEN_LOGO_YOFF,LOADSCREEN_LOGO_SIZE);
	LCD_print_str("Power Couple TM\n");
	
	/* Initialize Drivers */
	pencil.fg = LOADSCREEN_TEXT_COLOR;
	LCD_print_str("Initializing SFX...\n");		SFX_init();					// Initialize SFX 
	LCD_setIconState(GPSICON,1);											// Set active GPS icon
	LCD_print_str("Initializing GPS...\n");		GPS_configure_firmware();	// Initialize GPS
	LCD_setIconState(GPSICON,0);											// Set inactive GPS icon
	LCD_setIconState(CARDICON,1);											// Set active card icon
	LCD_print_str("Initializing Disk...\n");	DISK_init();				// Initialize disk
	LCD_setIconState(CARDICON,0);											// Set inactive card icon
	
	/* Format/Load from Card */
	LCD_print_str("Checking Signature...\n");
	if(APP_formatCard()) 
	LCD_print_str("Formatting Done...\n");
	else                
	LCD_print_str("Data Loaded...\n");	
	
	/* Configure Update Settings */				// ***
	LCD_print_str("Configuring System...\n");	// Print test
	OCR0A = 1000 / 8;							// Timer 0: CTC Period = 1 ms
	TCCR0A = (1<<WGM01);						// Timer 0: Mode = "CTC"
	TCCR0B = (1<<CS01)|(1<<CS00);				// Timer 0: N = 64
	TIMSK0 |= (1<<OCIE0A);						// Enable Timer 0 CMPA interrupt
	
	/* Play Tune */
//	SFX_tone(FREQ_G3,150);
//	_delay_ms(150);
//	SFX_tone(FREQ_C4,150);
	
	/* Enable Master Updates */
	APP_startMode_main();
	sei();
}

void APP_startMode_main()
{
	/* Start Main */
	APP_setUpdateState(0);
	KEY_setState(0);
	LCD_generateScreen(MAINSCREEN);
	settings.mode = NONE;
	APP_setUpdateState(1);
	KEY_setState(1);
}

void APP_startMode_debug()
{
	/* Start Debug */
	APP_setUpdateState(0);
	KEY_setState(0);
	LCD_generateScreen(DEBUGSCREEN);
	settings.mode = DEBUGGING;
	APP_setUpdateState(1);
	KEY_setState(1);
}

void APP_startMode_trace()
{	
	/* Turn Updates OFF */
	APP_setUpdateState(0);
	
	/* Turn Keys OFF */
	KEY_setState(0);
	
	/* Request Initial GPS Info */
	if(!settings.isDGPSon) GPS_request_update();
	
	/* Generate Navigation Screen */
	LCD_generateScreen(TRACESCREEN);
	
	/* Update Manifest */ 
	APP_write_manifest(M_TRACE);
	
	/* Turn Keys ON */
	KEY_setState(1);
	
	/* Wait Until GPS Parsing Yields Valid Data */
	if(!settings.isDGPSon){
		do{
			while(SYS_GPS.IS_PROCESSING) ;
			if(SYS_GPS.STATUS != 'A')		GPS_request_update();
		} while(SYS_GPS.STATUS != 'A');	 
		
		/* Set Latitude and Longitude Past Values for Offset Calculation */
		for(int i = 0; i < GPS_BYTES_ASCII_LATITUDE; i++)	SYS_GPS.LATITUDE_ASCII_LAST[i] = SYS_GPS.LATITUDE_ASCII[i];
		for(int i = 0; i < GPS_BYTES_ASCII_LONGITUDE; i++)	SYS_GPS.LONGITUDE_ASCII_LAST[i] = SYS_GPS.LONGITUDE_ASCII[i];
		for(int i = 0; i < GPS_BYTES_ASCII_UTC_TIME; i++)	SYS_GPS.UTC_TIME_ASCII_LAST[i] = SYS_GPS.UTC_TIME_ASCII[i];
	}
	
	/* Write Initial Router */
	APP_write_router();
	
	/* Populate Navigation Screen */
	APP_update_trace();
	
	/* Set Mode To Tracing */
	settings.mode = TRACING;
	
	/* Turn Updates ON */
	APP_setUpdateState(1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   APP Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t APP_course2rot()
{
	return (atoi(SYS_GPS.COURSE_ASCII) + 90) % 360;
}

int16_t APP_lastSuper2rot()
{
	float tmp = ((float)(trace.pos.y - trace.sup.y))/((float)(trace.pos.x - trace.sup.x));
	tmp = atan(tmp) * 180 / M_PI;
	if(tmp < 0) tmp += 360;
	return (int16_t)tmp;
}

void APP_update_MASTER ()
{	 		
	if(!SYS_GPS.IS_PROCESSING){
		LCD_setIconState(GPSICON,0);
		switch(settings.mode){
			case NONE:
			break;
			
			case DEBUGGING:
			APP_update_debug();
			break;
			
			case TRACING:
			if(SYS_GPS.STATUS == 'D' || SYS_GPS.STATUS == 'A') APP_update_trace();
			break;
			
			case RETRACING:
			break;
		}
		
		if(settings.isDGPSon) APP_DGPS_incTime();
		else {
			if(SYS_GPS.STATUS == 'A') LCD_setIconState(GPSICON,1);
			GPS_request_update();
		}
	}	
}

void APP_update_trace()
{	
	/* If Time Has Changed */
	if(SYS_GPS.UTC_TIME_ASCII_LAST != SYS_GPS.UTC_TIME_ASCII)
	{
		/* Update UTC Pane */
		for(int i = 0; i < GPS_BYTES_ASCII_UTC_TIME; i++)	SYS_GPS.UTC_TIME_ASCII_LAST[i] = SYS_GPS.UTC_TIME_ASCII[i];
		LCD_setText(NAVSCREEN_UTC_TEXTX,NAVSCREEN_UTC_TEXTY,2,WHITE,NAVSCREEN_SCREENCOLOR);
		LCD_println_str_len(SYS_GPS.UTC_TIME_ASCII,8);  
		pencil.size = 1;	pencil.fg = OLIVE;
		LCD_print_str(SYS_GPS.UTC_DATE_ASCII);
	}

	/* If Position Has Changed */
	int latMinOff = atoi(SYS_GPS.LATITUDE_ASCII) - atoi(SYS_GPS.LATITUDE_ASCII_LAST);
	int lonMinOff = atoi(SYS_GPS.LONGITUDE_ASCII) - atoi(SYS_GPS.LONGITUDE_ASCII_LAST); 
	if(abs(latMinOff) >= NODESIZE*2 || abs(lonMinOff) >= NODESIZE*2)
	{
		/* Update Position */
		trace.pos.x += lonMinOff;	strcpy(SYS_GPS.LONGITUDE_ASCII_LAST,SYS_GPS.LONGITUDE_ASCII);
		trace.pos.y += latMinOff;	strcpy(SYS_GPS.LATITUDE_ASCII_LAST,SYS_GPS.LATITUDE_ASCII);
		
		/* Check if New Quadrant is Entered */
		if     (trace.pos.x - trace.ref.x > MAPXBOUND) { trace.quad.x++; trace.ref.x += NAVSCREEN_MAP_PANEW * D2PX; APP_write_router(); }
		else if(trace.ref.x - trace.pos.x > MAPXBOUND) { trace.quad.x--; trace.ref.x -= NAVSCREEN_MAP_PANEW * D2PX; APP_write_router(); }
		else if(trace.pos.y - trace.ref.y > MAPYBOUND) { trace.quad.y++; trace.ref.y += NAVSCREEN_MAP_PANEH * D2PX; APP_write_router(); }
		else if(trace.ref.y - trace.pos.y > MAPYBOUND) { trace.quad.y--; trace.ref.y -= NAVSCREEN_MAP_PANEH * D2PX; APP_write_router(); }
		
		/* Write New Node */
		static uint8_t nodeCount = 0;	nodeCount++;
		if(nodeCount == 10) 
		{ 		
			trace.sup.x = trace.pos.x;
			trace.sup.y = trace.pos.y;
			APP_write_node(D_SUPERNODE);
			nodeCount = 0;
		}
		else				
			APP_write_node(D_NORMALNODE);
	}
}

void APP_update_debug()
{
	/* Set Text Parameters */
	LCD_setText(DEBUGSCREEN_START_X,DEBUGSCREEN_START_Y,DEBUGSCREEN_TEXT_SIZE,DEBUGSCREEN_TEXT_COLOR,DEBUGSCREEN_SCREENCOLOR);
	
	/* Print All Parameters */
	LCD_print_str_len (SYS_GPS.UTC_TIME_ASCII,8);		LCD_print_char('\n');
	LCD_print_str (SYS_GPS.UTC_DATE_ASCII);		LCD_print_char('\n');
	LCD_print_char(SYS_GPS.STATUS);				LCD_print_char('\n');
	LCD_print_str (SYS_GPS.LATITUDE_ASCII);		LCD_print_char('\n');
	LCD_print_str (SYS_GPS.LONGITUDE_ASCII);	LCD_print_char('\n');
	LCD_print_str (SYS_GPS.SPEED_ASCII);		LCD_print_char('\n');
	LCD_print_str (SYS_GPS.COURSE_ASCII);		LCD_print_char('\n');
}

uint8_t APP_formatCard()
{
	/* Load All Data From EEPROM */
//	EEPROM_recovery();
		
	/* Load 8-bit Signature From Card */
//	LCD_setIconState(CARDICON,1);
//	APP_disk_2_buff(DISK_SIGNTURE_SECTOR,)
// LCD_setIconState(CARDICON,0);
			
	/* If Signatures Match, Load Previous Settings and Return 0 */
//	if(buff[0] == NV_USER_PREFERENCES_0){
//		settings.isDGPSon = ;				/* CHRIS PLEASE DO THIS CODE */
//		settings.mode = ;					/* Change NV_USER_PREFERENCES_0 to
//		settings.pos2px = ;					/* variable corrosponding to signiture */
//		settings.posThres = ;				/* and load variables into corrsponding */
//		settings.entryCount = ;				/* settings on the left */
//		settings.liveSector = ;
//		return 0;
//	}
	
	/* Else, Run Default Settings, Write New Signatures, and Return 1 */ 
//	else{
		LCD_setIconState(CARDICON,1);
		DISK_wipe(0,10);
		DISK_wipe(256,10);
		DISK_wipe(750,100);
		DISK_wipe(1600,500);
		LCD_setIconState(CARDICON,0);
		settings.isDGPSon = 0;
		settings.mode = NONE;
		settings.entryCount = 0;
		settings.liveSector = DAT_SECTOR;
//		byte newSig = APP_genSig();					/* CHRISTOPHER HERE TOO */
//		EEPROM_writeAll();							/* Please write a signiture generation */		
//		buff[0] = newSig;							/* function and write all settings into EEPROM space */
//		DISK_write(buff,1,DISK_SIGNTURE_SECTOR);	/* You don't need to change anything else here */
		return 1;
//	}		
}
// ASSUMES TRACE HANDLER HAS CURRENT QUAD
uint8_t APP_write_router()
{
	/* Read Router at Current Quadrant */
	uint32_t quadSector =
		trace.startSector 
		+ (trace.quad.x + QUAD_COLCOUNT / 2)
		+ (trace.quad.y + QUAD_ROWCOUNT / 2) * QUAD_COLCOUNT; 
	if(DISK_read(quadSector)) return 1;
	
	/* Clear Map Pane */
	LCD_drawRect_filled(NAVSCREEN_MAP_PANEX+1,NAVSCREEN_MAP_PANEY+1,NAVSCREEN_MAP_PANEW-2,NAVSCREEN_MAP_PANEH-2,NAVSCREEN_SCREENCOLOR);
	
	/* If Router Does NOT Exist */
	if(disk.buffIt == 0)
	{
		/* Write New Router Into Bitmap */
		DISK_loadBuff_int(D_ROUTER,DAT_TYPE_OFF);
		DISK_loadBuff_int(settings.entryCount,DAT_EID_OFF);
		DISK_loadBuff_int(settings.liveSector,DAT_ADDRN_OFF);
		if(DISK_write(quadSector)) return 1;
		
		/* Write New Origin/Reference Into Bitmap */
		APP_write_node((trace.quad.x == 0 && trace.quad.y == 0) ? D_ORIGINNODE : D_REFNODE);
	}
	
	else
	{
		/* Initialize Address/Node Iterators and Address Tracker */
		uint16_t addrIt = DAT_ADDRN_OFF;
		uint16_t nodeIt;
		uint32_t currAddr = atoi(disk.buff + addrIt);
		Vector2 readQuad;
		
		do {
			/* Read First Node and Reset Node Iterator */
			DISK_read(currAddr);
			nodeIt = 0;
			
			do {
				/* Draw Node */
				switch(atoi(disk.buff+DAT_TYPE_OFF)){
					case D_NORMALNODE:	LCD_drawCircle_filled(NAVSCREEN_MAP_X0 - atoi(disk.buff+DAT_X_OFF) / D2PX, NAVSCREEN_MAP_Y0 + atoi(disk.buff+DAT_Y_OFF) * D2PX, NODESIZE_S, NODECOLOR_NORMAL); break;
					case D_SUPERNODE:	LCD_drawCircle_filled(NAVSCREEN_MAP_X0 - atoi(disk.buff+DAT_X_OFF) / D2PX, NAVSCREEN_MAP_Y0 + atoi(disk.buff+DAT_Y_OFF) * D2PX, NODESIZE, NODECOLOR_SUPER); break;
					case D_ORIGINNODE:	LCD_drawCircle_filled(NAVSCREEN_MAP_X0, NAVSCREEN_MAP_Y0, NODESIZE, NODECOLOR_USER); break;
					default: ;
				}
			
				/* Read Next Node */
				DISK_read(currAddr + ++nodeIt);
			
				/* While Node is Within Quadrant */
				readQuad.x = atoi(disk.buff+DAT_QUADC_OFF);
				readQuad.y = atoi(disk.buff+DAT_QUADR_OFF);
			} while(disk.buffIt != 0 && (readQuad.x == trace.quad.x) && (readQuad.y == trace.quad.y));
			
			/* Read Next Address In Router */
			addrIt += DAT_ADDRN_SIZE;
			DISK_read(quadSector);
			currAddr =  atoi(disk.buff + addrIt);
		
		/* While Address is Valid */		
		} while(currAddr != 0);
		
		/* Append Live Sector to Router */
		DISK_loadBuff_int(settings.liveSector,addrIt);
		DISK_write(quadSector);
	}
	
	return 0;
}

// ASSUMES TRACE HANDLER HAS CURRENT REFERENCE AND POSITION
uint8_t APP_write_node(DataType type)
{
	/* Calculate Offset of Current Node */
	Vector2 tmpOff = {trace.ref.x - trace.pos.x, trace.ref.y - trace.pos.y};
	
	/* Buffer First Generic Payload */
	LCD_setIconState(CARDICON,1);
	DISK_loadBuff_int(type,DAT_TYPE_OFF);				// [TYPE]
	DISK_loadBuff_int(settings.entryCount,DAT_EID_OFF);	// [EID]

	/* Buffer Node-Specific Payload Section */
	switch(type)
	{
		case D_NORMALNODE:		
			/* Buffer Relative Offset */
			DISK_loadBuff_int(tmpOff.x, DAT_X_OFF);
			DISK_loadBuff_int(tmpOff.y, DAT_Y_OFF);
		
			/* Draw Node */
			LCD_drawCircle_filled(
				NAVSCREEN_MAP_X0 - tmpOff.x / D2PX, 
				NAVSCREEN_MAP_Y0 + tmpOff.y / D2PX,
				NODESIZE_S,
				NODECOLOR_NORMAL);
				
			/* Update DIRA Pane */
			LCD_setText(NAVSCREEN_DIRA_TEXTX,TFTHEIGHT-18,NAVSCREEN_DIRA_SIZE,NODECOLOR_NORMAL,NAVSCREEN_SCREENCOLOR);
			LCD_print_str("X:");	LCD_print_str(SYS_GPS.LONGITUDE_ASCII+(SYS_GPS.LONGITUDE_ASCII[0]=='-'?1:0));	LCD_print_char(SYS_GPS.EW);
			LCD_print_str("\nY:");	LCD_print_str(SYS_GPS.LATITUDE_ASCII+(SYS_GPS.LATITUDE_ASCII[0]=='-'?1:0));		LCD_print_char(SYS_GPS.NS);
			LCD_drawArrow(NAVSCREEN_DIRA_TEXTX+40,NAVSCREEN_DIRA_TEXTY,APP_course2rot(),NODECOLOR_NORMAL,NAVSCREEN_SCREENCOLOR);
			
			/* Update DIRB Pane */
			LCD_drawArrow(NAVSCREEN_DIRB_TEXTX+40,NAVSCREEN_DIRB_TEXTY,APP_lastSuper2rot(),NODECOLOR_SUPER,NAVSCREEN_SCREENCOLOR);
			
		break;
		
		case D_SUPERNODE:
			/* Buffer Relative Offset */
			DISK_loadBuff_int(tmpOff.x, DAT_X_OFF);
			DISK_loadBuff_int(tmpOff.y, DAT_Y_OFF);
		
			/* Draw Node */
			LCD_drawCircle_filled(
				NAVSCREEN_MAP_X0 - tmpOff.x / D2PX, 
				NAVSCREEN_MAP_Y0 + tmpOff.y / D2PX,
				NODESIZE,
				NODECOLOR_SUPER);

			/* Update DIRA Pane */
			LCD_setText(NAVSCREEN_DIRA_TEXTX,TFTHEIGHT-18,NAVSCREEN_DIRA_SIZE,NODECOLOR_NORMAL,NAVSCREEN_SCREENCOLOR);
			LCD_print_str("X:");	LCD_print_str(SYS_GPS.LONGITUDE_ASCII+(SYS_GPS.LONGITUDE_ASCII[0]=='-'?1:0));	LCD_print_char(SYS_GPS.EW);
			LCD_print_str("\nY:");	LCD_print_str(SYS_GPS.LATITUDE_ASCII+(SYS_GPS.LATITUDE_ASCII[0]=='-'?1:0));		LCD_print_char(SYS_GPS.NS);
			LCD_drawArrow(NAVSCREEN_DIRA_TEXTX+40,NAVSCREEN_DIRA_TEXTY,APP_course2rot(),NODECOLOR_NORMAL,NAVSCREEN_SCREENCOLOR);
			
			/* Update DIRB Pane */
			LCD_setText(NAVSCREEN_DIRB_TEXTX,TFTHEIGHT-18,NAVSCREEN_DIRB_SIZE,NODECOLOR_SUPER,NAVSCREEN_SCREENCOLOR);
			LCD_print_str("X:");	LCD_print_str(SYS_GPS.LONGITUDE_ASCII+(SYS_GPS.LONGITUDE_ASCII[0]=='-'?1:0));	LCD_print_char(SYS_GPS.EW);
			LCD_print_str("\nY:");	LCD_print_str(SYS_GPS.LATITUDE_ASCII+(SYS_GPS.LATITUDE_ASCII[0]=='-'?1:0));		LCD_print_char(SYS_GPS.NS);
			trace.sup.x = trace.pos.x;	trace.sup.y = trace.pos.y;
			//LCD_drawArrow(NAVSCREEN_DIRB_TEXTX+40,NAVSCREEN_DIRB_TEXTY,APP_lastSuper2rot(),NODECOLOR_SUPER,NAVSCREEN_SCREENCOLOR);
			LCD_drawRect_filled(NAVSCREEN_DIRB_TEXTX+40,NAVSCREEN_DIRB_TEXTY,29,29,NAVSCREEN_SCREENCOLOR);
			
		break;

		case D_ORIGINNODE:
			/* Buffer Absolute Position */
			DISK_loadBuff_int(trace.pos.x, DAT_X_OFF);
			DISK_loadBuff_int(trace.pos.y, DAT_Y_OFF);
			
			/* Draw Node */
			LCD_drawCircle_filled(
				NAVSCREEN_MAP_X0,
				NAVSCREEN_MAP_Y0,
				NODESIZE,
				NODECOLOR_USER);
				
			/* Update DIRA Pane */
			LCD_setText(NAVSCREEN_DIRA_TEXTX,TFTHEIGHT-18,NAVSCREEN_DIRA_SIZE,NODECOLOR_SUPER,NAVSCREEN_SCREENCOLOR);
			LCD_print_str("X:");	LCD_print_str(SYS_GPS.LONGITUDE_ASCII+(SYS_GPS.LONGITUDE_ASCII[0]=='-'?1:0));	LCD_print_char(SYS_GPS.EW);
			LCD_print_str("\nY:");	LCD_print_str(SYS_GPS.LATITUDE_ASCII+(SYS_GPS.LATITUDE_ASCII[0]=='-'?1:0));		LCD_print_char(SYS_GPS.NS);
			LCD_drawArrow(NAVSCREEN_DIRA_TEXTX+40,NAVSCREEN_DIRA_TEXTY,APP_course2rot(),NODECOLOR_USER,NAVSCREEN_SCREENCOLOR);
			
			/* Update DIRB Pane */
			LCD_setText(NAVSCREEN_DIRB_TEXTX,TFTHEIGHT-18,NAVSCREEN_DIRB_SIZE,NODECOLOR_SUPER,NAVSCREEN_SCREENCOLOR);
			LCD_print_str("X:");	LCD_print_str(SYS_GPS.LONGITUDE_ASCII+(SYS_GPS.LONGITUDE_ASCII[0]=='-'?1:0));	LCD_print_char(SYS_GPS.EW);
			LCD_print_str("\nY:");	LCD_print_str(SYS_GPS.LATITUDE_ASCII+(SYS_GPS.LATITUDE_ASCII[0]=='-'?1:0));		LCD_print_char(SYS_GPS.NS);
			//LCD_drawArrow(NAVSCREEN_DIRB_TEXTX+40,NAVSCREEN_DIRB_TEXTY,APP_lastSuper2rot(),NODECOLOR_USER,NAVSCREEN_SCREENCOLOR);
			LCD_drawRect_filled(NAVSCREEN_DIRB_TEXTX+40,NAVSCREEN_DIRB_TEXTY,29,29,NAVSCREEN_SCREENCOLOR);
		break;
		
		case D_REFNODE:
			/* Buffer Absolute Position */
			DISK_loadBuff_int(trace.pos.x, DAT_X_OFF);
			DISK_loadBuff_int(trace.pos.y, DAT_Y_OFF);
		break;
		
		default: ;		
	}
	
	/* Buffer Last Generic Payload */
	DISK_loadBuff_str(SYS_GPS.UTC_TIME_ASCII,DAT_TIME_OFF);	// [TIME]
	DISK_loadBuff_str(SYS_GPS.UTC_DATE_ASCII,DAT_DATE_OFF);	// [DATE]
	DISK_loadBuff_int(trace.quad.x,DAT_QUADC_OFF);			// [QUAD COLUMN]
	DISK_loadBuff_int(trace.quad.y,DAT_QUADR_OFF);			// [QUAD ROW]
	
	/* Append Node to Database */
	if(DISK_write(settings.liveSector++))	return 1; 
	LCD_setIconState(CARDICON,0);			return 0;
}

uint8_t APP_write_manifest(DataType type)
{
	/* If 'type' is M_TRACE */
	if(type == M_TRACE)
	{
		/* Update Trace Handler */
		trace.startSector = settings.liveSector;				// Save bitmap start address
		settings.liveSector += QUAD_COLCOUNT * QUAD_ROWCOUNT;	// Shift live sector past bitmap
		trace.quad.x = 0;										// Set starting quadrant
		trace.quad.y = 0;										// ...
		trace.pos.x = atoi(SYS_GPS.LONGITUDE_ASCII);			// Set starting position
		trace.pos.y = atoi(SYS_GPS.LATITUDE_ASCII);				// ...
		trace.sup.x = trace.pos.x;								// Set super position
		trace.sup.y = trace.pos.y;								// ...
		trace.ref.x = trace.pos.x;								// Set reference position
		trace.ref.y = trace.pos.y;								// ...
	}
		
	/* Write Marker Into Manifest */							// ***
	LCD_setIconState(CARDICON,1);								// ICON ON
	DISK_loadBuff_int(type,MAN_TYPE_OFF);						// [ENTRY TYPE]
	DISK_loadBuff_int(settings.liveSector,MAN_START_OFF);		// [START SECTOR]
	if(DISK_write(++settings.entryCount)) return 1;				// [..to Manifest]
	LCD_setIconState(CARDICON,0); return 0;						// ICON OFF
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//										  Debug Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

void APP_DGPS_setState(uint8_t state)
{
	if(state == 1 && !settings.isDGPSon)
	{
		/* Set All Characters To NULL By Default */
		for(int i = 0; i < sizeof(SYS_GPS); i++) *((char*)(&SYS_GPS)+i) = 0;
		
		/* Set Default Strings */
		SYS_GPS.STATUS = 'D';
		strcpy(SYS_GPS.UTC_TIME_ASCII,"00:00:00");	strcpy(SYS_GPS.UTC_TIME_ASCII_LAST,"00:00:00");
		strcpy(SYS_GPS.UTC_DATE_ASCII,"00/00/00");
		strcpy(SYS_GPS.LATITUDE_ASCII,"0");		strcpy(SYS_GPS.LATITUDE_ASCII_LAST,"0");
		strcpy(SYS_GPS.LONGITUDE_ASCII,"0");	strcpy(SYS_GPS.LONGITUDE_ASCII_LAST,"0");
		strcpy(SYS_GPS.SPEED_ASCII,"0");
		strcpy(SYS_GPS.COURSE_ASCII,"0");
		SYS_GPS.NS = 'N';
		SYS_GPS.EW = 'E';
		
		/* Enable DGPS */
		settings.isDGPSon = 1;
	}
	
	/* Else If State = 0, Disable DGPS */
	else if(state == 0)	settings.isDGPSon = 0;
}

void APP_DGPS_setTime(uint8_t h, uint8_t m, uint8_t s)
{
	/* Save Time */
//	strcpy(SYS_GPS.UTC_TIME_ASCII_LAST,SYS_GPS.UTC_TIME_ASCII);
	
	/* Set Time */
	itoa(h, SYS_GPS.UTC_TIME_ASCII,     10);
	itoa(m, SYS_GPS.UTC_TIME_ASCII + 3, 10);
	itoa(s, SYS_GPS.UTC_TIME_ASCII + 6, 10);
}

void APP_DGPS_setDate(uint8_t d, uint8_t m, uint8_t y)
{
	/* Set Date */
	itoa(d, SYS_GPS.UTC_DATE_ASCII,     10);
	itoa(m, SYS_GPS.UTC_DATE_ASCII + 3, 10);
	itoa(y, SYS_GPS.UTC_DATE_ASCII + 6, 10);
}

void APP_DGPS_setLocation(int16_t lat, int16_t lon)
{	
	/* Set Position */
	sprintf(SYS_GPS.LATITUDE_ASCII,"%-d",lat);
	sprintf(SYS_GPS.LONGITUDE_ASCII,"%-d",lon);
	
	/* Set Direction */
	SYS_GPS.NS = lat >= 0 ? 'N' : 'S';
	SYS_GPS.EW = lon >= 0 ? 'E' : 'W';
}

void APP_DGPS_setCourse(uint16_t course)
{
	/* Clear Data */
	strcpy(SYS_GPS.COURSE_ASCII,"000");
	
	/* Set Data */
	char tmp[GPS_BYTES_ASCII_COURSE];
	itoa(course,tmp,10);
	strcpy(SYS_GPS.COURSE_ASCII+(GPS_BYTES_ASCII_COURSE-strlen(tmp)-1),tmp);
}

void APP_DGPS_incTime()
{	
	/* Increment Debug GPS Time */
//	strcpy(SYS_GPS.UTC_TIME_ASCII_LAST,SYS_GPS.UTC_TIME_ASCII);
	if(++SYS_GPS.UTC_TIME_ASCII[7] > '9' ){ SYS_GPS.UTC_TIME_ASCII[7] = '0';
	if(++SYS_GPS.UTC_TIME_ASCII[6] > '5' ){ SYS_GPS.UTC_TIME_ASCII[6] = '0';
	if(++SYS_GPS.UTC_TIME_ASCII[4] > '9' ){ SYS_GPS.UTC_TIME_ASCII[4] = '0';
	if(++SYS_GPS.UTC_TIME_ASCII[3] > '5' ){ SYS_GPS.UTC_TIME_ASCII[3] = '0';
	if(++SYS_GPS.UTC_TIME_ASCII[1] > '3' ){ SYS_GPS.UTC_TIME_ASCII[1] = '0';
	if(++SYS_GPS.UTC_TIME_ASCII[0] > '2' ){ SYS_GPS.UTC_TIME_ASCII[0] = '0';
	}}}}}}
}
	
void APP_setUpdateState(uint8_t state)
{
	/* Update Timer Interrupts: */
	switch(state){
		case 0: TIMSK0 &= ~(1<<OCIE0A);	return;
		case 1: 
		TCNT0 = 0;
		TIFR0 |= (1<<OCF0A);
		TIMSK0 |= (1<<OCIE0A);	
		return;
	}
}