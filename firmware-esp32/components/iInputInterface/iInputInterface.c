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

#if MBES_KEEPTRACK > 0
#define KEEPTRACK(X, Y) keepTrack(X, Y);
#else
#define KEEPTRACK(X, Y) ;
#endif

typedef struct {
	int8_t       pinID;
	iInputType   type;
	uint32_t     timerOffset;  // It allows the timer to count for about 1000 hours!!!! 
	bool         status;
	uint8_t      FSM;
} iInputItem_t;

typedef enum {
	INTDB_TAKE    = 0,
	INTDB_RELEASE = 1
} moduleDB_cmd_t;

typedef struct {
	iInputItem_t list[IINPUTIF_MAXITEMSNUMB];
	uint8_t      size;
} moduleDB_t;

//------------------------------------------------------------------------------------------------------------------------------
//                                     P R I V A T E   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
void _iInputItem_print (iInputItem_t obj) {
	//
	// JUST FOR DEBUG
	//
	ESP_LOGI(
		__FUNCTION__, "pinID:%d, type:%d, timerOffset:%ld, status:%s, FSM:%d",
		obj.pinID, obj.type, (long unsigned int)obj.timerOffset, obj.status ? "TRUE" : "FALSE", obj.FSM
	);
	return;
}

moduleDB_t* _moduleDB_get (moduleDB_cmd_t cmd) {
	//
	// Description:
	//	It returns a reference to its static db where all items information are stored. The function also manage race
	//	conditions using a mutex that you can control using the argument defined commans
	//
	// Arguments
	//	cmd         {INTDB_TAKE|INTDB_RELEASE}
	//
	// Returned value:
	//	NULL             ERROR!
	//	<db's address>   SUCCESS
	//
	static moduleDB_t        itemsStorage;
	static SemaphoreHandle_t mtx;
	static bool              flag = false;
	moduleDB_t               *out = NULL;

	//
	// Initialization
	//
	if (flag == false) {
		if ((mtx = xSemaphoreCreateMutex()) == NULL)
			ESP_LOGE(__FUNCTION__, "Mutex creation failed");
		else {
			ESP_LOGI(__FUNCTION__, "OK! moduleDB has been initialized");
			itemsStorage.size = 0;
			flag = true;
		}
	}

	if (cmd == INTDB_RELEASE) {
		//ESP_LOGI(__FUNCTION__, "---->%d", __LINE__);
		if (xSemaphoreGive(mtx) != pdTRUE)
			// ERROR!
			LOGERR
	
	} else if (cmd == INTDB_TAKE) {
		if (xSemaphoreTake(mtx, portMAX_DELAY) == pdTRUE)
			out = &itemsStorage;
			
		else {
			// ERROR!
			LOGERR
		}
	
	} else {
		// ERROR!
		LOGERR
	}

	return(out);
}


void _iInputInterface_update(uint8_t inputID) {
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
	moduleDB_t      *db = _moduleDB_get(INTDB_TAKE);
	static uint32_t myTimer = 0; 
	
	if (db == NULL)
		// WARNING
		ESP_LOGW(__FUNCTION__, "WARNING! I was unable to lock the module's db");
		
	else {
		// [!] Enable the following line to debug this function
		//_iInputItem_print(db->list[inputID]);
		
		if (db->list[inputID].FSM == 0) {
			db->list[inputID].FSM = 1;
	
		} else if (db->list[inputID].FSM == 1) {
			//
			// Waiting for the button/switch... pressing event
			//
			if (gpio_get_level(db->list[inputID].pinID) == 0) {
				KEEPTRACK_numID(db->list[inputID].pinID, 0);
				
				if (db->list[inputID].type == BUTTON || db->list[inputID].type == HOLDBUTTON) {
					ESP_LOGW(__FUNCTION__, "button-%d has been PUSHED", inputID);
					db->list[inputID].timerOffset = myTimer;
					db->list[inputID].FSM = 2;
	
					if (db->list[inputID].type == BUTTON)
						// Button's value is "true"
						db->list[inputID].status  = true;
					else
						// Swapping the old value
						db->list[inputID].status  = db->list[inputID].status ? false : true; 
					
				} else if (db->list[inputID].type == SWITCH) {
					// [!] The switch device does not need debouncing service
					ESP_LOGW(__FUNCTION__, "switch-%d has moved to ON", inputID);
					db->list[inputID].FSM     = 3;
					db->list[inputID].status  = true;
	
				}
			}
	 
		
		} else if (db->list[inputID].FSM == 2) {
			// If the object is in this state, then the associated selector has been pressed/switched-on.
			// The selector will be disabled for a time slot
			
			//
			// The button/switch... is ready to be released/deactivated
			//
			if (db->list[inputID].timerOffset + IINPUTIF_DEBOUNCETIME < myTimer) {
				ESP_LOGI(__FUNCTION__, "inputInterface-%d is available now!", inputID);
				db->list[inputID].FSM = 3;
				db->list[inputID].timerOffset = 0;
				
			} else {
				ESP_LOGI(__FUNCTION__, "inputInterface-%d temporary unavailable (%ld/%ld)", inputID,
					(long unsigned int)(myTimer - db->list[inputID].timerOffset), 
					(long unsigned int)IINPUTIF_DEBOUNCETIME
				);
			}
			
			
		} else if (db->list[inputID].FSM == 3) {
			// The selector is ready to be released/switched-off
			// New activities will be acknowledged
			
			if (gpio_get_level(db->list[inputID].pinID) == 1) {
				KEEPTRACK_numID(db->list[inputID].pinID, 1);

				//
				// Selector releasing...
				//
				if (db->list[inputID].type == BUTTON || db->list[inputID].type == HOLDBUTTON) {
					ESP_LOGI(__FUNCTION__, "button-%d released", inputID);
					db->list[inputID].timerOffset = myTimer;
					
					db->list[inputID].FSM = 14;
					
					if (db->list[inputID].type == BUTTON)
						db->list[inputID].status = false;
	
				} else if (db->list[inputID].type == SWITCH) {
					ESP_LOGI(__FUNCTION__, "switch-%d move to OFF", inputID);
					db->list[inputID].status = false;
					db->list[inputID].FSM = 1;
				}
		
			}
			
			
		} else if (db->list[inputID].FSM == 14) {
			// If the object is in this state, the selector is a button (or hold button) and it has been released.
			// The seelector will be disabled for a time slot
			
			//
			// The button/switch... is ready to be pressed/activated, again
			//
			if (db->list[inputID].timerOffset + IINPUTIF_DEBOUNCETIME < myTimer) {
				ESP_LOGI(__FUNCTION__, "inputInterface-%d is available now!", inputID);
				db->list[inputID].FSM = 1;
				db->list[inputID].timerOffset = 0;
				
			} else 
				ESP_LOGI(__FUNCTION__, "inputInterface-%d temporary unavailable (%ld/%ld)", inputID,
					(long unsigned int)(myTimer - db->list[inputID].timerOffset),
					(long unsigned int)IINPUTIF_DEBOUNCETIME
				);
		}
		
		myTimer++;
		_moduleDB_get(INTDB_RELEASE);
	}
	
	return;
}


void _iInputInterface_updateAll() {
	moduleDB_t *db = _moduleDB_get(INTDB_TAKE);

	if (db == NULL) {
		// WARNING
		ESP_LOGW(__FUNCTION__, "WARNING! I was unable to lock the module's db");
		
	} else {
		uint8_t noi = db->size;
		_moduleDB_get(INTDB_RELEASE);
		
		for (uint8_t x=0; x<noi; x++)
			_iInputInterface_update(x);
	}
	
	return;
}
	
//------------------------------------------------------------------------------------------------------------------------------
//                                           P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------

uint8_t iInputInterface_init () {
	//
	// Description:
	//	Module's initialization. This is the first function the user has to call
	//	The timer period should be 5ms
	//
	// Returned value:
	//	IINPUTIF_SUCCESS
	//	IINPUTIF_ERROR_TIMERAPI
	//
	uint8_t                 ec = IINPUTIF_SUCCESS;
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
		ec = IINPUTIF_ERROR_TIMERAPI;
	}
	return(ec);
}

uint8_t iInputInterface_new  (uint8_t *inputID, iInputType type, int8_t pin) {
	//
	// Description:
	//	It create a new internal object to manage the argument defined input-control and returns its numeric id
	//
	// Returned value:
	//	IINPUTIF_SUCCESS             Interface has been correctly created
	//	IINPUTIF_WARNING_RESBUSY     Interface not created because I was unable to get the db's mutex control
	//	IINPUTIF_ERROR_OVERFLOW      The nax interfaces number has been reach
	//	IINPUTIF_ERROR_GPIOAPI       The system GPIO API failed
	//
	uint8_t        ec = IINPUTIF_SUCCESS;
	gpio_config_t  phyPin;
	moduleDB_t     *db = _moduleDB_get(INTDB_TAKE);
	
	// PIN direction and PULL-UP resistor setting
	phyPin.intr_type    = GPIO_INTR_DISABLE;
	phyPin.mode         = GPIO_MODE_INPUT;
	phyPin.pin_bit_mask = (1ULL << pin);
	phyPin.pull_down_en = GPIO_PULLDOWN_DISABLE;
	phyPin.pull_up_en   = GPIO_PULLUP_ENABLE;

	if (db == NULL) {
		// WARNING!
		ESP_LOGW(__FUNCTION__, "WARNING! I cannot get the module's db control");
		ec = IINPUTIF_WARNING_RESBUSY;
		
	} else {
		if ( db->size == IINPUTIF_MAXITEMSNUMB) {
			// ERROR!
			LOGERR
			ec = IINPUTIF_ERROR_OVERFLOW;
			
		} else if (gpio_config(&phyPin) != ESP_OK) {
			// ERROR!
			LOGERR
			ec = IINPUTIF_ERROR_GPIOAPI;
			
		} else {
			ESP_LOGI(__FUNCTION__, "OK! The object-%d has been correctly regitered", db->size);
			
			db->list[db->size].pinID       = pin;
			db->list[db->size].type        = type;
			db->list[db->size].timerOffset = 0;
			db->list[db->size].status      = false;
			db->list[db->size].FSM         = 0;
			
			// The input-id used to by iInputInterface_get to find the object
			*inputID = db->size;
			
			db->size++;
		}
		_moduleDB_get(INTDB_RELEASE);
	}
	return(ec);
}

uint8_t iInputInterface_get (uint8_t inputID, bool *currStat) {
	//
	// Description:
	//	It looks for interface with id equals to the argument defined one, and writes its status in the memory area
	//	pointed by the currStat argument
	//
	// Returned value:
	//	IINPUTIF_SUCCESS
	//	IINPUTIF_WARNING_RESBUSY
	//	IINPUTIF_ERROR_ILLEGALARG
	//
	uint8_t    ec  = IINPUTIF_SUCCESS;
	moduleDB_t *db = _moduleDB_get(INTDB_TAKE);
	
	if (db == NULL)
		// WARNING!
		ec = IINPUTIF_WARNING_RESBUSY;
		
	else {
		if (db->size <= inputID)
			// ERROR!
			ec = IINPUTIF_ERROR_ILLEGALARG;
		else {
			*currStat = db->list[inputID].status;
			//ESP_LOGI(
			//	__FUNCTION__, "PIN=%d  status=%s", 
			//	db->list[inputID].pinID, db->list[inputID].status ? "ACTIVE" : "NOT-ACTIVE"
			//);
		}
		_moduleDB_get(INTDB_RELEASE);
	}
	
	return(ec);
}
