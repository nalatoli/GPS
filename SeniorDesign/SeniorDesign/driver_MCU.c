#include "header_MCU.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//										MCU Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									    MCU Driver Objects									      //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   MCU Public Functions										  //
////////////////////////////////////////////////////////////////////////////////////////////////////

Bool MCU_isNum(char c)
{
	/* Returns Whether 'c' Is A Numerical Character (0 -> 9) */
	return ((c < '0' || c > '9') ? FALSE : TRUE);
}

uint8_t MCU_strlen(char * str)
{
	/* Return Number of Characters In String */
	uint8_t count = 0;
	while(!(*str)) count++;
	return count;
}

int MCU_atoi(char * str)
{
	/* Initialize Sum to 0 and Negative Modifier to 1 */
	int sum = 0;
	int8_t negMod = 1;
	
	/* Indicate If Number is Negative */
	if(*str == '-'){
		negMod = -1;
		str++;
	} 
	
	/* Convert String to Integer */
	while(MCU_isNum(*str)){
		sum = sum * 10 + ((uint8_t)(*str)-'0');
		str++;
	}
	
	/* Return Integer */
	return sum * negMod;
}

/***************************************************************************************************
	Function: itoa
		- Places integer 'val' into 'str', including terminating character
		
***************************************************************************************************/
void MCU_itoa(int val, char * str)
{
	/* Initialize Iterator to 0 */
	uint8_t i = 0;
	
	/* Determine if String is Negative */
	if(val < 0){
		str[i] = '-';
		i++;
		val *= -1;
	}
	
	/* Load First Character */
	str[i] = (char)((val % 10) + '0');
	val /= 10;
	
	/* Load Subsequent Characters */
	while(val){
		i++;
		str[i+1] = str[i];
		str[i] = (char)((val % 10) + '0');
		val /= 10;
	}
	
	/* Teminate String */
	str[i+1] = 0;
}

///***************************************************************************************************
	//Function: strcpy
		//- Copies all characters from 'src' into 'dest', including terminating character
		//
//***************************************************************************************************/
//void MCU_strcpy(char * dest, char * src);
//
///***************************************************************************************************
	//Function: strcmp
		//- Compares 'len' characters of 'str1' and 'str2'
		//
//***************************************************************************************************/
//Bool MCU_strcmp(char * str1, char * str2, uint8_t len);

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   MCU Private Functions									  //
////////////////////////////////////////////////////////////////////////////////////////////////////
