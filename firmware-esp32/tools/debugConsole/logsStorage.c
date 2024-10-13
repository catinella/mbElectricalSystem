/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: logsStorage.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module provides a ring buffer where you can store the log messages. They will be saved with their time-stamp
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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <logsStorage.h>


static logRow *newest = NULL;
static logRow *oldest = NULL;


werror getMyEpoch(uint32_t *tstamp) {
	//
	// Description:
	//	It returns a time-stamp in milliseconds
	//
	// Returned value:
	//	WERRCODE_SUCCESS            Success
	//	WERRCODE_ERROR_TIMESYNC     gettimeofday() failed
	//
	werror         err = WERRCODE_SUCCESS;
	static long int tZero = 0;
	struct timeval tv = {0, 0};

	if (gettimeofday(&tv,NULL) < 0)
		// ERROR!
		err = WERRCODE_ERROR_TIMESYNC;
		
	else if (tZero == 0) {
		tZero = tv.tv_sec*1000 + tv.tv_usec/1000;
		*tstamp = 0;
		
	} else 
		*tstamp = (tv.tv_sec*1000 + tv.tv_usec/1000) - tZero;
	
	return(err);
}


void printSingleMsg (const logRow *item, uint16_t cols) {
	//
	// Description:
	//	It prints a log message in the console's log-section. But if the message is too much long, then the function
	//	will trunc the message is a nice way
	//
	printf("%5d: ", item->tstamp);
	if (strlen(item->message) > (cols - 10)) {
		for (uint16_t x=0; x<(cols - 10); x++) printf("%c", item->message[x]);
		printf("...\n");
	} else 
		printf("%s\n", item->message);
		
	return;
}


logRow* new_logRow (logRow *item) {
	//
	// Description:
	//	This function creates a new object, links it to the argument defined object, and sets the log timestamp field
	//
	// Returned value:
	//	NULL:         malloc() failed
	//	<valid addr>: The new-oject's address
	//
	logRow *newObj = (logRow*)malloc(sizeof(logRow));

	if (newObj != NULL) {
		// Object creation
		newObj->message[0] = '\0';
		newObj->tstamp     = 0;
		newObj->next       = NULL;

		// Object linking
		if (item != NULL) item->next = newObj;
	}
	
	return(newObj);
}

//-----------------------------------------------------------------------------------------------------------------------------
//                                       P U B L I C   F U N C T I O N S
//-----------------------------------------------------------------------------------------------------------------------------
werror logsStorage_add (const char *logMsg) {
	//
	// Description:
	//
	// Returned value:
	//	WERRCODE_SUCCESS
	//	WERRCODE_ERROR_OUTOFMEMORY
	//
	werror          err = WERRCODE_SUCCESS;
	static uint16_t logCounter = 0;
	static bool     ringFlag = false;

	if (logCounter < TTY_MAXLOGLINES) {
		newest = new_logRow(newest);
		if (newest == NULL) {
			// ERROR!
			err = WERRCODE_ERROR_OUTOFMEMORY;
			fprintf(stderr, "ERROR(%d)! new_logRow() failed", __LINE__);
			
		} else {
			if (logCounter == 0) oldest = newest;
			logCounter++;
		}

	} else {
		// It closes the ring structure
		if (ringFlag == false) {
			newest->next = oldest;
			ringFlag = true;
		}
		newest = oldest;
		oldest = oldest->next ;
	}
	strcpy(newest->message, logMsg);
		
	// Timestamp
	if (getMyEpoch(&(newest->tstamp)) == 0) {
		// ERROR!
		fprintf(stderr, "ERROR! System-time retriving operation failed: %s\n", strerror(errno));
	}

	return(err);
}


void logsStorage_free() {
	//
	// Description:
	//	It release all dinamically allocated memory
	//
	logRow *ptr = NULL;
	while (oldest != newest) {
		ptr = oldest;
		oldest = oldest->next;
		free(ptr);
	}
	if (oldest != NULL) free(oldest);
	
	return;
}


void logsStorage_print (uint16_t cols) {
	//
	// Description:
	//	It prints all the archived logs
	//
	logRow *ptr = oldest;
		if (ptr != NULL) {
		while (ptr != newest) {
			printSingleMsg(ptr, cols);
			ptr = ptr->next;
		}
		printSingleMsg(newest, cols);
		
	} else
		printf("\n\n\n       EMPTY!!\n\n\n");
	
	return;
}
