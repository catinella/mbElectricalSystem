/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: stringBuilder_test.c
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
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stringBuilder.h>

bool loop = true;
int  fd = -1;


void sigHandled (int signum) {
	loop = false;
	return;
}

int main (int argc, char *argv[]) {
	int ecode = 0;
	
	if (argc == 1) {
		fprintf(stderr, "ERROR! use %s <fifo-filename>\n", argv[0]);
		ecode =127;
		
	} else if (signal(SIGINT, sigHandled) == SIG_ERR || signal(SIGTERM, sigHandled) == SIG_ERR) {
		fprintf(stderr, "ERROR! Signal handles installation failed\n");
		ecode = 129;
		
	} else if ((fd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "ERROR! I cannot open the argument defined fifo (\"%s\")\n", argv[1]);
		ecode = 131;
		
	} else {
		//ndfs_t        nfds = 1;             // Number od FDS I want to monitor
		struct pollfd fdsArr[1];
		int           actives = 0;
		char          buff[BUILDER_MAXSTRINGSIZE];
		ssize_t       nrb = 0;
		
		
		fdsArr[0].fd      = fd;
		fdsArr[0].events  = POLLIN;
		
		while (loop) {
			
			actives = poll(fdsArr, 1, 10);
			
			if (actives < 0) {
				fprintf(stderr, "ERROR! poll() failes: %s\n", strerror(errno));
				ecode = 133;
				loop = false;
			
			} else if (actives == 0) {
	//			while (stringBuilder_get(buff) == 1) 
	//				printf("%d: [%s]\n", buff);
			} else {
				if ((nrb = read(fd, buff, BUILDER_MAXSTRINGSIZE)) < 0) {
					fprintf(stderr, "ERROR! read() failes: %s\n", strerror(errno));
					ecode = 135;
					loop = false;
				 
				 } else { 
					for (uint16_t t=0; t<nrb; t++) printf("%c", buff[t]);
					// uint8_t stringBuilder_put(const char *data, buffSize_t size);
				}
			}
		}
		stringBuilder_close();
		close(fd);
	}
	return(ecode);
}
