/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesSelector.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module is responsible on the input controls (eg. buttons, switches..) management
//	debug purples, mainly.
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

#if MOCK == 1
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <debugTools.h>

# else
#include <avr/io.h>
#endif

//
// Project's libraries
//
#include "mbesMock.h"
#include "mbesUtilities.h"
#include "mbesSelector.h"

#if MOCK == 1
typedef enum _timer_cmd {
	GET,
	RESET,
	START
} timer_cmd;

static int fd = 0;
#endif

static uint8_t timerAvailabilityCounter = 0;
static bool    isInitialized            = false;


//------------------------------------------------------------------------------------------------------------------------------
//                                     P R I V A T E   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
#if MOCK == 1
int ubRead (void *bytes, uint8_t size) {
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


int wTimer (timer_cmd cmd) {
	//
	// Description:
	//	Because the POSIX timer are only countdown ones, this function allow you to simulate an ARM monothonic timer
	//	The funcion accepts only one argument: the command. It selects the following feature: GET, RESET, START
	//
	struct timeval         tv;
	static struct timeval  t0;
	int                    out;
	static bool            enabled = false;
	
	if (gettimeofday(&tv, NULL) < 0) {
		ERRORBANNER(127)
		fprintf(stderr, "gettimeofday() failed: %s\n", strerror(errno));
		_exit(127);
		 
	} else if (cmd == RESET) 
		enabled = false;
		
	else if (cmd == GET && enabled)
		out = (tv.tv_sec - t0.tv_sec)*1000 + (tv.tv_usec - t0.tv_usec)/1000;
	
	else {
		enabled = true;
		t0 = tv;
	}
	return(out);
}
#endif


static void _pullUpEnabling (const char *code) {
	//
	// Description:
	//	It enable the pull-up resistor for the argument defined input pin
	//
#if MOCK == 0
	char    port;
	uint8_t pinNumber;

	codeConverter(code, &port, &pinNumber);
	if      (port == 'A') PORTA |= (1 << pinNumber);
	else if (port == 'B') PORTB |= (1 << pinNumber);
	else if (port == 'C') PORTC |= (1 << pinNumber);
	else if (port == 'D') PORTD |= (1 << pinNumber);
	else {
		// ERROR!
		logMsg("ERROR(%d)! \"%c\" is not a valid port\n", __LINE__, port);
	}
#endif

	return;
}


static bool _getPinValue (const char *code) {
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
	} else if (ubRead(&numOfRec, 1) != 1) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(stderr, "I/O operation failed: %s\n", strerror(errno));
		_exit(127);
		
	} else {
		char buffer[3];
		
		// Data reading
		for (uint8_t t=0; t<numOfRec; t++) {
			if (ubRead(buffer, 3) != 3) {
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


static void _timer_init() {
	//
	// Description:
	//	This function configures the ATmega16's 16-bit timer. It should be called one time only
	//
#if MOCK == 0
	
	// Normal mode setting...
	TCCR1A &= ~((1 << WGM10) | (1 << WGM11));
	TCCR1B &= ~((1 << WGM12) | (1 << WGM13));
	
#else
#endif
	
	isInitialized = true;
	
	return;
}


static void _timer_reset() {
	//
	// Description:
	//	This function stops and resets the timer
	//
#if MOCK == 0
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // Prescaler == 0 --> timer stopped (disabled)
	TCNT1 = 0;
#else
	wTimer(RESET);
#endif

	return;
}


static void _timer_start() {
	//
	// Description:
	//	It sets the prescaler to 1024. This action implies the timer starts itself
	//
#if MOCK == 0
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);
#else
	wTimer(START);
#endif
	return;
}


static uint16_t _timer_gettime() {
	//
	// Description:
	//	It return the current timer's value
	//
#if MOCK == 1
	return(wTimer(GET));
#else
	return(TCNT1);
#endif
}

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
}
#endif

void mbesSelector_init (struct mbesSelector *item, selectorType type, const char *pin) {
	//
	// Description:
	//	Thi function initializes the argument defined input selector (button, switch, hold-button...) 
	//
	
	// 16bit-timer initialization...
	if (isInitialized == false) {
		logMsg("mbesSelector module initialization...\n");
		_timer_init();

#if MOCK == 1
		// Virtual selectors file opening
		if ((fd = open(MBES_VIRTUALSEVECTOR_SWAPFILE, O_RDONLY)) && fd < 0) {
			// ERROR!
			ERRORBANNER(127)
			fprintf(stderr, "I cannot open the \"%s\" file\n", MBES_VIRTUALSEVECTOR_SWAPFILE);
			_exit(127);
		}
#endif
		
	}
	
	// PINs direction setting
	pinDirectionRegister(pin, INPUT);
	
	// PullUP resistor setting
	_pullUpEnabling(pin);
	
	item->pin[0]  = pin[0];
	item->pin[1]  = pin[1];
	item->pin[2]  = '\0';
	item->myTime  = 0;
	item->devType = type;
	item->status  = false;        // released status
	item->fsm     = 1;
	
	return;
}


bool mbesSelector_get (struct mbesSelector item) {
	//
	// Description:
	//	It returns the control device's status. The true value mining depends by the device-type as shown following:
	//		button)        the user is pushing it
	//		switch)        it is placed on ON-position
	//		holdon-button) it has been activated
	//
	return(item.status);
}


void mbesSelector_update (struct mbesSelector *item) {
	//
	// Description:
	//	This function updates the internal representation of the phisical device (button/switch..).
	//
	
	if (item->fsm == 1) {
		// It is the initial status: selector has been not yet activated. So its value is "false"
		// The selector is ready to be activated
		
		item->status  = false;
		
		//
		// The button/switch... has been pressed/activated
		//
		if (_getPinValue(item->pin) == false) {
#if MOCK == 0
			logMsg("Selector on pin-%s: just PUSHED/SWITCHED-ON\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "Selector on pin-%s: just PUSHED/SWITCHED-ON\n", item->pin);
			MYSYSLOG(LOG_INFO, "Active timers: %d\n", timerAvailabilityCounter);
#endif
			timerAvailabilityCounter++;
			if (_timer_gettime() == 0) {
				// Nobody is using the timer, I started it
				item->myTime = 0;
				_timer_start();
			} else {
				item->myTime = _timer_gettime();
			}
			item->fsm     = 2;
			item->status  = true;
		}
		
	
	} else if (item->fsm == 2) {
		// If the object is in this state, then the associated selector has been pressed/switched-on.
		// The selector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be released/deactivated
		//
		if (item->myTime + MBESSELECTOR_DEBOUNCETIME < _timer_gettime()) {
#if MOCK == 0
			logMsg("Selector on pin-%s: ACTIVE\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "Selector on pin-%s: ACTIVE\n", item->pin);
			MYSYSLOG(LOG_INFO, "(%d) Active timers: %d\n", __LINE__, timerAvailabilityCounter);
#endif
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm    = 3;
			// item->status is still true;
		}
		MYSYSLOG(LOG_INFO, "deathline: %d/%d", _timer_gettime(), (item->myTime + MBESSELECTOR_DEBOUNCETIME));
		
	} else if (item->fsm == 3) {
		// The selector is ready to be released/switched-off
		// New activities will be acknowledged
		
		//
		// Selector releasing...
		//
		if (	
			(item->devType == HOLDBUTTON && _getPinValue(item->pin) == false) ||
			(item->devType != HOLDBUTTON && _getPinValue(item->pin) == true)
		) {
			timerAvailabilityCounter++;
			if (_timer_gettime() == 0) {
				// Nobody is using the timer, I started it
				item->myTime = 0;
				_timer_start();
			} else {
				item->myTime = _timer_gettime();
			}
			if (item->devType == HOLDBUTTON) {
#if MOCK == 0
				logMsg("The hold-button (pin-%s) has been RELEASED\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "The hold-button (pin-%s) has been RELEASED\n", item->pin);
#endif
				item->fsm = 4;
				// item->status is still true (GND)
			} else {
#if MOCK == 0
				logMsg("The button/switch (pin-%s) has been RELEASED/SWITCHED-OFF\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "The button/switch (pin-%s) has been RELEASED/SWITCHED-OFF\n", item->pin);
#endif
				item->fsm    = 14;
				item->status = false;
			}
	
		}
	} else if (item->fsm == 14) {
		// If the object is in this state, then the associated selector has been released/switched-off.
		// The selector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be pressed/activated, again
		//
		if (item->myTime + MBESSELECTOR_DEBOUNCETIME < _timer_gettime()) {
#if MOCK == 0
			logMsg("Selector on pin-%s: NOT-ACTIVE\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "Selector on pin-%s: NOT-ACTIVE\n", item->pin);
			MYSYSLOG(LOG_INFO, "(%d) Active timers: %d\n", __LINE__, timerAvailabilityCounter);
#endif
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm = 1;
			// item->status is still false (VCC)
		}

		MYSYSLOG(LOG_INFO, "deathline: %d/%d", _timer_gettime(), (item->myTime + MBESSELECTOR_DEBOUNCETIME));
	 
	} else if (item->fsm == 4) {
		// If the object is in this state, then the associated selector is an hold-button and it has been released.
		// The selector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be pressed/activated, again
		//
		if (item->myTime + MBESSELECTOR_DEBOUNCETIME < _timer_gettime()) {
#if MOCK == 0
			logMsg("Selector on pin-%s is active and available\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "Selector on pin-%s is active and available\n", item->pin);
#endif
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm = 5;
		}
	
	
	} else if (item->fsm == 5) {
		// The selector is an hold-button and it ready to be deactivated
		// New activities will be acknowledged
	
		if (_getPinValue(item->pin) == false) {
#if MOCK == 0
			logMsg("The hols-button (pin-%s) has been pushed again\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "The hols-button (pin-%s) has been pushed again\n", item->pin);
#endif
			timerAvailabilityCounter++;
			if (_timer_gettime() == 0) {
				// Nobody is using the timer, I started it
				item->myTime = 0;
				_timer_start();
			} else {
				item->myTime = _timer_gettime();
			}
			item->fsm     = 6;
			item->status  = false;
		}
		
		
	} else if (item->fsm == 6) {
		// The selector is waiting to come back available.
		// The selector will be disabled for a time slot
		
		if (item->myTime + MBESSELECTOR_DEBOUNCETIME < _timer_gettime()) {
#if MOCK == 0
			logMsg("Selector on pin-%s is NOT-ACTIVE and available\n", item->pin);
#else
			MYSYSLOG(LOG_INFO, "Selector on pin-%s is NOT-ACTIVE and available\n", item->pin);
#endif
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm = 1;
			// item->status is still false (VCC)
		}
	
	}
	return;
}
