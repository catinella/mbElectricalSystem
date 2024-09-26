/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: debugConsole.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This software allows you to display the console messages and the pin status too
//
//	Testing process
//		In order to test this debug-console you need to creates a virtual serial-ports couple where they are linked to
//		eachother. To achieve this result you need the socat tool.
//		To create the ports-couple type the following command: socat -d -d pty,raw,echo=0 pty,raw,echo=0
//
//	Symbols description:
//		TTY_DATACHUNK    Number of bytes the console will try to read on every round
//		TTY_MAXLOGSIZE   Max length of the log-message to display
//		TTY_MAXLOGLINES  Max number of lines to show in the console before to drop the oldest logs
//		MBES_PINMAPFILE  Header file where every pin's symbol is defined
//		MBES_ROWMAXSIZE  Maximum length of the MBES_PINMAPFILE rows
//		MBES_MAXSYMSIZE  Maximum size of every symbol name
//		CONS_MAXPINS     Maximum number of pins to keep track. Also the max number of symbols
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
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <regex.h>
#include <syslog.h>
#include <unistd.h>

#define TTY_DATACHUNK    16
#define TTY_MAXLOGSIZE   126
#define TTY_MAXLOGLINES  16

#define MBES_PINMAPFILE  "../mbesPinsMap.h"
#define MBES_ROWMAXSIZE  256
#define MBES_MAXSYMSIZE  32

#define CONS_MAXPINS     32
#define CONS_FACILITY    LOG_LOCAL0
#define CONS_PINDEMATCH "^[A-Z][0-9]:[0-9]\\+$"
#define CONS_DEFAREGEX  "^[ \t]*#define[ \t]\\+"
#define CONS_DEFBREGEX  "^[io]_[A-Z0-9]\\+ \\+\"[A-Z0-9][0-9]\""

#define CONS_KEEPTRACK syslog(LOG_INFO, "------->%s(%d)", __FUNCTION__, __LINE__);


//
// Custom data-types
//

// Saved log item
typedef struct _logRow {
	char           message[TTY_MAXLOGSIZE];
	uint32_t       tstamp;
	struct _logRow *next;
} logRow;

// Pins-Symbols associations DB item
typedef struct {
	char pin[3];
	char symbol[32];
} ptsDbItem;

// Available commands for the Log Storadge engine
typedef enum {
	LGS_ADD,
	LGS_CLOSE,
	LGS_PRINT
} logStorage_cmd;


// Global vars
bool loop = true;

//------------------------------------------------------------------------------------------------------------------------------
//                                                  F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
	
uint8_t iatoi (int *dst, const char *src) {
	//
	// Description:
	//	Intelligent atoi() function
	//
	// Arguments:
	//	dst:  the converted number
	//	src:  the characters string version of the number
	//
	// Returned vale:
	//	0:  conversion error
	//	1:  SUCCESS
	//
	uint8_t t   = 0;
	uint8_t err = 1;
	char    tmp[16];
	
	// Zero padding removing...
	if (strlen(src) > 1) while (src[t] == '0') t++;
		 
	strcpy(tmp, (src+t));
	*dst = atoi(tmp);
	
	// Checking for atoi() error
	if (dst == 0 && (strlen(tmp) > 1 || *tmp != '0'))
		// ERROR!
		err = 0;
	
	return(err);
}

	
void linePrinting (char pattern, uint16_t colums) {
	//
	// Description:
	//	It prints a line on the screen
	//
	for (uint16_t t=0; t<colums; t++) printf("%c", pattern);
	printf("\n");
	return;
}
	
				
void titlePrinting (const char *title, uint16_t colums) {
	uint16_t padSize = (colums - strlen(title))/2;
	for (uint16_t t=0; t<padSize; t++) printf(" ");
	printf("%s\n", title);
	return;
}


uint8_t getMyEpoch(uint32_t *tstamp) {
	//
	// Description:
	//	It returns a time-stamp in milliseconds
	//
	// Returned value:
	//	0: Error
	//	1: Success
	//
	static long int tZero = 0;
	struct timeval tv = {0, 0};
	uint8_t        err = 1;

	if (gettimeofday(&tv,NULL) < 0)
		err = 0;
	else if (tZero == 0) {
		tZero = tv.tv_sec*1000 + tv.tv_usec/1000;
		*&tstamp = 0;
	} else 
		*tstamp = (tv.tv_sec*1000 + tv.tv_usec/1000) - tZero;

	return(err);
}


void sigHandler (int signum) {
	//
	// Signals handler
	//
	switch (signum) {
		case SIGTERM:
			loop = false;
			break;

		case SIGINT:
			loop = false;
			break;
	}
	return;
}


uint8_t set_ttyAttribs (int fd) {
	//
	// Description:
	//	It sets the serial port's attributes
	//
	// Returned value
	//	0: Error
	//	1: Success
	//
	struct termios tty;
	uint8_t err = 1;

	if (tcgetattr (fd, &tty) != 0) {
		// ERROR!
		fprintf(stderr, "ERROR! tcgetattr() call failed: %s\n", strerror(errno));
		err = 0;
	
	} else if ((cfsetospeed(&tty, TTY_SPEED) < 0) || cfsetispeed(&tty, TTY_SPEED) < 0) {
		// ERROR!
		fprintf(stderr, "ERROR! cfsetospeed() call failed: %s\n", strerror(errno));
		err = 0;
	
	} else {

		//
		// Input modes
		//
		tty.c_iflag |= IGNPAR; // Ignore data parity check for
		tty.c_iflag &= ~(	
			IGNBRK |         // Break-condition is not ignored
			BRKINT |         // No interrupt signal on break
			PARMRK |         // No parity errors marking
			ISTRIP |         // All 8-bits data are processed
			INLCR  |         // No NL mapping to CR
			IGNCR  |         // CR-character ignoring disabled 
			ICRNL  |         // No CR mapping to NL
			IXON   |         // No input XON
			IXANY  |         // NO output suspending support
			IXOFF            // NO input XOFF
		);


		//
		// Control modes
		//	If a modem disconnect is detected by the terminal interface for a controlling terminal, and if CLOCAL is not
		//	set in the c_cflag field for the terminal, the SIGHUP signal shall be sent to the controlling process for
		//	which the terminal is the controlling terminal.
		//
		tty.c_lflag &= ~(
			CSIZE   |        // Number of bits transmitted or received per byte
			PARENB  |        // Parity generation/check disabling
			PARODD  |        // Odd parity disabled (??)
//			CRTSCTS |        // ???
			CSTOPB           // One stop bit
		);
		tty.c_cflag |= (
			CS8     |        // 8-bit data field length
			CLOCAL  |        // Modem status lines ignoring
			CREAD            // Data reading enabling
		);


		//
		// Local modes
		//	In canonical mode input bytes are assembled into lines, and erase and kill processing shall occur. For this
		//	reason this mode will be disabled
		//
		tty.c_lflag &= ~(
			ISIG    |        // (INTR, QUIT, and SUSP) signals disabling 
			ICANON  |        // No canonical input
			ECHO    |        // NO echo
			ECHOE   |        // No character ereasing control
			ECHOK   |        // ""
			IEXTEN  |        // No extended functions
			ECHONL           // No new-line echo
		);


		//
		// Output modes
		//
		tty.c_oflag &= ~OPOST; // Output disabling


		//
		// Terminal special characters array
		//
		tty.c_cc[VMIN]  = 0;   // Minimum number of characters for noncanonical read (MIN)
		tty.c_cc[VTIME] = 5;   // Timeout in deciseconds for noncanonical read (TIME)


		if (tcsetattr (fd, TCSANOW, &tty) < 0) {
			// ERROR!
			fprintf(stderr, "ERROR! tcsetattr() call failed: %s\n", strerror(errno));
			err = 0;
		}
	}
	
	return(err);
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
		
		// Timestamp
		if (getMyEpoch(&item->tstamp) == 0) {
			// ERROR!
			fprintf(stderr, "ERROR! System-time retriving operation failed: %s\n", strerror(errno));
		}
	}

	return(newObj);
}


uint8_t logAreaStorage (logStorage_cmd cmd, const char *logMsg) {
	//
	// Description:
	//	This function manages the log storage and the log display area
	//	The storage is a tipycal dynamically allocated ring list
	//
	//
	static logRow   *newest = NULL, *oldest = NULL;
	static uint16_t logCounter = 0;
	static bool     ringFlag = false;
	uint8_t         err = 1;

	if (cmd == LGS_ADD) {
		if (logCounter < TTY_MAXLOGLINES) {
			newest = new_logRow(newest);
			if (newest == NULL) {
				// ERROR!
				err = 0;
				syslog(LOG_ERR, "ERROR(%d)! new_logRow() failed", __LINE__);
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
		//syslog(LOG_INFO, "(addr=%p) : %s", newest, newest->message);
	
	} else if (cmd == LGS_CLOSE) {
		logRow *ptr = NULL;
		while (oldest != newest) {
			ptr = oldest;
			oldest = oldest->next;
			free(ptr);
		}
		if (oldest != NULL) free(oldest);

	} else if (cmd == LGS_PRINT) {
		//
		// Data printing mode
		//
		logRow *ptr = oldest;
		if (ptr != NULL) {
			while (ptr != newest) {
				printf("%5d: %s\n", ptr->tstamp, ptr->message);
				ptr = ptr->next;
			}
			if (newest != NULL)
				printf("%5d: %s\n", newest->tstamp, newest->message);
		} else
			printf("\n\n\n       EMPTY!!\n\n\n");
	}


	return(err);
}
		
uint8_t pinToSymbol (char *symbol, const char *pin) {
	//
	// Description:
	//	This function find the symbol associated to the argument defined pin and write it in the "symbol" argument.
	//	During the initialization phase, the procedure reads the mbesPinsMap.h file as a text one, and stores all
	//	pin-symbol associations inside its static array.
	//
	// Arguments:
	//	symbol:  It is used to store symbol you are looking for
	// 	pin:     The pin associated to the symbol
	//	
	// Returned value:
	//	0:       ERROR! I cannot compile the regex or the map-file does not exist
	//	1:       SUCCESS!
	//	16:      WARNING! No symbol has been associated to the pin
	//
	static bool      initFlag = false;
	static ptsDbItem ptsDb[CONS_MAXPINS];
	uint8_t          err = 1;
	regex_t          reegex_a, reegex_b;   // Regexes
	int              ea = 0,   eb = 0;     // Regexes error codes

	if (initFlag == false) {
		//
		// Initialization procedure
		//
		
		// DB cleaning
		for (uint8_t t=0; t<CONS_MAXPINS; t++) {
			ptsDb[t].pin[0]    = '\0';
			ptsDb[t].symbol[0] = '\0';
		}
		
		if (
			(ea = regcomp(&reegex_a, CONS_DEFAREGEX, 0)) == 0 &&
			(eb = regcomp(&reegex_b, CONS_DEFBREGEX, 0)) == 0
		) { 
			FILE   *FH = NULL;
			size_t size = MBES_ROWMAXSIZE;
			char   *row = (char*)malloc(size * sizeof(char)); // getline() requires dinamically allocated memory!!
			
			if ((FH = fopen(MBES_PINMAPFILE, "r")) == NULL) {
				// ERROR!
				fprintf(stderr, "ERROR! I cannot open the \"%s\" file: %s\n", MBES_PINMAPFILE, strerror(errno));
				err = 0;
		
			} else {
				regmatch_t pmatch[3]; // Up to 3 sub-expressions
				char    tmp[MBES_ROWMAXSIZE];
				uint8_t t = 0, x = 0, st = 0;
				uint8_t dbIndex = 0;
				
				while (feof(FH) == 0 && err == 1) {
					if (getline(&row, &size, FH) < 0 && errno != 0) {
						// ERROR!
						fprintf(stderr, "ERROR! data readingfailed: %s\n", strerror(errno));
						err = 0;
						break;
						
					} else if (regexec(&reegex_a, row, 3, pmatch, 0) == 0) {
						strcpy (tmp, (row + pmatch[0].rm_eo));
						if (regexec(&reegex_b, tmp, 3, pmatch, 0) == 0) {
							*(tmp+pmatch[0].rm_eo+1) = '\0';
							t = 0; x = 0; st = 0;
							while (tmp[t] != '\0') {
								if (st == 0) {
									if (tmp[t] != ' ' && tmp[t] != '\t') {
										(ptsDb[dbIndex].symbol)[x] = tmp[t];
										x++;
									} else {
										(ptsDb[dbIndex].symbol)[x] = '\0';
										st = 1;
										x = 0;
									}
								} else if (st == 1) {
									if (tmp[t] == '"') st = 2;
									
								} else if (st == 2) {
									if (tmp[t] != '"') {
										ptsDb[dbIndex].pin[x] = tmp[t];
										x++;
									} else {
										ptsDb[dbIndex].pin[x] = '\0';
										break;
									}
								}
								t++;
							}
							dbIndex++;
						}
					}
				}
/*				
				// The following lines-code block shows you the ptsDb content  (it is just for debug)
				{
					uint8_t dbIndex = 0;
					while (ptsDb[dbIndex].pin[0] != '\0' && dbIndex < CONS_MAXPINS) {
						printf("%s (%s)\n", ptsDb[dbIndex].symbol, ptsDb[dbIndex].pin);
						dbIndex++;
					}
				}
*/
				if (err == 1) initFlag = true;
				
				fclose(FH);
			}
			free(row);
			regfree(&reegex_a);
			regfree(&reegex_b);
		} else {
			// ERROR!
			char regErrBuff[128];
			if (ea != 0) {
				regerror(errno, &reegex_a, regErrBuff, 128);
				syslog(LOG_ERR, "ERROR! I cannot compile the \"%s\" regex: %s\n", CONS_DEFAREGEX, regErrBuff);
			}
			if (eb != 0) {
				regerror(errno, &reegex_b, regErrBuff, 128);
				syslog(LOG_ERR, "ERROR! I cannot compile the \"%s\" regex: %s\n", CONS_DEFBREGEX, regErrBuff);
			}
			
		}
	}
	
	if (initFlag == true) {
		uint8_t dbIndex = 0;
		
		err = 16;
		while (ptsDb[dbIndex].pin[0] != '\0' && dbIndex < CONS_MAXPINS) {
			if (strcmp(ptsDb[dbIndex].pin, pin) == 0) {
				err = 1;
				strcpy(symbol, ptsDb[dbIndex].symbol);
				break;
			} else {
				dbIndex++;
			}
		}
	}
	return(err);
}


uint8_t pinAreaStorage (logStorage_cmd cmd, const char *logMsg) {
	//
	// Description:
	//
	//
	uint8_t err = 1;
	
	if (cmd == LGS_ADD) {

	} else if (cmd == LGS_CLOSE) {

	} else if (cmd == LGS_PRINT) {
		// TODO: rows/colums calculating
		// 
	}
	return(err);
}


uint8_t checkPinStatus (char *pin, const char *log, int *value) {
	//
	// Description:
	//	This function checks for special-log syntax inside the argument defined (log) string.
	//	The special ones are used to keep track of the pins' value, and they have to respect the following syntax:
	//		<port><pin-number>:<int value> // port=<A-Z>, pin=<0-9>, value=<0..n>
	//
	// Arguments:
	//	log:    The received log message
	//	pin:    The memory area where the pin-id will be stored
	//	value:  The area where the pin'svalue will be stored. It can be an ADC result, too.
	//
	// Returned value:
	//	0:  ERROR!
	//	1:  It is a pin status information
	//
	static regex_t regx;
	static bool    initFlag = false;
	uint8_t        err = 1;
	
	if (initFlag == false) {
		if (regcomp(&regx, CONS_PINDEMATCH, 0) == 0) {
			initFlag = true;
			syslog(LOG_INFO, "OK: \"%s\" regex has been compiled", CONS_PINDEMATCH);
		} else {
			// ERROR!
			char regErrBuff[128];
			regerror(errno, &regx, regErrBuff, 128);
			syslog(LOG_ERR, "ERROR(%d)! I cannot compile the regex: %s\n", __LINE__, regErrBuff);
			err = 0;
		}
	} 
	
	if (initFlag) {
		regmatch_t pmatch[3];
		
		if (regexec(&regx, log, 3, pmatch, 0) == 0) {
			CONS_KEEPTRACK
			strncpy(pin, log, 2);
			pin[2] = '\0';
			if (iatoi(value, (log+3)) == 0) {
				// ERROR!
				syslog(LOG_ERR, "ERROR(%d)! atoi() failed\n", __LINE__);
				err = 0;
			}
		} else
			// It is a normal log message
			err = 4;
			
	}
	
	return(err);
}

//------------------------------------------------------------------------------------------------------------------------------
//                                                     M A I N
//------------------------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[]) {
	uint8_t     err = 0;
	struct stat buff;
	int         ttyFD;

/*
	//
	// Enable the followin scope, to test the pinToSimbol() function
	//
	{
		char symbol[MBES_MAXSYMSIZE];
		char pin[3];
		uint8_t te = 0;
		
		for (int x=0; x<5; x++) {
			for (int t=0; t<8; t++) {
				switch (x) {
					case 0:
						sprintf(pin, "A%d", t);
					break;
					
					case 1:
						sprintf(pin, "B%d", t);
					break;
					
					case 2:
						sprintf(pin, "C%d", t);
					break;
					
					case 3:
						sprintf(pin, "D%d", t);
					break;
					
					case 4:
						sprintf(pin, "0%d", t);
					break;
				}
				te = pinToSymbol(symbol, pin);
				if (te == 0) {
					fprintf(stderr, "ERROR! pinToSymbol() call failed\n");
					break;
				} else if (te == 1) {
					printf("%s (%s)\n", symbol, pin);
				} else {
					fprintf(stderr, "WARNING! No symbol defined for %s\n", pin);
				}
			}
		}
	}

*/
	if (argc != 2 || *argv[1] == '\0') {
		// ERROR!
		fprintf(stderr, "ERROR! port name missing\n");
		err = 127;

	} else if (stat(argv[1], &buff) < 0) {
		// ERROR!
		fprintf(stderr, "ERROR! \"%s\" file not found\n", argv[1]);
		err = 129;

	} else if (signal(SIGTERM, sigHandler) == SIG_ERR || signal(SIGINT, sigHandler) == SIG_ERR) {
		// ERROR!
		fprintf(stderr, "ERROR! I cannot set the signal-handlers: %s\n", strerror(errno));
		err = 131;

	} else if ((ttyFD = open(argv[1], O_RDWR | O_NOCTTY | O_SYNC)) < 0) {
		// ERROR!
		fprintf(stderr, "ERROR! I cannot open the \"%s\" file: %s\n", argv[1], strerror(errno));
		err = 133;

	} else if (set_ttyAttribs(ttyFD) == 0) {
		// ERROR!
		err = 135;

	} else {
		char     chunk[TTY_DATACHUNK];
		char     buff[TTY_MAXLOGSIZE];
		int      nb       = 0;                // number of received bytes
		char     *lPtr    = NULL;             // Left side pointer
		char     *rPtr    = NULL;             // Right side pointer
		uint8_t  pSize    = 0;
		uint8_t  buffIndx = 0;                // Database's index
		bool     tooLong  = false;            // Flag to indicate too-long messages
		bool     nlFlag   = false;            // When it is true, one or more end-of-line chars are present
		int      value    = 0;                // PIN's value
		char     pin[3]   = {'\0','\0','\0'}; // PIN-id
		

		openlog(argv[0], LOG_NDELAY|LOG_PID, CONS_FACILITY);
		syslog(LOG_INFO, "------------------------- [DEBUG CONSOLE START] -------------------------");

		memset(buff,  '\0', TTY_MAXLOGSIZE);
		
		while (loop) {
			memset(chunk, '\0', TTY_DATACHUNK);
			nb = read(ttyFD, &chunk, (TTY_DATACHUNK * sizeof(char)));
			
			if (nb < 0) {
				// ERROR!
				syslog(LOG_ERR, "ERROR! data reading operation failed: %s\n", strerror(errno));
				loop = 0;
				err = 137;
				
			} else if (nb == 0) {
				// Timeout (NO new messages)
				//printf("!\n");
				
			} else {
				// Starting conditions
				lPtr = chunk; rPtr = NULL; nlFlag = true;

				syslog(LOG_INFO, "New data detected");
				while (nlFlag) {
					if ((rPtr = strchr(lPtr, '\n')) == NULL) {
						if (tooLong == false) {
							pSize = abs(chunk + nb/sizeof(char) - lPtr);
							if ((pSize + buffIndx + 3) > TTY_MAXLOGSIZE) {
								strcpy((buff + buffIndx), "...");
								tooLong = true;
							} else { 
								strncpy((buff + buffIndx), lPtr, pSize);
							}
							buffIndx += pSize;
						}
						nlFlag = false;

					} else {
						if (tooLong == false) {
							pSize = abs(rPtr - lPtr);
							if ((pSize + buffIndx + 3) > TTY_MAXLOGSIZE) {
								strcpy((buff + buffIndx), "...");
								tooLong = true;
						
							} else {
								strncpy((buff + buffIndx), lPtr, pSize);
								*(buff + buffIndx + pSize) = '\0';
								syslog(LOG_INFO, "Acknowledged log: \"%s\"", buff);
								
								err = checkPinStatus(pin, buff, &value);
								if (err == 0) {
									// ERROR!
									syslog(LOG_ERR, "ERROR(%d)! checkPinStatus() failed", __LINE__);
			
								} else if (err == 1) {
									// Keeping-track info
									if (pinAreaStorage (LGS_ADD, buff) == 1) {
									} else {
										// ERROR!
										syslog(LOG_ERR, "ERROR(%d)! pinAreaStorage(ADD) failed", __LINE__);
									}
			
								} else {
									if (logAreaStorage(LGS_ADD, buff) == 0)
										// ERROR!
										syslog(LOG_ERR, "ERROR(%d)! logAreaStorage(ADD) failed", __LINE__);
									else {
										syslog(LOG_INFO, "OK the log-message has been saved");
									}
								};
								
								lPtr = rPtr + 1;
							}
						}

						memset(buff, '\0', TTY_MAXLOGSIZE);
						buffIndx = 0;
						tooLong = false;
					}
				}	
					
				//------------------------------------------
				// Debug console displaying
				//------------------------------------------
				{
					struct winsize ts;
					
					CONS_KEEPTRACK
					system("clear");
					
					ioctl(0, TIOCGWINSZ, &ts);
					//printf ("columns %d\n", ts.ws_col);
	
					linePrinting('-', ts.ws_col);
					titlePrinting("D E B U G   C O N S O L E", ts.ws_col);
					
					linePrinting('=', ts.ws_col);
	
					// TODO: print PINs status report
					
					// Normal log section printing
					linePrinting('-', ts.ws_col);
					logAreaStorage(LGS_PRINT, NULL);
					
					linePrinting('=', ts.ws_col);
				}
			}
		}

		close(ttyFD);
		closelog();
	}

	return(err);
}
