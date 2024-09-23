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
// 
//	Modem control
//		If a modem disconnect is detected by the terminal interface for a controlling terminal, and if CLOCAL is not
//		set in the c_cflag field for the terminal, the SIGHUP signal shall be sent to the controlling process for
//		which the terminal is the controlling terminal.
//
//
//	Canonical mode
//		In this mode input processing, input bytes are assembled into lines, and erase and kill processing shall
//		occur.
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
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


// Global vars
bool loop = true;

//------------------------------------------------------------------------------------------------------------------------------
//                                                  F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
void sigHandler (int signum) {
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
	struct termios tty;
	uint8_t err = 1;

	if (tcgetattr (fd, &tty) != 0) {
		// ERROR!
		fprintf(stderr, "ERROR! tcgetattr() call failed: %s\n", strerror(errno));
		err = 0;
	
	} else if ((cfsetospeed(&tty, TTYSPEED) < 0) || cfsetispeed(&tty, TTYSPEED) < 0) {
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
//------------------------------------------------------------------------------------------------------------------------------
//                                                     M A I N
//------------------------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[]) {
	uint8_t     err = 0;
	struct stat buff;
	int         ttyFD;


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
		char buff;
		int  ch = 0;

		while (read(ttyFD, &buff, 1) >= 0 && loop) 
			fprintf(stderr, "%c", buff);

		close(ttyFD);
	}
	
	return(err);
}
