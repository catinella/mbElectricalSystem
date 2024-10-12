/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: stringBuilder.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module allows you to receive partial strings and collect them to build a sequence of single strings
//	Each string is limited by the BUILDER_ENDOFDATA character
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
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stringBuilder.h>


typedef struct _logsSetItem_t {
	char buffer[BUILDER_MAXSTRINGSIZE];
	struct _logsSetItem_t  *next;
} logsSetItem_t;

typedef enum {
	BUILDER_NORMAL,
	BUILDER_OVRFLOW,
	BUILDER_ESCSEQ
} parser_FSM_t;

static logsSetItem_t *oldest = NULL;
static logsSetItem_t *newest = NULL;

//------------------------------------------------------------------------------------------------------------------------------
//                                       P R I V A T E   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
/*
static void partPrint (const char *string, uint16_t size) {
	//
	// Description:
	//	It prints the argument defined part of the string. (IT IS JUST A DEBUG TOOL)
	//
	for (uint16_t t=0; t<size; t++) printf("%c", string[t]);
	printf("\n");
	return;
}
*/


static bool checkForValidData (const char *string) {
	//
	// Description:
	//	It is used to prevent empty chasracters string
	//
	// Returned value:
	//	TRUE   OK! The string is valid
	//	TALSE  WARNING! It is an empty string
	//
	uint8_t size  = strlen(string);
	bool    valid = false;
		
	for (uint8_t t=0; t<size; t++) {
		if (string[t] != ' ') {
			valid = true;
			break;
		}
	}
	return(valid);
}


#if BUILDER_NOSCAPECODES == 1
static bool chInSet (char ch, const char set[]) {
	//
	// Description:
	//	It returns true value if the argument defined character belongs to the other arg specified set
	//
	bool    out = false;
	uint8_t t = 0;
	while (set[t] != '\0' && out == false) {
		if (set[t] == ch) out = true;
		t++;
	}
	return(out);
}
#endif
//------------------------------------------------------------------------------------------------------------------------------
//                                        P U B L I C   F U N C T I O N S 
//------------------------------------------------------------------------------------------------------------------------------
werror stringBuilder_put(const char *data, buffSize_t size) {
	//
	// Decription:
	//	Use this function to store/parse new characters stream. It can contains many strings definitions or no one
	//	if it is just a part of a string
	//
	// Arguments:
	//	data:   characters stream
	//	size    the size of the stream (remember, '\0' can be missed in the stream)
	//
	// Returned value
	//	WERRCODE_SUCCESS
	//	WERRCODE_ERROR_OUTOFMEM
	//
	werror              ecode = WERRCODE_SUCCESS;
	bool                eolFlag = false;
	static buffSize_t   bufferSize = 0;  // The buffesr size of the newest's item or 0 if the log-item is completed
	static parser_FSM_t fsm = BUILDER_NORMAL;
	
	if (oldest == NULL) {
		oldest = (logsSetItem_t*)malloc(sizeof(logsSetItem_t));
		memset(oldest->buffer, '\0', sizeof(oldest->buffer));
		if (oldest == NULL)
			// ERROR!
			ecode = WERRCODE_ERROR_OUTOFMEM;
			
		else {
			*(oldest->buffer) = '\0';
			newest = oldest;
			newest->next = NULL;
		}
	}
	if (ecode) {
		uint16_t     t = 0, x = 0;
		
		while (t < size) {
			if (fsm == BUILDER_NORMAL) {
				//
				// Row assembling....
				//
				if (data[t] == '\n') {
					*(newest->buffer + bufferSize + x) = '\0';
					eolFlag = true;
#if BUILDER_NOSCAPECODES == 1
				} else if (data[t] == 27  ) {
					fsm = BUILDER_ESCSEQ;
#endif
				} else if ((bufferSize + x) > (BUILDER_MAXSTRINGSIZE - 3)) {
					fsm = BUILDER_OVRFLOW;
					*(newest->buffer + bufferSize + x) = '\0';
					eolFlag = true;
					
				} else {
					// Adding a char to the buffer line
					*(newest->buffer + bufferSize + x) = data[t];
					//partPrint(newest->buffer, (bufferSize+x+1));
					x++;
				}
		
		
			} else if (fsm == BUILDER_OVRFLOW) {
				//
				// Size overflow even detected I wait for the end of the row
				//
				if (data[t] == '\n') fsm = BUILDER_NORMAL;
				
		
#if BUILDER_NOSCAPECODES == 1
			} else if (fsm == BUILDER_ESCSEQ) {
				//
				// Escape char has been detected, I wait for the end of sequence
				//
				if (isdigit(data[t]) == 0 && chInSet(data[t], ";[]m") == false) {
					fsm = BUILDER_NORMAL;
					t--;
				}
#endif
			}
	
			
			if (eolFlag) {
				if (checkForValidData(newest->buffer)) {
					newest->next = (logsSetItem_t*)malloc(sizeof(logsSetItem_t));
				
					if (newest->next == NULL) {
						// ERROR!
						ecode = WERRCODE_ERROR_OUTOFMEM;
						break;
					} else
						newest = newest->next;
				}
				memset(newest->buffer, '\0', sizeof(newest->buffer));
				eolFlag = false;
				newest->next = NULL;
				bufferSize = 0;
				x = 0;
				
			}
			
			t++;
		}
		bufferSize += x;
		*(newest->buffer + bufferSize) = '\0';
	}
	
	return(ecode);
}


werror stringBuilder_get(char *data) {
	//
	// Decription:
	//	This function allows you to retrive the strings defined in the previousely added chras streams
	//	
	// Returned value:
	//	WERRCODE_SUCCESS
	//	WERRCODE_WARNING_EMPTYLST
	//
	werror ecode = WERRCODE_WARNING_EMPTYLST;

	if (oldest != NULL && oldest != newest) {
		logsSetItem_t *ptr = oldest;
		strcpy(data, oldest->buffer);
		oldest = oldest->next;
		free(ptr);
		ecode = WERRCODE_SUCCESS;
	}
	
	return(ecode);
}

void stringBuilder_close() {
	//
	// Description:
	//	It releases all in-use system resources
	//
	logsSetItem_t *ptr = oldest;
	while (ptr != NULL) {
		ptr = ptr->next;
		free(oldest);
		oldest = ptr;
	}
	return;
}
	
