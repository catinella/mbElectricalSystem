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
//	       +-----+-----+------+----+-----+----+-----+------+
//	UCSRA: | RXC | TXC | UDRE | FE | DOR | PE | U2X | MPCM |
//	       +--+--+--+--+--+---+--+-+--+--+-+--+--+--+--+---+
//	          |     |     |      |    |    |     |     |
//	          |     |     |      |    |    |     |     +------- Multi-processor Communication Mode
//	          |     |     |      |    |    |     +------------- Double the USART Transmission Speed
//	          |     |     |      |    |    +------------------- Parity Error
//	          |     |     |      |    +------------------------ Data OverRun
//	          |     |     |      +----------------------------- Frame Error
//	          |     |     +------------------------------------ Empty buffer and ready to be written
//	          |     +------------------------------------------ USART Receive Complete flag
//	          +------------------------------------------------ USART Transmit Complete flag
//	
//	
//	       +-------+-------+-------+------+------+-------+------+------+
//	UCSRB: | RXCIE | TXCIE | UDRIE | RXEN | TXEN | UCSZ2 | RXB8 | TXB8 |
//	       +---+---+---+---+---+---+---+--+---+--+---+---+---+--+---+--+
//	           |       |       |       |      |      |       |      |
//	           |       |       |       |      |      |       |      +---- Transmit Data Bit 8
//	           |       |       |       |      |      |       +----------- Receive Data Bit 8
//	           |       |       |       |      |      +------------------- Data sizes list index 2
//	           |       |       |       |      +-------------------------- Transmitter Enable
//	           |       |       |       +--------------------------------- Receiver Enable
//	           |       |       +----------------------------------------- USART Data Register Empty Interrupt Enable
//	           |       +------------------------------------------------- USART Transmit Complete (interrupt enabling)
//	           +--------------------------------------------------------- USART Receive Complete (interrupt enabling)
//	
//	       +-------+-------+------+------+------+-------+-------+-------+
//	UCSRC: | URSEL | UMSEL | UPM1 | UPM0 | USBS | UCSZ1 | UCSZ0 | UCPOL |
//	       +---+---+---+---+---+--+---+--+---+--+---+---+---+---+---+---+
//	           |       |       |      |      |      |       |       |
//	           |       |       |      |      |      |       |       +----- Clock Polarity (Synchronous mode only)
//	           |       |       |      |      |      |       +------------- Data sizes list index 0
//	           |       |       |      |      |      +--------------------- Data sizes list index 1
//	           |       |       |      |      +---------------------------- Stop Bit Select (1-2 bits) 
//	           |       |       |      +----------------------------------- Parity modes list index 0
//	           |       |       +------------------------------------------ Parity modes list index 1
//	           |       +-------------------------------------------------- USART Mode delect (asynchronous/synchronous mode)
//	           +---------------------------------------------------------- Register Select (UCSRC/UBRRH)
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
#include "mbesMock.h"
#include <stdio.h>

#if MOCK == 0
#include <avr/io.h>

#else
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <debugTools.h>
static FILE *fakeConsole = NULL;
static bool initialized = false;

#endif

#include "mbesSerialConsole.h"

extern void USART_Init (unsigned int baud_rate) {
	//
	// Description:
	//	It initializes the internal USART module and sets the port speed with the argument defined value
	//
#if MOCK == 0
	unsigned int ubrr = F_CPU / 16 / baud_rate - 1;
	UBRRH = (unsigned char)(ubrr >> 8);                 // USART Baud Rate Register (H)
	UBRRL = (unsigned char)ubrr;                        //   ""   ""   ""    ""     (L)
	UCSRB = (1 << RXEN) | (1 << TXEN);                  // It enables the ATmega16 to transmit and receive data
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // N81
#else
	fakeConsole = fopen(FAKE_CONSOLE, "w");
	if (fakeConsole == NULL) {
		ERRORBANNER(64);
		fprintf(stderr, "I cannot open the (%s) fake console\n", FAKE_CONSOLE); fflush(stderr);
	}
	initialized = true;
#endif

	return;
}


extern void USART_writeChar (char data) {
	//
	// Description:
	//	It sends the argument defined char
	//
#if MOCK == 0
	while (!(UCSRA & (1 << UDRE))); // Waiting for the serial channel availability
	UDR = data;
#else
	if (initialized == false) USART_Init(0);
	fprintf(fakeConsole, "%c", data); fflush(fakeConsole);
#endif

	return;
}


extern void USART_writeString (const char *data) {
	//
	// Description:
	//	It sends the argument defined characters string
	//
#if MOCK == 0
	uint16_t counter = 0;
	while (data[counter] != '\0') USART_writeChar(data[counter++]);
	USART_writeChar('\0');
#else
	if (initialized == false) USART_Init(0);
	fprintf(fakeConsole, "%s", data); fflush(fakeConsole);
#endif
	
	return;
}

#if MOCK == 1
extern void USART_close() {
	fclose(fakeConsole);
}
#endif
/*
extern char USART_readChar() {
	//
	// Description:
	//	It sends the argument defined char
	//
	while (!(UCSRA & (1 << RXC))); // Waiting for data receiving...
	return UDR;
	
}
*/
