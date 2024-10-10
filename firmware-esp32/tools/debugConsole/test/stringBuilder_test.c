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
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stringBuilder.h>

int main (int argc, char *argv[]) {
	int      ecode = 0;
	char     inputString[64];
	char     builtString[1024];
	uint16_t size = 0, t = 0;
	
	inputString[0] = '\0';
	
	printf("Special characters:\n");
	printf("\t'#': to insert a ('\\n') RETURN character\n");
	printf("\t'!': to exit\n");
	printf("\t' ': to break your string\n");
	printf("\t'?': to see the resulted strings\n");
	printf("\n");
	
	while (1) {
		memset(inputString, '0', 64);
		printf("> "); scanf("%s", inputString);
	
		size = strlen(inputString);
		for (t=0; t<size; t++)
			if (inputString[t] == '#') inputString[t] = '\n';
		
	
		if (inputString[0] == '!' && inputString[1] == '\0')
			break;
			
		else if (inputString[0] == '?' && inputString[1] == '\0') {
			printf("--------- Build strings --------- \n");
			while (stringBuilder_get(builtString) == 1)
				printf("\"%s\"\n", builtString);
			printf("\n");
			
		} else if (stringBuilder_put(inputString, strlen(inputString)) == 0) {
			fprintf(stderr, "ERROR!\n");
			ecode = 127;
			break;
			
		}
		
		
	}
	stringBuilder_close();
	
	return(ecode);
}
