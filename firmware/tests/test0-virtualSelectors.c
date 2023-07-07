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
#include <signal.h>

#define MAX_SAMPLES   128
#define MAX_SELECTORS 16
#define POLLINGPERIOD 10000

struct selectorData {
	char pinName[3];
	bool samplePool[MAX_SAMPLES];
	uint8_t start;
	uint8_t stop;   // it is like '\0' for characters strings
};

struct selectorData db[MAX_SELECTORS];
uint8_t             db_index = 0;
bool                loop = true;
int                 fd = -1;

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
			if (x == MAX_SAMPLES)
				x = 0;
			else {
				printf("%c", db[t].samplePool[x] ? '#' : '_');
				x++;
			}
		}
		printf("\n\n");
		
		t++;
	}

	return;
}


void sigStopHandler (int signum) {
	printf("SIG#%d received\n", signum);
	loop = false;
	return;
};


int ubRead (void *bytes, uint8_t size) {
	int     tot = 0;
	uint8_t part = 1;
	void    *ptr = NULL;

	while (part > 0 && tot < size) {
		ptr = ((void*)bytes + tot);
		part = read(fd, ptr, (size - tot));
		if (part < 0) tot = -1;
		else          tot = tot + part;
	}
	return(tot);
}

//------------------------------------------------------------------------------------------------------------------------------
//                                                   M A I N
//------------------------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[]) {
	uint8_t numOfRecs = 0;
	uint8_t err = 0;

	if (argc != 2) {
		fprintf(stderr, "ERROR! use %s <filename>\n", argv[0]);
		_exit(64);
	}
	
	// File opening
	if ((fd = open(argv[1], O_RDONLY)) && fd < 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(stderr, "I cannot open the \"%s\" file: %s\n", argv[1], strerror(errno));
		_exit(127);
	}

	signal(SIGINT,  sigStopHandler);
	signal(SIGTERM, sigStopHandler);

	while (loop && err == 0) {
	
		// File locking
		if (flock(fd, LOCK_EX) < 0) {
			// ERROR!
			err = 127;
			ERRORBANNER(err)
			fprintf(
				stderr, "I cannot lock the \"%s\" file, because flock() syscall failed: %s\n", argv[0], strerror(errno)
			);
	
		// Rewind...
		} else if (lseek(fd, 0, SEEK_SET) < 0) {
			// ERROR!
			err = 127;
			ERRORBANNER(err)
			fprintf(stderr, "I cannot rewind the file: %s\n", strerror(errno));

		// Data size reading
		} else if (read(fd, &numOfRecs, 1) != 1) {
			// ERROR!
			err = 127;
			ERRORBANNER(err)
			fprintf(stderr, "I/O operation failed: %s\n", strerror(errno));
		
		} else {
			bool   value;
			char   buffer[4];

			// Data reading
			for (uint8_t t=0; t<numOfRecs; t++) {
				if (ubRead(buffer, 3) != 3) {
					// ERROR!
					err = 127;
					ERRORBANNER(err)
					fprintf(stderr, "I/O operation failed: %s\n", strerror(errno));
					break;
				
				} else {
					value = (buffer[2] == 1) ? true : false;
					buffer[2] = '\0';
					dbUpdate (buffer, value);
				}
			}
			
			// File unlocking
			if (flock(fd, LOCK_UN) < 0) {
				err = 127;
				ERRORBANNER(err)
				// ERROR!
				fprintf(
					stderr, "I cannot unlock the \"%s\" file, because flock() syscall failed: %s\n",
					argv[0], strerror(errno)
				);
			}

			dbPrinting();
		}

		usleep(POLLINGPERIOD);
	}

	// File closing
	close(fd);
	


	return((int)err);
}
