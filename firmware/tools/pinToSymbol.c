/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: pinToSymbol.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
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
#include <regex.h>
#include <errno.h>
#include <string.h>
#include <pinToSymbol.h>
		
static ptsDbItem_t ptsDb[PTS_MAXPINS];
static bool initFlag = false;

uint8_t pinToSymbol_get (char *symbol, const char *pin) {
	//
	// Description:
	//	This function find the symbol associated to the argument defined pin and write it in the "symbol" argument.
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
	uint8_t     ecode = 1;

	if (initFlag == false)
		pinToSymbol_init(PTS_PINMAPFILE);
	
	if (initFlag == true) {
		uint8_t dbIndex = 0;
		
		ecode = 16;
		while (ptsDb[dbIndex].pin[0] != '\0' && dbIndex < PTS_MAXPINS) {
			if (strcmp(ptsDb[dbIndex].pin, pin) == 0) {
				ecode = 1;
				strcpy(symbol, ptsDb[dbIndex].symbol);
				break;
			} else {
				dbIndex++;
			}
		}
	}
	return(ecode);
}



uint8_t pinToSymbol_init (const char *headerFile) {
	//
	// Description:
	//	Initialization procedure. It reads the mbesPinsMap.h file as a text one, and stores all pin-symbol associations
	//	inside the module's static array.
	//
	// Returned value:
	//	0: ERROR! (data reading or regex-compiling ... faliled)
	//	1: The module has been successfully initialized
	//
	regex_t reegex_a, reegex_b;
	int     ea = 0,   eb = 0;     // Regexes error codes
	uint8_t ecode = 1;
	
	// DB cleaning
	for (uint8_t t=0; t<PTS_MAXPINS; t++) {
		ptsDb[t].pin[0]    = '\0';
		ptsDb[t].symbol[0] = '\0';
	}
		
	if (
		(ea = regcomp(&reegex_a, PTS_DEFAREGEX, 0)) == 0 &&
		(eb = regcomp(&reegex_b, PTS_DEFBREGEX, 0)) == 0
	) { 
		FILE   *FH = NULL;
			
		if ((FH = fopen(headerFile, "r")) == NULL) {
			// ERROR!
			fprintf(stderr, "ERROR! I cannot open the \"%s\" file: %s\n", headerFile, strerror(errno));
			ecode = 0;
		
		} else {
			size_t     size = PTS_ROWMAXSIZE;
			char       *row = (char*)malloc(size * sizeof(char)); // getline() requires dinamically allocated memory!!
			regmatch_t pmatch[3]; // Up to 3 sub-expressions
			char       tmp[PTS_ROWMAXSIZE];
			uint8_t    t = 0, x = 0, st = 0;
			uint8_t    dbIndex = 0;
			
			if (row == NULL) ecode = 0;
				
			while (feof(FH) == 0 && ecode == 1) {
				if (getline(&row, &size, FH) < 0 && errno != 0) {
					// ERROR!
					fprintf(stderr, "ERROR! data readingfailed: %s\n", strerror(errno));
					ecode = 0;
					break;
						
				} else if (regexec(&reegex_a, row, 3, pmatch, 0) == 0) {
					// "#define[ \t]\+" ereasing...
					strcpy (tmp, (row + pmatch[0].rm_eo));
					
					if (regexec(&reegex_b, tmp, 3, pmatch, 0) == 0) {
						*(tmp+pmatch[0].rm_eo+1) = '\0';
						t = 0; x = 0; st = 0;
						
						while (tmp[t] != '\0') {
							if (st == 0) {
								//
								// Symbol name recording...
								//
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
								//
								// PIN name recording...
								//
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

				// The following lines-code block shows you the ptsDb content  (it is just for debug)
			//	{
			//		uint8_t dbIndex = 0;
			//		while (ptsDb[dbIndex].pin[0] != '\0' && dbIndex < CONS_MAXPINS) {
			//			printf("%s (%s)\n", ptsDb[dbIndex].symbol, ptsDb[dbIndex].pin);
			//			dbIndex++;
			//		}
			//	}
			}
			fclose(FH);
			free(row);
			regfree(&reegex_a);
			regfree(&reegex_b);
		}
	} else {
		// ERROR!
		char regErrBuff[128];
		if (ea != 0) {
			regerror(errno, &reegex_a, regErrBuff, 128);
//			syslog(LOG_ERR, "ERROR! I cannot compile the \"%s\" regex: %s\n", PTS_DEFAREGEX, regErrBuff);
		}
		if (eb != 0) {
			regerror(errno, &reegex_b, regErrBuff, 128);
//			syslog(LOG_ERR, "ERROR! I cannot compile the \"%s\" regex: %s\n", PTS_DEFBREGEX, regErrBuff);
		}
	}
	
	if (ecode) initFlag = true;
	
	return(ecode);
}
