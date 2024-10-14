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
//		TTY_DATACHUNK     Number of bytes the console will try to read on every round
//		TTY_MAXLOGSIZE    Max length of the log-message to display
//		TTY_MAXLOGLINES   Max number of lines to show in the console before to drop the oldest logs
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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <regex.h>
#include <syslog.h>
#include <unistd.h>
#include <math.h>

#include <stringBuilder.h>
#include <pinToSymbol.h>
#include <pinsStorage.h>
#include <screenUtils.h>
#include <logsStorage.h>

#define TTY_DATACHUNK     16
#define TTY_MAXLOGSIZE    126

#define CONS_FACILITY     LOG_LOCAL0

#define CONS_KEEPTRACK syslog(LOG_INFO, "------->%s(%d)", __FUNCTION__, __LINE__);


// Global vars
bool loop = true;

//------------------------------------------------------------------------------------------------------------------------------
//                                                  F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

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


werror set_ttyAttribs (int fd) {
	//
	// Description:
	//	It sets the serial port's attributes
	//
	// Returned value
	//	WERRCODE_SUCCESS
	//	WERRCODE_ERROR_TTYCONFIG
	//
	struct termios tty;
	werror err = WERRCODE_SUCCESS;

	if (tcgetattr (fd, &tty) != 0) {
		// ERROR!
		fprintf(stderr, "ERROR! tcgetattr() call failed: %s\n", strerror(errno));
		err = WERRCODE_ERROR_TTYCONFIG;
	
	} else if ((cfsetospeed(&tty, TTYSPEED) < 0) || cfsetispeed(&tty, TTYSPEED) < 0) {
		// ERROR!
		fprintf(stderr, "ERROR! cfsetospeed() call failed: %s\n", strerror(errno));
		err = WERRCODE_ERROR_TTYCONFIG;
	
	} else {

		//
		// Input modes
		//
		tty.c_iflag |= (
			ICRNL  |         // No CR mapping to NL
			IGNPAR           // Ignore data parity check forA
		);
		tty.c_iflag &= ~(	
			IGNBRK |         // Break-condition is not ignored
			BRKINT |         // No interrupt signal on break
			PARMRK |         // No parity errors marking
			ISTRIP |         // All 8-bits data are processed
			INLCR  |         // No NL mapping to CR
			IGNCR  |         // CR-character ignoring disabled 
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
			err = WERRCODE_ERROR_TTYCONFIG;
		}
	}
	
	return(err);
 }



//------------------------------------------------------------------------------------------------------------------------------
//                                                     M A I N
//------------------------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[]) {
	werror      err = 0;
	struct stat buff;
	int         ttyFD;

	if (argc != 2 || *argv[1] == '\0') {
		// ERROR!
		fprintf(stderr, "ERROR! port name missing\n");
		err = WERRCODE_ERROR_MISSINGARG;

	} else if (stat(argv[1], &buff) < 0) {
		// ERROR!
		fprintf(stderr, "ERROR! \"%s\" file not found\n", argv[1]);
		err = WERRCODE_ERROR_FILENOTFOUND;

	} else if (signal(SIGTERM, sigHandler) == SIG_ERR || signal(SIGINT, sigHandler) == SIG_ERR) {
		// ERROR!
		fprintf(stderr, "ERROR! I cannot set the signal-handlers: %s\n", strerror(errno));
		err = WERRCODE_ERROR_SYSCALL;

	} else if ((ttyFD = open(argv[1], O_RDWR | O_NOCTTY | O_SYNC)) < 0) {
		// ERROR!
		fprintf(stderr, "ERROR! I cannot open the \"%s\" file: %s\n", argv[1], strerror(errno));
		err = WERRCODE_ERROR_IOOPERFAILED;

	} else if (wErrCode_isError(set_ttyAttribs(ttyFD))) {
		// ERROR!
		err = WERRCODE_ERROR_TTYCONFIG;

	} else {
		char     chunk[TTY_DATACHUNK];
		char     buff[BUILDER_MAXSTRINGSIZE];
		int      nb       = 0;                // number of received bytes
		int      value    = 0;                // PIN's value
		char     pin[PTS_PINLABSIZE];
		struct winsize ts;
		

		openlog(argv[0], LOG_NDELAY|LOG_PID, CONS_FACILITY);
		//syslog(LOG_INFO, "------------------------- [DEBUG CONSOLE START] -------------------------");
		
		while (loop && wErrCode_isError(err) == false) {
			memset(chunk, '\0', TTY_DATACHUNK);
			nb = read(ttyFD, &chunk, (TTY_DATACHUNK * sizeof(char)));
			
			if (nb < 0) {
				// ERROR!
				syslog(LOG_ERR, "ERROR! data reading operation failed: %s\n", strerror(errno));
				err = WERRCODE_ERROR_IOOPERFAILED;
				
			} else if (nb == 0) {
				// Timeout (NO new messages)
				//printf("!\n");
				
			} else if (wErrCode_isError(stringBuilder_put(chunk, nb))) {
				// ERROR!
				syslog(LOG_ERR, "ERROR(%d)! Out of memory", __LINE__);
				err = 139;
			
			} else {
				//syslog(LOG_INFO, "New data detected");
				memset((void*)buff,  '\0', BUILDER_MAXSTRINGSIZE * sizeof(char));
				
				while (stringBuilder_get(buff) == 1) {
					//syslog(LOG_INFO, "Acknowledged log: \"%s\"", buff);
							
					err = pinDef_get(buff, pin, &value);
					if (wErrCode_isError(err)) {
						// ERROR!
						syslog(LOG_ERR, "ERROR(%d)! checkPinStatus() failed", __LINE__);
	
					} else if (err == WERRCODE_SUCCESS) {
						// Keeping-track info
						if (wErrCode_isError(pinsStorage_update(pin, value))) {
							// ERROR!
							syslog(
								LOG_ERR, "ERROR(%d)! I cannot add the \"%s\" pin to the moniotored ones",
								__LINE__, pin
							);
						}
			
					} else if (err == WERRCODE_WARNING_ITNOTFOUND) {
						if (wErrCode_isError(logsStorage_add(buff)))
							// ERROR!
							syslog(LOG_ERR, "ERROR(%d)! I cannot store further new logs", __LINE__);
						else {
							//syslog(LOG_INFO, "OK the log-message has been saved");
						}
						
					} else {
						// Unknown error!
						err = WERRCODE_ERROR_GENIRIC;
					}
				}	
			}

			//------------------------------------------
			// Debug console displaying
			//------------------------------------------
			{
				system("clear");
				
				ioctl(0, TIOCGWINSZ, &ts);
				//printf ("columns %d\n", ts.ws_col);

				linePrinting('-', ts.ws_col);
				titlePrinting("D E B U G   C O N S O L E", ts.ws_col);
				
				linePrinting('=', ts.ws_col);

				pinsStorage_print(ts.ws_col);
				
				// Normal log section printing
				linePrinting('-', ts.ws_col);
				logsStorage_print(ts.ws_col);
				
				linePrinting('=', ts.ws_col);
			}
		}
		
		stringBuilder_close();
		pinDef_free();
		logsStorage_free();
		close(ttyFD);
		closelog();
	}

	return(wErrCodeToShell(err));
}
