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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <syslog.h>

# else
#include <avr/io.h>
#endif

#include <stdio.h>

//
// Project's libraries
//
#include <mbesMock.h>
#include <debugTools.h>
#include <tools/debugConsole.h>
#include <mbesUtilities.h>
#include <mbesSelector.h>


#if MOCK == 1
typedef enum _timer_cmd {
	GET,
	RESET,
	START
} timer_cmd;
#endif

//
// Status log messages in not-MOCKed mode
//
#if MBES_SELECTOR_DEBUG > 1
#define NOP_LOGMSG(X)     logMsg(PSTR(X));
#define W1P_LOGMSG(X,Y)   logMsg(PSTR(X), Y);
#define W2P_LOGMSG(X,Y,Z) logMsg(PSTR(X), Y, Z);
#else
#define NOP_LOGMSG(X)     ;
#define W1P_LOGMSG(X,Y)   ;
#define W2P_LOGMSG(X,Y,Z) ;
#endif

//
// ERROR messages in not-MOCKed mode
//
#if MBES_SELECTOR_DEBUG > 0
#define LOGERR   logMsg(PSTR("ERROR! in %s(%d)"), __FUNCTION__, __LINE__);
#else
#define LOGERR   ;
#endif

#if MBES_KEEPTRACK > 0
#define KEEPTRACK(X, Y) keepTrack(X, Y);
#else
#define KEEPTRACK(X, Y) ;
#endif


static uint8_t timerAvailabilityCounter = 0;
static bool    isInitialized            = false;

static struct mbesSelector *itemsStorage[MBESSELECTOR_MAXITEMSNUMB];

//------------------------------------------------------------------------------------------------------------------------------
//                                     P R I V A T E   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

#if MOCK == 1
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


static void _timer_init() {
	//
	// Description:
	//	This function configures the ATmega16's 16-bit timer. It should be called one time only
	//
#if MOCK == 0
	
	// Normal mode setting...
	TCCR1A &= ~((1 << WGM10) | (1 << WGM11));
	TCCR1B &= ~((1 << WGM12) | (1 << WGM13));
	
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


static uint8_t _update (struct mbesSelector *item) {
	//
	// Description:
	//	This function updates the internal representation of the phisical device (button/switch..).
	//
	uint8_t pinValue = 1;
	uint8_t ec = 1;

	
	if (item->fsm == 1) {
		
		//
		// The button/switch... has been pressed/activated
		//
		if (getPinValue(item->pin, &pinValue) == 0) {
			// ERROR!
			LOGERR
			
			//TODO: reset
			ec = 0;

		} else if (pinValue == 0) {
			if (item->devType == BUTTON || item->devType == HOLDBUTTON) {
				W1P_LOGMSG("(%s) button has been PUSHED", item->pin);
				W2P_LOGMSG("(%d) Active timers: %d", __LINE__, timerAvailabilityCounter);
				
				timerAvailabilityCounter++;
				if (_timer_gettime() == 0) {
					// Nobody is using the timer, I started it
					item->myTime = 0;
					_timer_start();
				} else {
					item->myTime = _timer_gettime();
				}
				item->fsm     = 2;

				if (item->devType == BUTTON)
					// Button's value is "true"
					item->status  = true;
				else
					// Swapping the old value
					item->status  = item->status ? false : true; 

			} else if (item->devType == SWITCH) {
				W1P_LOGMSG("(%s) switch has moved to ON", item->pin);
				item->fsm     = 3;
				item->status  = true;

			}
		}
		
	
	} else if (item->fsm == 2) {
		// If the object is in this state, then the associated selector has been pressed/switched-on.
		// The selector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be released/deactivated
		//
		if (item->myTime + MBESSELECTOR_DEBOUNCETIME < _timer_gettime()) {
			W1P_LOGMSG("(%s) selector: ACTIVE\n", item->pin)
			W2P_LOGMSG("(%d) Active timers: %d\n", __LINE__, timerAvailabilityCounter);
			
			timerAvailabilityCounter--;
			item->fsm    = 3;
			// item->status is still true;
		} else {
			W2P_LOGMSG("deathline: %d/%d", _timer_gettime(), (item->myTime + MBESSELECTOR_DEBOUNCETIME))
		}
		
		
	} else if (item->fsm == 3) {
		// The selector is ready to be released/switched-off
		// New activities will be acknowledged
		
		if (getPinValue(item->pin, &pinValue) == 0) {
			// ERROR!
			LOGERR
			
			//TODO: reset
			ec = 0;
			
		} else if (pinValue == 1) {
			//
			// Selector releasing...
			//
			if (item->devType == BUTTON || item->devType == HOLDBUTTON) {
				W1P_LOGMSG("(%s) button released", item->pin);
				
				timerAvailabilityCounter++;
				if (_timer_gettime() == 0) {
					// Nobody is using the timer, I started it
					item->myTime = 0;
					_timer_start();
				} else {
					item->myTime = _timer_gettime();
				}
				
				item->fsm = 14;
				
				if (item->devType == BUTTON)
					item->status = false;

			} else if (item->devType == SWITCH) {
				W1P_LOGMSG("(%s) switch moved to OFF", item->pin);
				item->status = false;
				item->fsm = 1;
			}
	
		}
	} else if (item->fsm == 14) {
		// If the object is in this state, the selector is a button (or hold button) and it has been released.
		// The seelector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be pressed/activated, again
		//
		if (item->myTime + MBESSELECTOR_DEBOUNCETIME < _timer_gettime()) {
			W1P_LOGMSG("(%s) button's value is false", item->pin);
			W2P_LOGMSG("(%d) Active timers: %d", __LINE__, timerAvailabilityCounter);

			if (item->devType == BUTTON)
				timerAvailabilityCounter--;

			// End of cycle
			item->fsm = 1;
			
		} else 
			W2P_LOGMSG("deathline: %d/%d", _timer_gettime(), (item->myTime + MBESSELECTOR_DEBOUNCETIME))
		
	}
	
	//W2P_LOGMSG("mbesSelector_update(): PIN-%s = %d", item->pin, item->status);
	
	if (timerAvailabilityCounter == 0) _timer_reset();
	
	return(ec);
}
//------------------------------------------------------------------------------------------------------------------------------
//                                           P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

uint8_t mbesSelector_init (struct mbesSelector *item, selectorType type, const char *pin) {
	//
	// Description:
	//	The function initializes the argument defined input selector (button, switch, hold-button...) 
	//
	// Returned value:
	//	0   Some problem occurred with the I2C connected device
	//	1   SUCCESS
	//
	uint8_t        ec = 1;
	static uint8_t index = 0;
	
	// 16bit-timer initialization...
	if (isInitialized == false) {
		NOP_LOGMSG("mbesSelector module initialization...")

		_timer_init();

		for (uint8_t t=0; t<MBESSELECTOR_MAXITEMSNUMB; t++)
			itemsStorage[t] = NULL;
			
		isInitialized = true;
	}
	
	// PIN direction and PULL-UP resistor setting
	ec = pinDirectionRegister(pin, INPUT);

	if (ec == 1) {
		item->pin[0]  = pin[0];
		item->pin[1]  = pin[1];
		item->pin[2]  = '\0';
		item->myTime  = 0;
		item->devType = type;
		item->status  = false;        // released status
		item->fsm     = 1;

		itemsStorage[index] = item;
		index++;
		
	} else {
		// ERROR! (MPC23008 problems)
		LOGERR
	}
	
	return(ec);
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


uint8_t mbesSelector_update (struct mbesSelector *item) {
	//
	// Description:
	//	It upodates the argument defined item's FSM, or the ones of all registered itels
	//	This function is a frontend of _update() private function, mainly
	//	
	// Returned value:
	//	The number of successfully updated items
	//	
	
	uint8_t out = 0;
	if (item != NULL)
		out = _update(item) ? 1 : 0;
	else {
		uint8_t t = 0;
		while (t<MBESSELECTOR_MAXITEMSNUMB && itemsStorage[t] != NULL)
			if (_update(itemsStorage[t])) out++;
	}

	return(out);
}
