/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   iInputInterface.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module is responsible on the input controls (eg. buttons, switches..) management
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
//
// POSIX's libraries
//
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
//
// Platform dependent libraries
//
#include "driver/gpio.h"
#include "esp_timer.h"

//
// ESP-IDF libraries
//
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include "esp_err.h"
#include <esp_log.h>
#include <inttypes.h>
#endif

#include <stdio.h>

//
// Project's libraries
//
#include <mock.h>
#include <werror.h>
#include <debugConsoleAPI.h>
#include <iInputInterface.h>

#ifndef MOCK
#define MOCK 0
#endif


//
// ERROR messages in not-MOCKed mode
//
#if MOCK == 0
#define LOGERR   ESP_LOGE(__FILE__, "ERROR(%d)! in %s()", __LINE__, __FUNCTION__);
#else
#define LOGERR   fprintf(stderr, "ERROR(%d)! in %s()", __LINE__, __FUNCTION__);
#endif

//------------------------------------------------------------------------------------------------------------------------------
//                                     P R I V A T E   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
/*
static void _iInputItem_print (iInputItem_t obj) {
	//
	// JUST FOR DEBUG
	//
	ESP_LOGI(
		__FUNCTION__, "pinID:%d, type:%d, timerOffset:%ld, status:%s, FSM:%d",
		obj.pinID, obj.type, (long unsigned int)obj.timerOffset, obj.status ? "TRUE" : "FALSE", obj.FSM
	);
	return;
}
*/



static void _iInputInterface_update(iInputItem_t *item) {
	//
	// Description:
	//	This function updates the internal representation of the phisical device (button/switch..).
	//
	//
	//	          (devType = switch)
	//	+--------------------------------------+
	//	|                                      |
	//	|   +------+        +------+        +--+---+        +------+
	//	+-->|      |        |      |        |      |        |      |
	//	    |  01  +------->|  02  |------->|  03  |------->|  14  |
	//	+-->|      |        |      |    +-->|      |        |      |
	//	|   +--+---+        +------+    |   +------+        +---+--+
	//	|       |                       |                       |
	//	|       +-----------------------+                       |
	//	|           (devType = switch)                          |
	//	+-------------------------------------------------------+
	//
	static uint32_t myTimer = 0; 
	
	// [!] Enable the following line to debug this function
	//_iInputItem_print(*item);
		
	if (item->FSM == 0) {
		item->FSM = 1;
	
	} else if (item->FSM == 1) {
		//
		// Waiting for the button/switch... pressing event
		//
		if (keepTrack_getGPIO(item->pinID) == 0) {
				
			if (item->type == BUTTON || item->type == HOLDBUTTON) {
				ESP_LOGW(__FUNCTION__, "button-%d has been PUSHED", item->pinID);
				item->timerOffset = myTimer;
				item->FSM = 2;

				if (item->type == BUTTON)
					// Button's value is "true"
					item->status  = true;
				else
					// Swapping the old value
					item->status  = item->status ? false : true; 
				
			} else if (item->type == SWITCH) {
				// [!] The switch device does not need debouncing service
				ESP_LOGW(__FUNCTION__, "switch-%d has moved to ON", item->pinID);
				item->FSM     = 3;
				item->status  = true;

			}
		}
	 
		
	} else if (item->FSM == 2) {
		// If the object is in this state, then the associated selector has been pressed/switched-on.
		// The selector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be released/deactivated
		//
		if (item->timerOffset + IINPUTIF_DEBOUNCETIME < myTimer) {
			ESP_LOGI(__FUNCTION__, "inputInterface-%d is available now!", item->pinID);
			item->FSM = 3;
			item->timerOffset = 0;
			
		} else {
			ESP_LOGI(__FUNCTION__, "inputInterface-%d temporary unavailable (%ld/%ld)", item->pinID,
				(long unsigned int)(myTimer - item->timerOffset), (long unsigned int)IINPUTIF_DEBOUNCETIME
			);
		}
			
			
	} else if (item->FSM == 3) {
		// The selector is ready to be released/switched-off
		// New activities will be acknowledged
		
		if (keepTrack_getGPIO(item->pinID) == 1) {

			//
			// Selector releasing...
			//
			if (item->type == BUTTON || item->type == HOLDBUTTON) {
				ESP_LOGI(__FUNCTION__, "button-%d released", item->pinID);
				item->timerOffset = myTimer;
				
				item->FSM = 14;
				
				if (item->type == BUTTON)
					item->status = false;

			} else if (item->type == SWITCH) {
				ESP_LOGI(__FUNCTION__, "switch-%d move to OFF", item->pinID);
				item->status = false;
				item->FSM = 1;
			}
	
		}
			
			
	} else if (item->FSM == 14) {
		// If the object is in this state, the selector is a button (or hold button) and it has been released.
		// The seelector will be disabled for a time slot
		
		//
		// The button/switch... is ready to be pressed/activated, again
		//
		if (item->timerOffset + IINPUTIF_DEBOUNCETIME < myTimer) {
			ESP_LOGI(__FUNCTION__, "inputInterface-%d is available now!", item->pinID);
			item->FSM = 1;
			item->timerOffset = 0;
			
		} else 
			ESP_LOGI(__FUNCTION__, "inputInterface-%d temporary unavailable (%ld/%ld)", item->pinID,
				(long unsigned int)(myTimer - item->timerOffset), (long unsigned int)IINPUTIF_DEBOUNCETIME
			);
	}
		
	myTimer++;
	
	
	return;
}


static void _iInputInterface_updateAll() {
	//
	// Description:
	//	This function sends all registered item to _iInputInterface_update() function, one be one.
	//	[!] If the db is already-in-use then the function will retry indefinitely
	//
	// Returned value:
	//	WERRCODE_WARNING_RESNOTAV    // Resource is not available
	//	The returned-by-moduleDB_iter() code
	//
	werror       ec = WERRCODE_SUCCESS;
	uint8_t      inputID;
	uint8_t      retryCounter = 10;
	iInputItem_t item;
	
	// Iterator resetting...
	moduleDB_iter(NULL, NULL);
	
	while (wErrCode_isError(ec) == false) {
		ec = moduleDB_iter(&inputID, &item);
		
		if (wErrCode_isWarning(ec)) {
			//
			// WARNING!
			//
			ESP_LOGW(__FUNCTION__, "WARNING! internal db resource was busy");
			
			vTaskDelay(1 / portTICK_PERIOD_MS);
			if (retryCounter > 0)
				retryCounter--;
			else
				// ERROR!
				ec = WERRCODE_WARNING_RESNOTAV;
				
		} else if (wErrCode_isError(ec) && ec != WERRCODE_ERROR_DATAOVERFLOW) {
			//
			// ERROR!
			//
			ESP_LOGE(__FUNCTION__, "ERROR! moduleDB_iter() returned %d", ec);
		
		
		} else if (wErrCode_isSuccess(ec)) {
			//
			// SUCCESS
			//
			// ESP_LOGI(__FUNCTION__, "item-%d updating...", inputID);
			_iInputInterface_update(&item);
			
			// DB updating with the updated item data
			ec = moduleDB_rw(inputID, &item, MODULEDB_WRITE);
		}
	}
	
	return;
}
	
//------------------------------------------------------------------------------------------------------------------------------
//                                           P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

werror iInputInterface_init () {
	//
	// Description:
	//	Module's initialization. This is the first function the user has to call
	//	The timer period should be 5ms
	//
	// Returned value:
	//	WERRCODE_SUCCESS            Module successfully initialized
	//	WERRCODE_ERROR_INITFAILED   HW timer initialization failed
	//
	uint8_t                 ec = WERRCODE_SUCCESS;
	esp_timer_create_args_t timerArgs;
	esp_timer_handle_t      timerHandle;

	// Timer configuration
	timerArgs.callback              = _iInputInterface_updateAll;
	timerArgs.arg                   = NULL;
	timerArgs.dispatch_method       = ESP_TIMER_TASK;
	timerArgs.name                  = "iInputIntercafe-updater-proc";
	timerArgs.skip_unhandled_events = true;	
	
	if (
		esp_timer_create(&timerArgs, &timerHandle)    != ESP_OK ||
		esp_timer_start_periodic(timerHandle, 100000) != ESP_OK
	) {
            // ERROR!
            ESP_LOGE(__FUNCTION__, "ERROR! I cannot crate the timer");
		ec = WERRCODE_ERROR_INITFAILED;
	}
	return(ec);
}

werror iInputInterface_new (uint8_t *inputID, iInputType_t type, int8_t pin) {
	//
	// Description:
	//	It create a new internal object to manage the argument defined input-control and returns its numeric id
	//
	// Returned value:
	//	WERRCODE_SUCCESS             Interface has been correctly created
	//	WERRCODE_ERROR_SYSCALL       The GPIO-configuration API failed
	//	moduleDB_add() error codes
	//
	werror         ec = WERRCODE_SUCCESS;
	gpio_config_t  phyPin;
	
	// PIN direction and PULL-UP resistor setting
	phyPin.intr_type    = GPIO_INTR_DISABLE;
	phyPin.mode         = GPIO_MODE_INPUT;
	phyPin.pin_bit_mask = (1ULL << pin);
	phyPin.pull_down_en = GPIO_PULLDOWN_DISABLE;
	phyPin.pull_up_en   = GPIO_PULLUP_ENABLE;

	if (gpio_config(&phyPin) != ESP_OK) {
		// ERROR!
		LOGERR
		ec = WERRCODE_ERROR_SYSCALL;
	
	} else {
		iInputItem_t it = {	
			.pinID       = pin,
			.type        = type,
			.timerOffset = 0,
			.status      = false,
			.FSM         = 0
		};
		
		ec = moduleDB_add (inputID, it);
	}
	
	return(ec);
}

werror iInputInterface_get (uint8_t inputID, bool *currStat) {
	//
	// Description:
	//	It looks for interface with id equals to the argument defined one, and writes its status in the memory area
	//	pointed by the currStat argument
	//
	// Returned value:
	//	moduleDB_rw() error codes
	//
	iInputItem_t it;
	werror       ec = moduleDB_rw(inputID, &it, MODULEDB_READ);
	
	if (wErrCode_isSuccess(ec) && currStat != NULL)
		*currStat = it.status;
		
	return(ec);
}
