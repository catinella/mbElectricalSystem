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
#include <werror.h>
#include <pinToSymbol.h>

int main (int argc, char *argv[]) {
	werror wecode = WERRCODE_SUCCESS;
	char   symbol[PTS_MAXSYMSIZE];
	
	if (argc == 1) {
		fprintf(stderr, "ERROR! Use: %s [A-Z][0-9]\n", argv[0]);
		wecode = WERRCODE_ERROR_ILLEGALARG;
		
	} else {
		wecode = pinToSymbol_get(symbol, argv[1]);
		if (wErrCode_isSuccess(wecode))
			printf("Symbol: %s\n", symbol);
		else {
			if (wecode == WERRCODE_ERROR_INITFAILED)
				fprintf(stderr, "ERROR! Module's initialization failed\n");
				
			else if (wecode == WERRCODE_WARNING_ITNOTFOUND)
				fprintf(stderr, "WARNING! No symbol defined for the \"%s\" pin\n", argv[1]);
				
			else
				fprintf(stderr, "*** Unexpected returned code ***\n");
		}
	}		
	
	return((wErrCode_isSuccess(wecode) ? 0 : wecode));
}
