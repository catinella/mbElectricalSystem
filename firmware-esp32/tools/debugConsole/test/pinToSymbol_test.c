/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: pinToSymbol_test.h
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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pinToSymbol.h>

void usageErr (const char *file) {
	fprintf(stderr, "ERROR! Use: %s -p <pin-id> | -d \"<pin definition>\"\n", file);
}

int main (int argc, char *argv[]) {
	werror wecode = WERRCODE_SUCCESS;

	if (argc == 1) {
		usageErr(argv[0]);
		wecode = WERRCODE_ERROR_ILLEGALARG;
	
	} else {
		char   pinid[PTS_MAXSYMSIZE];
		char   string[PTS_MAXSYMSIZE];
		int    opt = 0;

		memset(pinid, '\0', sizeof(pinid));
		memset(string, '\0', sizeof(string));

		while ((opt = getopt(argc, argv, "p:d:")) != -1) {
			switch (opt) {
				case 'p':
					strcpy(pinid, optarg);
					break;
				case 'd':
					strcpy(string, optarg);
					break;
				default:
					usageErr(argv[0]);
					wecode = WERRCODE_ERROR_ILLEGALARG;
					break;
			}
		}

		if (wErrCode_isSuccess(wecode)) {
			char alias[PTS_PINLABSIZE];
			int  value = 0;

			memset(alias, '\0', sizeof(alias));

			if (*pinid != '\0') {
				wecode = pinToSymbol_get(alias, pinid);
				if (wErrCode_isSuccess(wecode))
					printf("Symbol: %s\n", alias);
				else {
					if (wecode == WERRCODE_ERROR_INITFAILED)
						fprintf(stderr, "ERROR! Module's initialization failed\n");
				
					else if (wecode == WERRCODE_WARNING_ITNOTFOUND)
						fprintf(stderr, "WARNING! No symbol defined for the \"%s\" pin\n", pinid);
				
					else
						fprintf(stderr, "*** Unexpected returned code ***\n");
				}
			}
			if (*string != '\0') {
				wecode = pinDef_get(string, pinid, &value);
				if (wErrCode_isSuccess(wecode))
					printf("The string contained the \"%s\"-pin definition (value=%d)\n", pinid, value);

				else if (wecode == WERRCODE_WARNING_ITNOTFOUND)
					printf("The typed string does not containe a valid pin definition\n");

				else
					printf("ERROR! pinDef_get() call failed\n");
			}
		}
	}
	return(wErrCodeToShell(wecode));
}
