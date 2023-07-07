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
#include <signal.h>
#include <time.h>
#include <debugTools.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

# else
#include <avr/io.h>
#endif

#include "mbesMock.h"
#include "mbesUtilities.h"
#include "mbesSelector.h"

#if MOCK == 1
static timer_t timerId = 0;                     // POSIX timer's ID
static FILE    *fh     = NULL;
#endif
static uint8_t timerAvailabilityCounter = 0;
static bool    isInitialized            = false;


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

	// File locking
	if (flock(fileno(fh), LOCK_EX) < 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf(
			stderr, "I cannot lock the \"%s\" file, because flock() syscall failed: %s\n", 
			MBES_VIRTUALSEVECTOR_SWAPFILE, strerror(errno)
		);
		_exit(127);
	
	} else {
		char   *buffer = (char*)malloc(8*sizeof(char));
		size_t tempSize = 6; // [A-Z] [0-7] ':' [0,1] '\n' '\0'
		bool   end = false;
		int    eSize;
		
		rewind(fh);

		// Data reading
		while (end == false) {
			eSize = getline(&buffer, &tempSize, fh); 
			if (eSize > 0 && (eSize != 5 || buffer[2] != ':')) {
				// ERROR!
				ERRORBANNER(127)
				fprintf(stderr, "The \"%s\" file is corrupted\n", MBES_VIRTUALSEVECTOR_SWAPFILE);
				_exit(127);
			
			} else if (feof(fh)) {
				end = true;
			
			} else if (eSize < 0) {
				// ERROR!
				ERRORBANNER(127)
				fprintf(stderr, "I/O operation failed: %s", strerror(errno));
				_exit(127);
				
			} else {
				buffer[2] = '\0';
				if (strcmp(buffer, code) == 0) {
					if (buffer[3] == '0')
						out = 0;
					else if (buffer[3] == '1')
						out = 1;
					else {
						// ERROR!
						ERRORBANNER(127)
						fprintf(stderr, "Corrupted file\n");
						_exit(127);
					}
					break;
				}
			}
		}
		
		// Memory releasing
		free(buffer);
		
		// File unlocking
		if (flock(fileno(fh), LOCK_UN) < 0) {
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

	return((out > 0) ? true : false);
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
	struct sigevent sigEvent;
	
	memset(&sigEvent, 0, sizeof(struct sigevent));
	sigEvent.sigev_notify = SIGEV_NONE;
	
	if (timer_create(CLOCK_MONOTONIC, &sigEvent, &timerId) != 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf (stderr, "POSIX timer creation failed: %s\n", strerror(errno));
	}
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
	struct itimerspec newdl;
	newdl.it_interval.tv_sec  = 0;
	newdl.it_interval.tv_nsec = 0;
	newdl.it_value.tv_sec  = 0;
	newdl.it_value.tv_nsec = 0;
	
	if (timer_settime(timerId, 0, &newdl, NULL) < 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf (stderr, "Stopping and resetting POSIX timer failed: %s\n", strerror(errno));
	}
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
	struct itimerspec newdl;
	newdl.it_interval.tv_sec  = 1;
	newdl.it_interval.tv_nsec = 0;
	newdl.it_value.tv_sec  = 0;
	newdl.it_value.tv_nsec = 1000000;
	
	timer_settime(timerId, 0, &newdl, NULL);
#endif
	return;
}


static uint16_t _timer_gettime() {
	//
	// Description:
	//	It return the current timer's value
	//
#if MOCK == 1
	struct itimerspec t;
	uint16_t          out;
	
	if (timer_gettime(timerId, &t) < 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf (stderr, "POSIX timer reading failed: %s\n", strerror(errno));
	
	} else if (t.it_value.tv_sec > 0) {
		// ERROR!
		ERRORBANNER(127)
		fprintf (stderr, "Timer overflow!!\n");
	
	} else
		out = (t.it_value.tv_nsec/1000000); // It converts it from nanoseconds to milliseconds
	
	return(out);
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
	fclose(fh);
	timer_delete(timerId);
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
		if ((fh = fopen(MBES_VIRTUALSEVECTOR_SWAPFILE, "r")) && fh == NULL) {
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
			logMsg("Selector on pin-%s: just PUSHED/SWITCHED-ON\n", item->pin);
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
			logMsg("Selector on pin-%s: ACTIVE\n", item->pin);
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm    = 3;
			// item->status is still true;
		}
	
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
				logMsg("The hold-button (pin-%s) has been RELEASED\n", item->pin);
				item->fsm = 4;
				// item->status is still true (GND)
			} else {
				logMsg("The button/switch (pin-%s) has been RELEASED/SWITCHED-OFF\n", item->pin);
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
			logMsg("Selector on pin-%s: NOT-ACTIVE\n", item->pin);
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm = 1;
			// item->status is still false (VCC)
		}
	
	 
	} else if (item->fsm == 4) {
		// If the object is in this state, then the associated selector is an hold-button and it has been released.
		// The selector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be pressed/activated, again
		//
		if (item->myTime + MBESSELECTOR_DEBOUNCETIME < _timer_gettime()) {
			logMsg("Selector on pin-%s is active and available\n", item->pin);
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm = 5;
		}
	
	
	} else if (item->fsm == 5) {
		// The selector is an hold-button and it ready to be deactivated
		// New activities will be acknowledged
	
		if (_getPinValue(item->pin) == false) {
			logMsg("The hols-button (pin-%s) has been pushed again\n", item->pin);
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
			logMsg("Selector on pin-%s is NOT-ACTIVE and available\n", item->pin);
			timerAvailabilityCounter--;
			if (timerAvailabilityCounter == 0) _timer_reset();
			item->fsm = 1;
			// item->status is still false (VCC)
		}
	
	}
	return;
}
