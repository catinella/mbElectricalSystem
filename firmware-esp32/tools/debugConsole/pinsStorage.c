/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: pinsStorage.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module allows you to stores/updates and prints all the monitored pins and their values
//	
//	Dependences graph:
//	                           +------------+
//	                           |            |
//	                     +-----+ pinStorage | 
//	                     |     |            |
//	                     |     +------------+
//	                     |     | static db  |
//	                     |     +------------+
//	  +-------------+    |
//	  |             |<---+
//	  | pinToSymbol |     
//	  |             |     +----------------+
//	  +-------------+     |   header file  | 
//	  |  static db  +---->| (pins aliases) | 
//	  +-------------+     +----------------+                   A ---> B: A uses B
//	
//	
// License:
//	Copyright (C) 2023 Silvano Catinella <catinella@yahoo.com>
//
//	This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
//	License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
//	version.
//
//	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
//	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with this program. If not, see
//		<https://www.gnu.org/licenses/gpl-3.0.txt>.
//
------------------------------------------------------------------------------------------------------------------------------*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pinsStorage.h>
#include <pinToSymbol.h>


// Pins status DB item
typedef struct {
	char     pin[PTS_PINLABSIZE];
	uint32_t value;
} pinsDbItem;


//
// Global variables
//
static pinsDbItem pinsDb[MBES_MAXNUMOFPINS];
static uint8_t    counter = 0;


void fillUp (char *string, uint8_t newsz) {
	//
	// Description:
	//	This function is used to get a pre-defined size string. To achieve the result, the procedure will append
	//	enough spaces to get the argument define size
	//
	uint8_t t = 0;
	for (t = strlen(string); t < newsz; t++) string[t] = ' ';
	string[t] = '\0';
	
	return;
}

//-----------------------------------------------------------------------------------------------------------------------------
//                                       P U B L I C   F U N C T I O N S
//-----------------------------------------------------------------------------------------------------------------------------
void pinStorage_print (uint16_t screenCols) {
	//
	// Description:
	//	This function prints every recorded pins and its value formatted by columns considering the argument defined
	//	screen size
	//
	uint8_t t = 0, x = 0;
	uint8_t cols = roundf(((screenCols) / (PTS_MAXSYMSIZE + 5)) - 1);
	char    *buff = (char*)malloc(PTS_MAXSYMSIZE+5);
	
	// Minimum setting
	cols = (cols == 0) ? 1 : cols;
		
	for (x = 0; x < counter; x++) {
			
		// Symbol retriving
		if (pinToSymbol_get(buff, pinsDb[x].pin) != 1)
			strcpy(buff, pinsDb[x].pin);
				
		sprintf((buff + strlen(buff)), ":%d", pinsDb[x].value);
		fillUp(buff, PTS_MAXSYMSIZE);
		printf("%s", buff);
			
		if (t == cols) {
			printf("\n");
			t = 0;
		} else {
			printf("      ");
			t++;
		}
	}
	free(buff);
	if (t != 0) printf("\n");
	
}

werror pinStorage_update (const char *pinID, uint32_t value) {
	//
	// Description:
	//	This function updates or adds the argument defined pin and its associated value
	//
	// Returned value:
	//	WERRCODE_SUCCESS
	//	WERRCODE_ERROR_DATAOVERFLOW
	//
	werror err = WERRCODE_SUCCESS;
	uint8_t t = 0;
	
	while (t < counter) {
		if (strcmp(pinID, pinsDb[t].pin) == 0) break;
		else                                 t++;
	}
	
	if (t == counter) {
		// New pin adding...
		strcpy(pinsDb[counter].pin, pinID);
		pinsDb[counter].value = value;
		counter++;
	
	} else if (t < MBES_MAXNUMOFPINS) {
		// Updating...
		pinsDb[t].value = value;
	
	} else {
		// ERROR!
		err = WERRCODE_ERROR_DATAOVERFLOW;
	}
	
	return(err);
}
