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
#include <mbesI2C.h>
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


#if MOCK == 0
//
// I2C I/O extender MCP23008XP's regirters
//
#define IODIR_ADDR 0x00
#define GPIO_ADDR  0x09
#define GPPU_ADDR  0x06


#else
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
		uint8_t iodirValue = 0;
		uint8_t myID = port - '0';
		uint8_t status;

		// "IODIR" register selecting
		I2C_START(status)
		if (status == 1) {
			I2C_WRITE((myID << 1), status)
			if (status) I2C_WRITE(IODIR_ADDR, status)
		}

		if (status) {
			// "IODIR" register reading
			I2C_START(status)
			if (status) {
				I2C_WRITE(((myID << 1)|1), status)
				if (status)
					I2C_READ(I2C_NACK, iodirValue, status);
			}
		} else {
			// ERROR!
		}

		if (status) { 
			// "IODIR" register changing
			if (dir == OUTPUT) iodirValue &= ~(1 << pinNumber);
			else               iodirValue |=  (1 << pinNumber);
		
			// "IODIR" register saving
			I2C_START(status)
			if (status) {
				I2C_WRITE((myID << 1), status)
				if (status) I2C_WRITE(iodirValue, status)
			}
		} else {
			// ERROR!
		}

		if (status == 0) {
			// ERROR!
		}

		I2C_STOP;
        
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
	if (port == 'A')
		out = (PINA & (1 << pinNumber));
		
	else if (port == 'B')
		out = (PINB & (1 << pinNumber));
	
	else if (port == 'C')
		out = (PINC & (1 << pinNumber));
	
	else if (port == 'D')
		out = (PIND & (1 << pinNumber));
	
	else if (port >= '0' && port <= '9') {
		uint8_t myID      = port - '0';
		uint8_t gpioValue = 0;
		uint8_t status    = 0;
		
		// "GPIO" register selecting
		I2C_START(status);
		if (status) {
			I2C_WRITE((myID << 1), status)
			if (status) 
				I2C_WRITE(GPIO_ADDR, status);
		}

		if (status) {
			// "GPIO" register reading
			I2C_START(status);
			if (status) {
				I2C_WRITE(((myID << 1) | 1), status)
				if (status) {
					I2C_READ(I2C_NACK, gpioValue, status)
					if (status)
						out = gpioValue & (1 << pinNumber);
				}
			}
		} else {
			// ERROR
		}

		if (status) {
			// ERROR
		}

		I2C_STOP;
		
	} else {
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

	return(out > 0 ? true : false);
}



void pullUpEnabling (const char *code) {
	//
	// Description:
	//	It enable the pull-up resistor for the argument defined input pin
	//
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
		uint8_t myID      = port - '0';
		uint8_t status;

		// "IODIR" register selecting
		I2C_START(status);
		if (status) {
			I2C_WRITE((myID << 1), status)
			if (status) {
				I2C_WRITE(GPPU_ADDR, status)
				if (status)
					I2C_WRITE((1 << pinNumber), status)
			}
		}
		
		if (status == 0) {
			// ERROR!
		}
		I2C_STOP;
		
	} else {
		// ERROR!
		logMsg("ERROR(%d)! \"%c\" is not a valid port\n", __LINE__, port);
	}
#endif

	return;
}


void setPinValue (const char *code, uint8_t value) {
	//
	// Description:
	//	It set the argument defined boolean value to the output pin
	//
	char    port;
	uint8_t pinNumber;

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
		uint8_t myID      = port - '0';
		uint8_t gpioValue = 0;
		uint8_t status = 0;

		// "GPIO" register selecting
		I2C_START(status);
		if (status) {
			I2C_WRITE((myID << 1), status)
			if (status) I2C_WRITE(GPIO_ADDR, status)
		}

		// "GPIO" register reading
		if (status) {
			I2C_START(status);
			if (status) {
				I2C_WRITE(((myID << 1)|1), status)
				if (status)
					I2C_READ(I2C_NACK, gpioValue, status);
			}
		} else {
			// ERROR
		}

		if (status) {
			if (value)  gpioValue |=  (1 << pinNumber);
			else        gpioValue &= ~(1 << pinNumber);
		
			// "GPIO" register saving
			I2C_START(status);
			if (status) {
				I2C_WRITE((myID << 1), status)
				if (status) I2C_WRITE(gpioValue, status)
			}
		} else {
			// ERROR
		}
		
		if (status == 0) {
			// ERROR
		}

		I2C_STOP;
		
	} else {
		// ERROR!
	}

	return;
}
