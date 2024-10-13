/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: pinsStorage_test.c
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <screenUtils.h>
#include <pinsStorage.h>


int main() {
	int  err = 0;
	bool loop = true;
	char pinid[64], pinvalue[16];
	struct winsize ts;
	
	while (loop) {
		system("clear");
		ioctl(0, TIOCGWINSZ, &ts);
		
		linePrinting('-', ts.ws_col);
		titlePrinting("PIDs CONSOLE", ts.ws_col);
		linePrinting('-', ts.ws_col);
		pinStorage_print(ts.ws_col);
		linePrinting('=', ts.ws_col);
		printf("\n\n");
		
		memset(pinid,    '0', sizeof(pinid));
		printf("Type a pid-id or \"!\" char to exit > "); scanf("%s", pinid);
	
		if (strcmp(pinid, "!") == 0) {
			loop = false;
			
		} else {
			memset(pinvalue, '0', sizeof(pinvalue));
			printf("PIN's value > "); scanf("%s", pinvalue);
			if (wErrCode_isError(pinStorage_update(pinid, atoi(pinvalue)))) {
				// ERROR!
				err = 1;
				loop = false;
			}
		}
	}
	
	return(err);
}
