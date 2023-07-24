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
#include <mbesUtilities.h>
#include <mbesSerialConsole.h>
#include <mbesMock.h>
#include <stdio.h>

#if MOCK == 0
#include <avr/io.h>

#else
#include <debugTools.h>
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static int fd = 0;

#endif

#if MOCK == 1
static int _ubRead (void *bytes, uint8_t size) {
	int     tot = 0;
	uint8_t part = 1;
	void    *ptr = NULL;

	while (part > 0 && tot < size) {
		ptr = ((void*)bytes + tot);
		part = read(fd, ptr, (size - tot));
		if (part < 0) tot = -1;
		else          tot = tot + part;
	}
	return(tot);
}
#endif

//------------------------------------------------------------------------------------------------------------------------------
//                                           P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
#if MOCK == 1
void mbesSelector_shutdown() {
	//
	// Decription:
	//	It close files and release resources, you should call it at the end of the test
	//	It has neaning JUST in (MOCK=1) test mode!!
	//
	close(fd);
	return;
}

void mbesUtilities_init() {
	//
	// Description:
	//	In order to stubb the PINs reading function, every virtual pin's value is acnowledged by a data-file. The file
	//	is filled by an external process (virtualSelectors), asyncronously. The file has to be opened (and closed) only one
	//	 time.
	//
	if ((fd = open(MBES_VIRTUALSEVECTOR_SWAPFILE, O_RDONLY)) && fd < 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(stderr, "I cannot open the \"%s\" file\n", MBES_VIRTUALSEVECTOR_SWAPFILE);
		_exit(127);
	}
	return;
}
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

	USART_writeString("LOGMSG: ");
	
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
	//	It sets the argument defined PIN with the given direction (INPUT|OUTPUT). The PIN can belong to the MCU or an
	//	extension I/O chip connected with I2C BUS. It the specified port is a number ([0-9]) then it will be interpretated
	//	as the MCP23008XP address where the pin is defined.
	//
	//	AVR Data Directions Register (DDR):
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
	//	MCP23008XP Directions register (IODIR 0x00):
	//		+----------+-------+-------+-------+-------+-------+-------+-------+-------+
	//		| Register | pin7  | pin6  | pin5  | pin4  | pin3  | pin2  | pin1  | pin0  |
	//		+----------+-------+-------+-------+-------+-------+-------+-------+-------+
	//		|  IODIR   | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} | {0|1} |
	//		+----------+-------+-------+-------+-------+-------+-------+-------+-------+
	//
	//		bitN == 0 --> pinN = output
	//		bitN == 1 --> pinN = input
	//
	//
#if MOCK == 0
	char    port;
	uint8_t pinNumber;
	
	if (port == 'A') {
		if (dir == OUTPUT) DDRA |= (1 << pinNumber);
		else               DDRA &= ~(1 << pinNumber);
	
	} else if (port == 'B') {
		if (dir == OUTPUT) DDRB |= (1 << pinNumber);
		else               DDRB &= ~(1 << pinNumber);
	
	} else if (port == 'C') {
		if (dir == OUTPUT) DDRC |= (1 << pinNumber);
		else               DDRC &= ~(1 << pinNumber);
	
	} else if (port == 'D') {
		if (dir == OUTPUT) DDRD |= (1 << pinNumber);
		else               DDRD &= ~(1 << pinNumber);
		
	} else if (port >= '0' && port <= '9') {
		uint8_t iodirValue = 0;
		uint8_t myID = port - '0':
		
		// "IODIR" register selecting
		I2C_Start();
		I2C_Write(myID << 1);         // LSB=0 --> writing operation
		I2C_Write(0x00);              // "IODIR" register (0x00) selection
		
		// "IODIR" register reading
		I2C_Start();
		I2C_Write((myID << 1) | 1);   // LSB=1 --> reading operation
		iodirValue = I2C_Read(I2C_NACK);
		
		// "IODIR" register changing
		if (dir == OUTPUT) iodirValue &= ~(1 << pinNumber)
		else               iodirValue |=  (1 << pinNumber);
		
		// "IODIR" register saving
		I2C_Start();
		I2C_Write(myID << 1);         // LSB=0 --> writing operation
		I2C_Write(iodirValue);
		
		I2C_Stop();
        
	} else {
		// ERROR!
	}
#endif

	return;
}


bool getPinValue (const char *code) {
	//
	// Description:
	//	It returns the argument defined input pin's value
	//
	// Returned value:
	//	true ---> pin=1 ---> button/switch = VCC ---> button = RFELEASED
	//	false --> pin=0 ---> button/switch = GND ---> button = PUSHED
	//
	char    port;
	uint8_t pinNumber;
	uint8_t out = 0;

	codeConverter(code, &port, &pinNumber);
	
#if MOCK == 0
	if      (port == 'A') out = (PINA & (1 << pinNumber));
	else if (port == 'B') out = (PINB & (1 << pinNumber));
	else if (port == 'C') out = (PINC & (1 << pinNumber));
	else if (port == 'D') out = (PIND & (1 << pinNumber));
	else {
		// ERROR!
		logMsg("ERROR(%d)! \"%c\" is not a valid port\n", __LINE__, port);
	}

#else
	uint8_t numOfRec = 0;
	
	// File locking
	if (flock(fd, LOCK_EX) < 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(
			stderr, "I cannot lock the \"%s\" file, because flock() syscall failed: %s\n", 
			MBES_VIRTUALSEVECTOR_SWAPFILE, strerror(errno)
		);
		_exit(127);
	
	// Rewind...
	} else if (lseek(fd, 0, SEEK_SET) < 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(stderr, "I cannot rewind the file; %s\n", strerror(errno));
		_exit(127);
		
	// Data size reading
	} else if (_ubRead(&numOfRec, 1) != 1) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(stderr, "I/O operation failed: %s\n", strerror(errno));
		_exit(127);
		
	} else {
		char buffer[3];
		
		// Data reading
		for (uint8_t t=0; t<numOfRec; t++) {
			if (_ubRead(buffer, 3) != 3) {
				// ERROR!
				ERRORBANNER(127)
				fprintf(stderr, "I/O operation failed: %s\n", strerror(errno));
				_exit(127);
				
			} else {
				out = buffer[2];
				buffer[2] = '\0';
				if (strcmp(buffer, code) == 0) break;
			}
		}
		
		// File unlocking
		if (flock(fd, LOCK_UN) < 0) {
			// ERROR!
			ERRORBANNER(127)
			fprintf(
				stderr, "I cannot unlock the \"%s\" file, because flock() syscall failed: %s\n",
				MBES_VIRTUALSEVECTOR_SWAPFILE, strerror(errno)
			);
			_exit(127);
		}
	}
#endif

	return((bool)out);
}


uint8_t I2C_Write (uint8_t data) {
	//
	// Description:
	//	It sends the argument defined byte using the I2C BUS, and returns the transmission status
	//
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	// Waiting for the operation end
	while (!(TWCR & (1 << TWINT)));
	
	return(TWSR & 0xF8);
}


uint8_t I2C_Read_ACK (mbesI2CopType optType) {
	//
	// Description:
	//	It reads a byte from the I2C bus and returns it. 
	//
	if (optType == I2C_ACK)
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); 
	else
		TWCR = (1 << TWINT) | (1 << TWEN);
		
	// Waiting  for data cknowledge
	while (!(TWCR & (1 << TWINT)));
	
	return(TWDR);
}


void I2C_Stop() {
	//
	// Description:
	//	It seands a STOP marker to the remote device on I2C
	//
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}


void I2C_Start() {
	//
	// Description:
	//	It seands a START marker to the remote device on I2C
	//
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}
