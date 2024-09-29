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
#include <stringBuilder.h>

typedef struct _logsSetItem_t {
	char buffer[BUILDER_MAXSTRINGSIZE];
	struct _logsSetItem_t  *next;
} logsSetItem_t;

static logsSetItem_t *oldest = NULL;
static logsSetItem_t *newest = NULL;

uint8_t stringBuilder_put(const char *data, buffSize_t size) {
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
	//	0 ERROR! Out of memory
	//	1 The chars have been succesfully parsed and stored
	//
	uint8_t     	ecode = 1;
	bool      	      eolFlag = false;
	static buffSize_t bufferSize = 0;  // The buffesr size of the newest's item or 0 if the log-item is completed
	
	if (oldest == NULL) {
		oldest = (logsSetItem_t*)malloc(sizeof(logsSetItem_t));
		if (oldest == NULL)
			// ERROR! Memory full
			ecode = 0;
		else {
			*(oldest->buffer) = '\0';
			newest = oldest;
			newest->next = NULL;
		}
	}
	if (ecode) {
		for (uint8_t t=0; t<size; t++) {
			if (data[t] == '\n') {
				// Characters string assembling has been completed
				*(newest->buffer + bufferSize + t) = '\0';
				eolFlag = true;
			
			} else if ((bufferSize + t) > (BUILDER_MAXSTRINGSIZE - 3)) {
				// WARNING! Too long string
				strcpy((newest->buffer + bufferSize + t), "...");
				eolFlag = true;
			
			} else {
				// Adding a char to the buffer line
				*(newest->buffer + bufferSize + t) = data[t];
			}
		
			if (eolFlag) {
				newest->next = (logsSetItem_t*)malloc(sizeof(logsSetItem_t));
				if (newest->next == NULL) {
					// ERROR! Memory full
					ecode = 0;
					break;
				} else {
					newest = newest->next;
					bufferSize = 0;
					eolFlag = false;
					newest->next = NULL;
				}
			} else
				bufferSize += t;
		}
	}
	
	return(ecode);
}


uint8_t stringBuilder_get(char *data) {
	//
	// Decription:
	//	This function allows you to retrive the strings defined in the previousely added chras streams
	//	
	// Returned value:
	//	0: N/A
	//	1: SUCCESS!
	//	16: WARNING! No strings available
	//
	uint8_t ecode = 0;

	if (oldest != NULL && oldest != newest) {
		logsSetItem_t *ptr = oldest;
		
		strcpy(data, oldest->buffer);
		oldest = oldest->next;
		free(ptr);
	}
	
	return(ecode);
}

void stringBuilder_close() {
	//
	// Description_
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
	
