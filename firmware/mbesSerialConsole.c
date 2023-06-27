/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesSerialConsole.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module is responsible on the serial (USART) data exchange to/from the Microcontroller. It has been developed for
//	debug purples, mainly.
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
#include <avr/io.h>
#include <stdio.h>
#include "mbesSerialConsole.h"


extern void USART_Init (unsigned int baud_rate) {
	//
	// Description:
	//	It initializes the internal USART module and sets the port speed with the argument defined value
	//
	unsigned int ubrr = F_CPU / 16 / baud_rate - 1;
	UBRRH = (unsigned char)(ubrr >> 8);
	UBRRL = (unsigned char)ubrr;
	UCSRB = (1 << RXEN) | (1 << TXEN);                  // It enables the ATmega16 to transmit and receive data
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // N81

	return;
}


extern void USART_writeChar (char data) {
	//
	// Description:
	//	It sends the argument defined char
	//
	while (!(UCSRA & (1 << UDRE))); // Waiting for the serial channel availability
	UDR = data;

	return;
}


extern void USART_writeString (const char *data) {
	//
	// Description:
	//	It sends the argument defined characters string
	//
	uint16_t counter = 0;
	while (data[counter] != '\0') USART_writeChar(data[counter++]);

	return;
}


extern char USART_readChar() {
	//
	// Description:
	//	It sends the argument defined char
	//
	while (!(UCSRA & (1 << RXC))); // Waiting for data receiving...
	
	return UDR;
}
