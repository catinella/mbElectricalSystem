/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesUtitities.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	Thif file contains generic functions used by the project's modules
//
// License:
//	KiCad Schematics distributed under the GPL-3 License.
//
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
#include "mbesUtilities.h"
#include "mbesSerialConsole.h"
#include <stdio.h>

#if MOCK == 0
#include <avr/io.h>

#else
#include <stdbool.h>
#endif

void logMsg (const char *fmt, ...) {
	//
	// Description:
	//	It sends the argument defined (printf-style) message to the debug console
	//
	#if DEBUG == 1
	uint8_t t = 0;
	bool    ctrvFlag = false;
	va_list argp;
	char    buffer[9];
	
	va_start(argp, fmt);

	while (fmt[t] != '\0') {
		if (ctrvFlag == false) {
			if (fmt[t] == '%')  ctrvFlag = true;
			else                USART_writeChar(fmt[t]);
		} else {
			if (fmt[t] == 'c')
				USART_writeChar((char)va_arg(argp, int));
				
			else if (fmt[t] == 'd') {
				sprintf(buffer, "%d", va_arg(argp, int));
				USART_writeString(buffer);
				
			} else if (fmt[t] == 's')
				USART_writeString(va_arg(argp, char*));
				
			else
				USART_writeString("%?");
				
			ctrvFlag = false;
		}
		t++;
	}
	
	va_end(argp);
	
	#endif
	
	return;
}


void codeConverter (const char *code, char *port, uint8_t *pinNumber) {
	//
	// Description:
	//	It check out the PIN attributes (port and pin number) vrom the argument defined (char[2]) code.
	//	This function allows you to define a PIN using two character, eg "A1"
	//
	if (port      != NULL) *port      = code[0];
	if (pinNumber != NULL) *pinNumber = code[1] - '0';
	
	return;
}


void pinDirectionRegister (const char *code, mbesPinDir dir) {
	//
	// Description:
	//	It sets the MCU's pin to function as input or output
	//
	//	AVR Data Direction Registers (DDR) registers:
	//		+----------+-------+-------+-------+-------+-------+-------+-------+-------+
	//		| Register | pin7  | pin6  | pin5  | pin4  | pin3  | pin2  | pin1  | pin0  |
	//		+----------+-------+-------+-------+-------+-------+-------+-------+-------+
	//		|   DDRA   | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} |
	//		|   DDRB   | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} |
	//		|   DDRC   | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} |
	//		|   DDRD   | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} |
	//		+----------+-------+-------+-------+-------+-------+-------+-------+-------+
	//
	//		bitN == 1 --> pinN = output
	//		bitN == 0 --> pinN = input
	//
#if MOCK == 0
	char    port;
	uint8_t pinNumber;
	
	_codeConverter(code, &port, &pinNumber);
	if      (port == 'A' && dir == OUTPUT)  DDRA |=  (1 << pinNumber);
	else if (port == 'A')                   DDRA &= ~(1 << pinNumber);
	else if (port == 'B' && dir == OUTPUT)  DDRB |=  (1 << pinNumber);
	else if (port == 'B')                   DDRB &= ~(1 << pinNumber);
	else if (port == 'C' && dir == OUTPUT)  DDRC |=  (1 << pinNumber);
	else if (port == 'C')                   DDRC &= ~(1 << pinNumber);
	else if (port == 'D' && dir == OUTPUT)  DDRD |=  (1 << pinNumber);
	else if (port == 'D')                   DDRD &= ~(1 << pinNumber);
	else {
		// ERROR!
	}
#endif

	return;
}


