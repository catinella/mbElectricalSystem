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
#include <ctype.h>
#include <pinToSymbol.h>
		
static ptsDbItem_t ptsDb[PTS_MAXPINS];
static bool        initFlag = false;
static regex_t     pinDef_regx;

	
werror _iatoi (int *dst, const char *src) {
	//
	// Description:
	//	Intelligent atoi() function
	//
	// Arguments:
	//	dst:  the converted number
	//	src:  the characters string version of the number
	//
	// Returned vale:
	//	WERRCODE_SUCCESS
	//	WERRCODE_ERROR_INVALIDDATA
	//
	werror  err  = WERRCODE_SUCCESS;
	
	if (strlen(src) > 0) {
		uint8_t t    = 0;
		char    *ptr = (char*)src;
		
		// Zero padding removing...
		while (src[t] == '0' && src[t] != '\0') t++;
	
		if (src[t] == '\0')
			// 00000.. = 0
			*dst = 0;
			
		else {
			*dst = atoi(ptr);
	
			// Checking for atoi() error
			if (dst == 0)
				// ERROR!
				// The string contains not-numeric characters too
				err = WERRCODE_ERROR_INVALIDDATA; 
		}
	} else
		// ERROR!
		// Empty data is not allowed
		err = WERRCODE_ERROR_INVALIDDATA;
		
	return(err);
}

//-----------------------------------------------------------------------------------------------------------------------------
//                                       P U B L I C   F U N C T I O N S
//-----------------------------------------------------------------------------------------------------------------------------

werror pinToSymbol_get (char *symbol, const char *pin) {
	//
	// Description:
	//	This function find the symbol associated to the argument defined pin and write it in the "symbol" argument.
	//
	// Arguments:
	//	symbol:  It is used to store symbol you are looking for
	// 	pin:     The pin associated to the symbol
	//	
	// Returned value:
	//	WERRCODE_SUCCESS
	//	WERRCODE_WARNING_ITNOTFOUND
	//	WERRCODE_ERROR_INITFAILED
	//
	werror ecode;

	if (initFlag == false)
		if (wErrCode_isError(pinToSymbol_init(PTS_PINMAPFILE)))
			// ERROR!
			ecode = WERRCODE_ERROR_INITFAILED;
	
	if (initFlag == true) {
		uint8_t dbIndex = 0;
		
		ecode = WERRCODE_WARNING_ITNOTFOUND;
		while (ptsDb[dbIndex].pin[0] != '\0' && dbIndex < PTS_MAXPINS) {
			if (strcmp(ptsDb[dbIndex].pin, pin) == 0) {
				ecode = WERRCODE_SUCCESS;
				strcpy(symbol, ptsDb[dbIndex].symbol);
				break;
			} else {
				dbIndex++;
			}
		}
	}
	return(ecode);
}



werror pinToSymbol_init (const char *headerFile) {
	//
	// Description:
	//	Initialization procedure. It reads the mbesPinsMap.h file as a text one, and stores all pin-symbol associations
	//	inside the module's static array.
	//
	// Returned value:
	//	WERRCODE_SUCCESS
	//	WERRCODE_ERROR_IOOPERFAILED
	//	WERRCODE_ERROR_REGEXCOMP
	//
	regex_t reegex_a, reegex_b;
	int     ea = 0,   eb = 0;     // Regexes error codes
	 werror ecode = WERRCODE_SUCCESS;
	
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
			ecode = WERRCODE_ERROR_IOOPERFAILED;
		
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
					ecode = WERRCODE_ERROR_IOOPERFAILED;
					break;
						
				} else if (regexec(&reegex_a, row, 3, pmatch, 0) == 0) {
					// "#define[ \t]\+" ereasing...
					strcpy (tmp, (row + pmatch[0].rm_eo));
					
					if (regexec(&reegex_b, tmp, 3, pmatch, 0) == 0) {
						// TODO: inside definition comments deleting
						//       (#define <pin> /*<comment*/ <symbol>
						
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
									//printf("Symbol:%s\n", ptsDb[dbIndex].symbol);
									st = 1;
									x = 0;
								}
								
							} else if (st == 1) {
#ifdef TARGET_AVR8
									// In AVR8 GPIOs are identified by a two chars string (eg. "A1")
									if (tmp[t] == '"') st = 2;
#elifdef TARGET_ESP32
									// In ESP32 GPIO-ID is a symbol (eg. GPIO_NUM_0)
									if (isalnum(tmp[t])) {
										st = 2;
										(ptsDb[dbIndex].pin)[x] = tmp[t];
										x++;
									}
#endif 
									
							} else if (st == 2) {
								//
								// PIN name recording...
								//
#ifdef TARGET_AVR8
								if (tmp[t] != '"') {
#elifdef TARGET_ESP32
								if (isalnum(tmp[t]) || tmp[t] == '_') {
#endif
									ptsDb[dbIndex].pin[x] = tmp[t];
									x++;
								} else {
									ptsDb[dbIndex].pin[x] = '\0';
									//printf("PIN: %s\n", ptsDb[dbIndex].pin);
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
			fprintf(stderr, "ERROR! I cannot compile the \"%s\" regex: %s\n", PTS_DEFAREGEX, regErrBuff);
		}
		if (eb != 0) {
			regerror(errno, &reegex_b, regErrBuff, 128);
			fprintf(stderr, "ERROR! I cannot compile the \"%s\" regex: %s\n", PTS_DEFBREGEX, regErrBuff);
		}
		ecode = WERRCODE_ERROR_REGEXCOMP;
	}
	
	if (wErrCode_isSuccess(ecode)) initFlag = true;
	
	return(ecode);
}



werror pinDef_get (const char *log, char *pin, int *value) {
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
	//	WERRCODE_SUCCESS
	//	WERRCODE_WARNING_ITNOTFOUND
	//	WERRCODE_ERROR_REGEXCOMP
	//	WERRCODE_ERROR_INVALIDDATA
	//
	static bool    initFlag = false;
	werror         err = WERRCODE_SUCCESS;
	
	if (initFlag == false) {
		if (regcomp(&pinDef_regx, CONS_PINDEMATCH, 0) == 0) {
			initFlag = true;
		} else {
			// ERROR!
			char regErrBuff[128];
			regerror(errno, &pinDef_regx, regErrBuff, 128);
			fprintf(stderr, "ERROR(%d)! I cannot compile the regex: %s", __LINE__, regErrBuff);
			err = WERRCODE_ERROR_REGEXCOMP;
		}
	} 
	
	if (initFlag) {
		regmatch_t pmatch[3];
		char       strValue[16]; 
		
		if (regexec(&pinDef_regx, log, 3, pmatch, 0) == 0) {
			strcpy(strValue, (strchr(log, ':') + 1));
			*strchr(log, ':') = '\0';
			strcpy(pin, log);
			
			if (wErrCode_isError(_iatoi(value, strValue))) {
				// ERROR!
				err = WERRCODE_ERROR_INVALIDDATA;
			}
		} else
			// It is just a normal log message
			err = WERRCODE_WARNING_ITNOTFOUND;
	}
	
	return(err);
}


void pinDef_free() {
	regfree(&pinDef_regx);
	return;
}
