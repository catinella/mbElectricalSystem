/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   virtualSelectors.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module has been written to be used by the test0-mbesSelector.c testing procedure. As the file-name says
//	virtualSelectors provide a set of configurable virtual selectors (switches or buttons) used to simulate the
//	motorbike's controls.
//
//	+------------------+       _____       +--------------------+
//	|                  |      /     \      |                    |
//	| virtualSelectors +---->|\_____/|---->| test0-mbesSelector |
//	|                  |     |       |     |                    |
//	+------------------+     |       |     +--------------------+
//	                         \_______/                    
//	                  
//	The read/write operation are performed asyncronousely, so flock() syscall is used to avoid data corruption
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
#define _GNU_SOURCE
#define DEBUG 1

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <debugTools.h>
#include <limits.h>
#include <poll.h>
#include <errno.h>
#include <termios.h>

#if DEBUG ==1
#include <syslog.h>
#endif

typedef enum _selectorType {
	VS_BUTTON,
	VS_SWITCH
} selectorType;

typedef struct _pinItem {
	char            pinName[3];
	selectorType    type;
	bool            status;
	struct _pinItem *next;
} pinItem;

#define VS_PUSHEDTIME 200000

#if DEBUG == 1
#define MYSYSLOG   syslog
#define MYOPENLOG  openlog(argv[0], LOG_NDELAY, LOG_LOCAL0);
#define MYCLOSELOG closelog();
#else
#define MYSYSLOG   fooFunction
#define MYOPENLOG  ;
#define MYCLOSELOG ;
#endif

//----------------------------------------------------------------------------------------------------------------------------------
//                                                   F U N C T I O N S
//----------------------------------------------------------------------------------------------------------------------------------

#if DEBUG == 1
void fooFunction (int priority, const char *format, ...) {};
#endif

void usageMsg (const char *filename) {
	fprintf(stderr, "ERROR! use %s --file=<filename> --pin=<pin-name>:<pin-type>  [--pin=<pin-name>:<pin-type> ...]\n", filename);
	fprintf(stderr, "       pin-type: {button|switch}\n");
	fprintf(stderr, "       pin-name: [A-Z][0-9]\n");
	return;
}


void dataUpdating (pinItem *myDB, uint8_t nunOfRecs) {
	for (uint8_t t=0; t<nunOfRecs; t++) 
		 if (myDB[t].type == VS_BUTTON) myDB[t].status = false;
	return;
}


uint8_t dataDumping (const char *file, const pinItem *myDB, uint8_t nunOfRecs) {
	//
	// Description:
	//	It writes the DB's content in the file used to simulate the selector devices. Yhe file has the following syntax:
	//		[A-Z][0-7]:[0-1]
	//
	uint8_t  err = 0;
	FILE     *fh = NULL;

	// File opening
	if ((fh = fopen(file, "w")) && fh == NULL)
		err = 92;
					
	// File locking
	else if (flock(fileno(fh), LOCK_EX) < 0)
		err = 94;

	// Data writing
	else {
		for (uint8_t t=0; t<nunOfRecs; t++) 
			// [!] Remember "true" means it is connected to GND and then 0
			fprintf(fh, "%s:%d\n", myDB[t].pinName, myDB[t].status ? 0 : 1);
		
		fflush(fh);

		// File unlocking
		if (flock(fileno(fh), LOCK_UN) < 0) err = 96;
						
		// File closing
		fclose(fh);
	}

	return(err);
}

//----------------------------------------------------------------------------------------------------------------------------------
//                                                           M A I N
//----------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	uint8_t err = 0;

	if (argc < 2) {
		usageMsg(argv[0]);
		err = 64;
	
	} else {
		uint8_t t;
		char    *ptr = NULL;
		char    key[16];
		char    value[16];
		char    pinName[4];
		char    pinType[16];
		char    filename[PATH_MAX];

		filename[0] = '\0';
		pinItem pinsDB[16];
		uint8_t numOfRecs = 0;

		for (t=1; t<argc; t++) {
			ptr = strchr(argv[t], '=');
			if (ptr == NULL) {
				// ERROR!
				usageMsg(argv[0]);
				err = 66;
				break;

			} else {
				strcpy(value, (ptr+1));
				*ptr = '\0';
				strcpy(key, argv[t]);

				if (strcmp(key, "--pin") == 0) {
					ptr = strchr(value, ':');
					if (ptr == NULL) {
						// ERROR!
						usageMsg(argv[0]);
						err = 68;
						break;

					} else {
						strcpy(pinType, (ptr+1));
						*ptr = '\0';
						strcpy(pinName, value);
						
						if (strcmp(pinType, "button") != 0 && strcmp(pinType, "switch") != 0) {
							// ERROR!
							usageMsg(argv[0]);
							err = 70;
							break;

						} else {
							//
							// Item creation
							//
							
							strcpy(pinsDB[numOfRecs].pinName, pinName);
							if (strcmp(pinType, "button") == 0) pinsDB[numOfRecs].type = VS_BUTTON;
							else                                pinsDB[numOfRecs].type = VS_SWITCH;
							pinsDB[numOfRecs].status = false; // NOT-CONNECTED
							
							numOfRecs++;
						}
					}
					
				} else if (strcmp(key, "--file") == 0) {
					strcpy(filename, value);

				} else {
					// ERROR!
					usageMsg(argv[0]);
					err = 72;
					break;
				}
			}
		}
	
		// The data file MUST be defined as file's argument
		if (strlen(filename) == 0) {
			usageMsg(argv[0]);
			err = 74;
		}

		if (err == 0) {
			uint8_t         t    = 0;
			char            answ = '\0';
			
			struct termios  oldt;    // Original terminal settings
			struct termios  newt;    // New terminal setting
			struct pollfd   pfd;
			struct timespec timeout;
			int             afd = 0;

			// Syslog service enabling...
			MYOPENLOG
			
			// Old terminal setting saving.....
			tcgetattr(STDIN_FILENO, &oldt);
			
			// New terminal setting
			newt = oldt;
			newt.c_lflag &= ~(ICANON | ECHO);
			tcsetattr(STDIN_FILENO, TCSANOW, &newt);
			
			pfd.events = POLLIN;
			pfd.fd     = STDIN_FILENO;

			timeout.tv_sec  = 0;
			timeout.tv_nsec = 2000000; // 2 ms
				
			while (answ != 'q' && err == 0) {
	
				// Clear screen
				printf("\e[1;1H\e[2J");

				t = 0;
				printf("\n+---------- Virtual Selectors ----------+\n");
				printf("|                                       |\n");
				for (t=0; t<numOfRecs; t++) {
					if (pinsDB[t].type == VS_BUTTON)
						printf("|  %d)  (%s)", t, pinsDB[t].pinName);
					else
						printf("|  %d)  [%s]", t, pinsDB[t].pinName);

					printf(": %s", (pinsDB[t].status == true) ? "PUSHED  " : "RELEASED");
					printf("                   |\n");
				}
				printf("|                                       |\n");
				printf("+---------------------------------------+\n");
				printf("|           Rpess 'q' to EXIT           |\n");
				printf("+---------------------------------------+\n\n");
			
				
				afd = ppoll(&pfd, 1, &timeout, NULL);

				if (afd < 0) {
					// ERROR!
					err = 78;
					ERRORBANNER(err)
					fprintf(stderr, "ppoll() syscall failed: %s\n", strerror(errno));
					
				} else if (afd == 0) {
					// Timeout
					dataUpdating(pinsDB, numOfRecs);
					err = dataDumping(filename, pinsDB, numOfRecs);
					
				} else if (read(STDIN_FILENO, &answ, sizeof(char)) != sizeof(char)) {
					// ERROR!
					err = 78;
					ERRORBANNER(err)
					fprintf(stderr, "I/O operation failed: %s\n", strerror(errno));

				} else if (answ == 'q') {
					MYSYSLOG(LOG_INFO, "Quit option has been selected\n");
					break;

				} else {
					uint8_t idx = answ - '0';

					MYSYSLOG(LOG_INFO, "Selection: %c(%d)", answ, idx);
					
					if (pinsDB[idx].type == VS_BUTTON)
						pinsDB[idx].status = true;
					else	
						pinsDB[idx].status = !(pinsDB[idx].status);

					err = dataDumping(filename, pinsDB, numOfRecs);
					
					if (err != 0) {
						// ERROR!
						ERRORBANNER(76)
						fprintf(stderr, "ERROR! I/O operation failed while data have been saving\n");
					}
				}
	
			} // *** while() loop ***


			// Original terminal setting restoring...
			tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

			MYCLOSELOG
		}
	}

	return(err);
}
