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
#if MOCK == 0
#include <avr/io.h>
#include <util/twi.h>
#include <mbesMCP23008.h>
#include <avr/pgmspace.h>

#else
#include <debugTools.h>
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#endif

#include <mbesUtilities.h>
#include <mbesSerialConsole.h>
#include <mbesMock.h>
#include <stdio.h>


#if DEBUG > 0
#define LOGERR(X, Y)   logMsg(PSTR("ERROR! in %s(%d)"), X, Y);
#else
#define LOGERR(X, Y)   ;
#endif

#if MOCK == 1
static int fd = 0;

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
	char    c = '#';
	
	va_start(argp, fmt);

	USART_writeString(PSTR("LOGMSG: "), USART_FLASH);
	
	while ((c = pgm_read_byte(fmt++)) != '\0') {
		if (ctrvFlag == false) {
			if (c == '%')  ctrvFlag = true;
			else           USART_writeChar(c);
		} else {
			if (c == 'c')
				USART_writeChar((char)va_arg(argp, int));

			else if (c == 'd') {
				sprintf(buffer, "%d", va_arg(argp, int));
				USART_writeString(buffer, USART_RAM);
				
			} else if (c == 's')
				USART_writeString(va_arg(argp, char*), USART_RAM);
				
			else
				USART_writeString(PSTR("%?"), USART_FLASH);
				
			ctrvFlag = false;
		}
		t++;
	}
	
	USART_writeChar('\n');
	USART_writeChar('\r');
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


uint8_t pinDirectionRegister (const char *code, mbesPinDir dir) {
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
	// Returned code:
	//	0	I2C BUS returned error
	//	1     Operation completed successfully
	//
	uint8_t ecode = 1; // SUCCESS
	
#if MOCK == 0
	char    port;
	uint8_t pinNumber;
	
	codeConverter(code, &port, &pinNumber);
	
	if (port == 'A') {
		if (dir == OUTPUT) DDRA |=  (1 << pinNumber);
		else               DDRA &= ~(1 << pinNumber);
		
	} else if (port == 'B') {
		if (dir == OUTPUT) DDRB |=  (1 << pinNumber);
		else               DDRB &= ~(1 << pinNumber);
	
	} else if (port == 'C') {
		if (dir == OUTPUT) DDRC |=  (1 << pinNumber);
		else               DDRC &= ~(1 << pinNumber);
	
	} else if (port == 'D') {
		if (dir == OUTPUT) DDRD |=  (1 << pinNumber);
		else               DDRD &= ~(1 << pinNumber);
		
	} else if (port >= '0' && port <= '9') {
		uint8_t iodirRegValue = 0;

		//
		// "IODIR" register configuration...
		//
		if (regSelecting_MCP23008(MCP23008_IODIR) == 0 || regReading_MCP23008(&iodirRegValue) == 0) {
			// ERROR!
			LOGERR(__FUNCTION__, __LINE__)
			ecode = 0;
			
		} else {
			if (dir == INPUT)  iodirRegValue |= (1 << pinNumber);
			else               iodirRegValue &= ~(1 << pinNumber);

			if (regSaving_MCP23008(iodirRegValue) == 0) {
				// ERROR!
				LOGERR(__FUNCTION__, __LINE__)
				ecode = 0;

			} else {
				uint8_t regValue = 0;

				//
				// Check for register setting
				//
				if (
					regSelecting_MCP23008(MCP23008_IODIR) == 0 ||
					regReading_MCP23008(&regValue)        == 0 ||
					regValue != iodirRegValue
				) {
					// ERROR! IODIR setting failed
					LOGERR(__FUNCTION__, __LINE__)
					ecode = 0;
				}
			}
		}

	} else {
		// ERROR! (please check for your source code)
		LOGERR(__FUNCTION__, __LINE__)
	}

	if (ecode && dir == INPUT) 
		ecode = pullUpEnabling(code);
	
#endif

	return(ecode);
}


uint8_t getPinValue (const char *code, uint8_t *pinValue) {
	//
	// Description:
	//	It returns the argument defined input pin's value
	//
	//	pinValue:
	//		1 ---> button/switch = VCC ---> button = RFELEASED
	//		0 ---> button/switch = GND ---> button = PUSHED
	//
	// Returned value:
	//	1 SUCCESS
	//	0 An error occurred in I2C bus or MCP23008 operations
	//
	char    port;
	uint8_t pinNumber;
	uint8_t ecode = 1;

	codeConverter(code, &port, &pinNumber);
	
#if MOCK == 0
	if (port == 'A')
		ecode = (PINA & (1 << pinNumber));
		
	else if (port == 'B')
		ecode = (PINB & (1 << pinNumber));
	
	else if (port == 'C')
		ecode = (PINC & (1 << pinNumber));
	
	else if (port == 'D')
		ecode = (PIND & (1 << pinNumber));
	
	else if (port >= '0' && port <= '9') {
		uint8_t regValue = 0;
		
		if (regSelecting_MCP23008(MCP23008_GPIO) == 0 || regReading_MCP23008(&regValue) == 0) {
			// ERROR!
			LOGERR(__FUNCTION__, __LINE__)
			ecode = 0;
			
		} else
			// [!] PIN's value extraction
			*pinValue = (regValue & (1 << pinNumber)) > 0 ? 1 : 0;
		
	} else {
		// ERROR!
		LOGERR(__FUNCTION__, __LINE__)
		ecode = 0;
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
				ecode = buffer[2];
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

	return(ecode);
}



uint8_t pullUpEnabling (const char *code) {
	//
	// Description:
	//	It enable the pull-up resistor for the argument defined input pin
	//
	uint8_t ecode = 1;
	
#if MOCK == 0
	char    port;
	uint8_t pinNumber;

	codeConverter(code, &port, &pinNumber);
	if (port == 'A')
		PORTA |= (1 << pinNumber);

	else if (port == 'B')
		PORTB |= (1 << pinNumber);

	else if (port == 'C')
		PORTC |= (1 << pinNumber);

	else if (port == 'D')
		PORTD |= (1 << pinNumber);

	else if (port >= '0' && port <= '9') {
		uint8_t gppuRegValue = 0;
		
		// Register reading...
		if (regSelecting_MCP23008(MCP23008_GPPU) == 0 || regReading_MCP23008(&gppuRegValue) == 0) {
			// ERROR!
			LOGERR(__FUNCTION__, __LINE__)
			ecode = 0;

		} else {
			gppuRegValue |= (1 << pinNumber);
			
			// Register setting...	
			if (regSaving_MCP23008(gppuRegValue) == 0) {
				// ERROR!
				LOGERR(__FUNCTION__, __LINE__)
				ecode = 0;

			} else {
				uint8_t regValue = 0;

				// Checking for the register's value
				if (
					regSelecting_MCP23008(MCP23008_GPPU)  == 0 ||
					regReading_MCP23008(&regValue)        == 0 ||
					regValue != gppuRegValue
				) {
					// ERROR!
					LOGERR(__FUNCTION__, __LINE__)
					ecode = 0;
				}
			}
		}
	} else {
		// ERROR! (please, check fot your code)
		LOGERR(__FUNCTION__, __LINE__)
	}
#endif

	return(ecode);
}


uint8_t setPinValue (const char *code, uint8_t value) {
	//
	// Description:
	//	It sets the argument defined output pin with the defined value
	//	Because the target is a single PIN, the value MUST be 0 or 1
	//
	char    port;
	uint8_t pinNumber;
	uint8_t ecode = 1;

	codeConverter(code, &port, &pinNumber);

	if (port == 'A') {
		if (value)  PORTA |=  (1 << pinNumber);
		else        PORTA &= ~(1 << pinNumber);
		
	} else if (port == 'B') {
		if (value) PORTB |=  (1 << pinNumber);
		else       PORTB &= ~(1 << pinNumber);
		
	} else if (port == 'C') {
		if (value) PORTC |=  (1 << pinNumber);
		else       PORTC &= ~(1 << pinNumber);
		
	} else if (port == 'D') {
		if (value) PORTD |=  (1 << pinNumber);
		else       PORTD &= ~(1 << pinNumber);
		
	} else if (port >= '0' && port <= '9') {
		uint8_t regValue = 0;
		
		if (regSelecting_MCP23008(MCP23008_GPIO) == 0 || regReading_MCP23008(&regValue) == 0 ) {
			// ERROR!
			LOGERR(__FUNCTION__, __LINE__)
			ecode = 0;

		} else if (value == 0) {
			regValue &= ~(1 << pinNumber);
			
		} else {
			regValue |= (1 << pinNumber);
		}
			
		if (ecode && regSaving_MCP23008(regValue) == 0) {
			// ERROR!
			LOGERR(__FUNCTION__, __LINE__)
			ecode = 0;
		}
		
	} else {
		// ERROR! (please, check fot your code)
		LOGERR(__FUNCTION__, __LINE__)
	}

	return(ecode);
}
