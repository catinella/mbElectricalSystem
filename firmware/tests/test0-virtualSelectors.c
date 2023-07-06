/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test0-virtualSelectors.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
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
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <debugTools.h>
#include <sys/file.h>
#include <errno.h>


#define MAX_SAMPLES   128
#define MAX_SELECTORS 16

struct selectorData {
	char pinName[3];
	bool samplePool[MAX_SAMPLES];
	uint8_t start;
	uint8_t stop;   // it is like '\0' for characters strings
};

struct selectorData db[MAX_SELECTORS];
uint8_t             db_index = 0;
bool                loop = true;

//------------------------------------------------------------------------------------------------------------------------------
//                                         P R I V A T E   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
void dbUpdate (const char *pin, bool value) {
	//
	// Description:
	//	It stores the argument defined data in the selectors data DB.
	//	If the pin does not exixts in the db, then a new record will be added to store the PIN's samples
	//
	uint8_t t = 0;

	// Looking for the argument defined pin
	while (t < db_index && strcmp(db[t].pinName, pin) != 0) t++;

	if (t < db_index) {
		// PIN's record found
		db[t].samplePool[db[t].stop] = value;
		if (db[t].stop == MAX_SAMPLES) db[t].stop = 0;
		else                           db[t].stop++;
		if (db[t].start == db[t].stop) {
			if (db[t].start == MAX_SAMPLES) db[t].start = 0;
			else                            db[t].start++;
		}
	
	} else {
		// New record for the PIN
		strcpy(db[db_index].pinName, pin);
		db[db_index].start = 0;
		db[db_index].stop  = 1;
		db[db_index].samplePool[0] = value;
		db_index++;
	}
	
	return;
}


void dbPrinting() {
	//
	// Description:
	//	It display the DB's content
	//
	uint8_t t = 0, x = 0;

	// Display cleaning...
	printf("\e[1;1H\e[2J");

	while (t < db_index) {
		printf("%s: ", db[t].pinName);

		x = db[t].start;
		while (x != db[t].stop) {
			printf("%c", db[t].samplePool[x] ? '#' : '_');
			if (x == MAX_SAMPLES) x = 0;
			else                  x++;
		}
		printf("\n\n");
		
		t++;
	}

	return;
}

//------------------------------------------------------------------------------------------------------------------------------
//                                                   M A I N
//------------------------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[]) {
	FILE *fh;
	
	if (argc != 2) {
		fprintf(stderr, "ERROR! use %s <filename>\n", argv[0]);
		_exit(64);
	}
	
	// File opening
	fh = fopen(argv[1], "r");
	
	if (fh == NULL) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(stderr, "I cannot open the \"%s\" file\n", argv[1]);
		_exit(127);
	}

	while (loop) {
	
		// File locking
		if (flock(fileno(fh), LOCK_EX) < 0) {
			// ERROR!
			ERRORBANNER(127)
			fprintf(
				stderr, "I cannot lock the \"%s\" file, because flock() syscall failed: %s\n", argv[0], strerror(errno)
			);
			_exit(127);
		
		} else {
			size_t tempSize = 6; // [A-Z][0-7]:[0,1]\n\0
			bool   value;
			char   *buffer = (char*)malloc(tempSize * sizeof(char));
			size_t eSize = 0;
			bool   end = false;

			// Data reading
			while (end == false) {
				eSize = getline(&buffer, &tempSize, fh);    // [!] getline() returns the number of CHARS read ('\0' is not included)
				
				if (feof(fh))
					end = true;

				else if (eSize != 5 || buffer[2] != ':') {
					// ERROR!
					ERRORBANNER(127)
					if (errno == 0) 
						fprintf(stderr, "I/O operation failed or the \"%s\" file is corrupted\n", argv[0]);
					else
						fprintf(stderr, "getline() failed: %s\n", strerror(errno));

					_exit(127);
				
				} else {
					buffer[2] = '\0';
					value = (buffer[3] == '1') ? true : false;
					dbUpdate (buffer, value);
	
				}
			}
			
			free(buffer);

			// File unlocking
			if (flock(fileno(fh), LOCK_UN) < 0) {
				// ERROR!
				ERRORBANNER(127)
				fprintf(
					stderr, "I cannot unlock the \"%s\" file, because flock() syscall failed: %s\n",
					argv[0], strerror(errno)
				);
				_exit(127);
			}

			dbPrinting();
		}

		usleep(1000);
	}

	// File closing
	fclose(fh);
	


	return(0);
}
