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
//
//
//
//
//
//
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
#include "mbesMock.h"
#include "mbesUtilities.h"
#include "mbesSelector.h"

static uint8_t timerAvailabilityCounter = 0;
static bool    isInitialized            = false;


static void _pullUpEnabling (const char *code) {
	//
	// Description:
	//	It enable the pull-up resistor for the argument defined input pin
	//
#if MOCK == 1
	char    port;
	uint8_t pinNumber;

	codeConverter(code, &port, &pinNumber);
	if      (port == 'A') PORTA |= (1 << pinNumber);
	else if (port == 'B') PORTB |= (1 << pinNumber);
	else if (port == 'C') PORTC |= (1 << pinNumber);
	else if (port == 'D') PORTD |= (1 << pinNumber);
	else {
		// ERROR!
	}
#endif

	return;
}

static bool _getPinValue (const char *code) {
	//
	// Description:
	//	It returns the argument defined input pin's value
	//
#if MOCK == 1
	char    port;
	uint8_t pinNumber;
	uint8_t out = 0;

	_codeConverter(code, &port, &pinNumber);
	
	if      (port == 'A') out = (PINA & (1 << pinNumber));
	else if (port == 'B') out = (PINB & (1 << pinNumber));
	else if (port == 'C') out = (PINC & (1 << pinNumber));
	else if (port == 'D') out = (PIND & (1 << pinNumber));
	else {
		// ERROR!
	}

	return((out > 0) ? true : false);
#else
#endif
}


static void _timer_init() {
	//
	// Description:
	//	This function configures the ATmega16's 16-bit timer. It should be called one time only
	//
#if MOCK == 1
	
	// Normal mode setting...
	TCCR1A &= ~((1 << WGM10) | (1 << WGM11));
	TCCR1B &= ~((1 << WGM12) | (1 << WGM13));
	
	isInitialized = true;
#else
#endif
	
	return;
}


static void _timer_reset() {
	//
	// Description:
	//	This function stops and resets the timer
	//
#if MOCK == 1
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // Prescaler == 0 --> timer stopped (disabled)
	TCNT1 = 0;
	
#else
#endif
	return;
}


static void _timer_start() {
	//
	// Description:
	//	It sets the prescaler to 1024. This action implies the timer starts itself
	//
#if MOCK == 1
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TCCR1B &= ~(1 << CS11);
	
#else
#endif
	return;
}


//------------------------------------------------------------------------------------------------------------------------------
//                                           P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------


void mbesSelector_init (struct mbesSelector *item, selectorType type, const char *pin, uint32_t *timer) {
	//
	// Description:
	//	Thi function initializes the argument defined input selector (button, switch, hold-button...) 
	//
	
	// 16bit-timer initialization...
	if (isInitialized == false) {
		logMsg("mbesSelector module initialization...\n");
		_timer_init();
	}
	
	// PINs direction setting
	pinDirectionRegister(pin, INPUT);
	
	// PullUP resistor setting
	_pullUpEnabling(pin);
	
	item->pin[0] = pin[0];
	item->pin[1] = pin[1];
	item->pin[2] = '\0';
	pin->myTime  = 0;
	pin->devType = type;
	pin->status  = false;        // released status
	pin->enabled = true;
	
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


void mbesSelector_update (struct mbesSelector item) {
	//
	// Description:
	//	This function updates the internal representation of the phisical device (button/switch..).
	//
	
	if (item.status == false && item.enabled == true && _getPinValue(item.pin) == false) {
		//
		// The button/switch... has been pressed/activated
		//
		item.status  = true;
		item.enabled = false;
		timerAvailabilityCounter++;
		
		if (TCNT1 == 0) {
			// Nobody is using the timer, I started it
			item.Time = 0;
			_timer_start();
		} else {
			item.Time = TCNT1;
		}
		
		logMsg("Selector on pin-%s: just PUSHED/SWITCHED-ON\n", item.pin);
	
	} else if (status == true && item.enabled == false && item.myTime + MBESSELECTOR_DEBOUNCETIME < TCNT1) {
		//
		// The button/switch... is ready to be released/deactivated
		//
		item.status  = true;
		item.enabled = true;
		timerAvailabilityCounter--;
		if (timerAvailabilityCounter == 0) _timer_reset();
		
		logMsg("Selector on pin-%s: ACTIVE\n", item.pin);
	
	
	} else if (status == true && item.enabled == true && _getPinValue(item.pin) == false) {
		item.status  = false;
		item.enabled = false;
		timerAvailabilityCounter++;
		 
		if (TCNT1 == 0) {
			// Nobody is using the timer, I started it
			item.Time = 0;
			_timer_start();
		} else {
			item.Time = TCNT1;
		}
		
		logMsg("Selector on pin-%s: just RELEASED/SWITCHED-OFF\n", item.pin);
	
	
	} else if (status == false && item.enabled == false && item.myTime + MBESSELECTOR_DEBOUNCETIME < TCNT1) {
		//
		// The button/switch... is ready to be pressed/activated
		//
		item.status  = false;
		item.enabled = true;
		timerAvailabilityCounter--;
		if (timerAvailabilityCounter == 0) _timer_reset();
	
		logMsg("Selector on pin-%s: NOT-ACTIVE\n", item.pin);
	}
	
	 
	return;
}
