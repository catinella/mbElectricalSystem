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
//
//
//
//
//
//
//
//
//
// License:
//	KiCad Schematics distributed under the GPL-3 License.
//
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


void usageMsg (const char *filename) {
	fprintf(stderr, "ERROR! use %s --file=<filename> --pin=<pin-name>:<pin-type>  [--pin=<pin-name>:<pin-type> ...]\n", filename);
	fprintf(stderr, "       pin-type: {button|switch}\n");
	fprintf(stderr, "       pin-name: [A-Z][0-9]\n");
	return;
}


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

				} else {
					// ERROR!
					usageMsg(argv[0]);
					err = 78;
					break;
				}
			}
		}
		
		if (err == 0) {
			pinItem *ptr    = NULL;
			pinItem *tgtPtr = NULL;
			uint8_t t       = 0;
			uint8_t tgt     = 0;
			int     answ    = '\0';

			initscr();
			keypad(stdscr, true);

			while (answ != 'q') {
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
					// Connecting the selector to GND
					if (tgtPtr->type == VS_BUTTON)
						tgtPtr->status = true;
					else if (tgtPtr->status == true)
						tgtPtr->status = false;
					else
						tgtPtr->status = true;


					// File locking

					// File opening

					// data writing

					// File closing

					// File unlocking

					// Button releasing...
					if (tgtPtr->type == VS_BUTTON)
						tgtPtr->status = false;
				}

				//printf("-------------->%d\n", answ); sleep(2); break;

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
