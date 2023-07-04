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
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/file.h>

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

//----------------------------------------------------------------------------------------------------------------------------------
//                                                   F U N C T I O N S
//----------------------------------------------------------------------------------------------------------------------------------
void usageMsg (const char *filename) {
	fprintf(stderr, "ERROR! use %s --file=<filename> --pin=<pin-name>:<pin-type>  [--pin=<pin-name>:<pin-type> ...]\n", filename);
	fprintf(stderr, "       pin-type: {button|switch}\n");
	fprintf(stderr, "       pin-name: [A-Z][0-9]\n");
	return;
}


uint8_t dataDumping (const char *file, const pinItem *myList) {
	//
	// Description:
	//
	//
	pinItem *ptr = (pinItem*)myList;
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
		while (ptr != NULL) {
			// [!] Remember "true" means it is connected to GND and then 0
			fprintf(fh, "%s:%d\n", ptr->pinName, ptr->status ? 0 : 1);
			ptr = ptr->next;
		}
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
		uint8_t numOfRecs = 0;
		char    filename[PATH_MAX];

		filename[0] = '\0';
		pinItem *pinsList = NULL;
		pinItem *pinsTail = NULL;

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
							if (pinsList == NULL) {
								pinsList = (pinItem*)malloc(sizeof(pinItem));
								pinsTail = pinsList;
							} else {
								pinsTail->next = (pinItem*)malloc(sizeof(pinItem));
								pinsTail = pinsTail->next;
							}
							
							strcpy(pinsTail->pinName, pinName);
							if (strcmp(pinType, "button") == 0) pinsTail->type = VS_BUTTON;
							else                                pinsTail->type = VS_SWITCH;
							pinsTail->status = false; // NOT-CONNECTED
							pinsTail->next = NULL;
							
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
			pinItem *ptr    = NULL;
			pinItem *tgtPtr = NULL;
			uint8_t t       = 0;
			uint8_t tgt     = 0;
			int     answ    = '\0';

			initscr();
			keypad(stdscr, true);

			while (answ != 'q' && err == 0) {
				erase();
				ptr = pinsList;
				t = 0;
				printw("\n+---------- Virtual Selectors ----------+\n");
				printw("|                                       |\n");
				while (ptr != NULL) {
					if (t == tgt)               {printw("| ---> "); tgtPtr = ptr;}
					else                        printw("|      ");
					if (ptr->type == VS_BUTTON) printw("(%s)", ptr->pinName);
					else                        printw("[%s]", ptr->pinName);
					printw(": %s", (ptr->status == true) ? "PUSHED  " : "RELEASED");
					printw("                   |\n");
					ptr = ptr->next;
					t++;
				}
				printw("|                                       |\n");
				printw("+---------------------------------------+\n");
				printw("|  Press RETURN to connect the selector |\n");
				printw("|           Rpess 'q' to EXIT           |\n");
				printw("+---------------------------------------+\n\n");
			
				refresh();
				
				answ = getch();

				// Arrow-UP
				if (answ == 259 && tgt > 0)
					tgt--;

				// Arrow-DOWN
				else if (answ == 258 && tgt < (numOfRecs-1))
					tgt++;

				// RETURN
				else if (answ == 10) {
					if (tgtPtr->type == VS_BUTTON)
						tgtPtr->status = true;
					else	
						tgtPtr->status = !tgtPtr->status;

					err = dataDumping(filename, pinsList);
					
					if (err == 0) {
						usleep(VS_PUSHEDTIME);

						// Button releasing...
						if (tgtPtr->type == VS_BUTTON)
							tgtPtr->status = false;

						err = dataDumping(filename, pinsList);
					
					}

					if (err != 0)
						fprintf(stderr, "ERROR! I/O operation failed while data have been saving\n");

				}

			}
			endwin();		


			//
			// Resources releasing....
			//
			pinItem *old = pinsList, *next = NULL;
			while (old != NULL) {
				next = old->next;
				free(old);
				old = next;
			}
		}
	}

	return(err);
}
