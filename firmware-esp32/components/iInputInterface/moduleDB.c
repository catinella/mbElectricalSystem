/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   moduleDB.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This source file provides a simple data-storage service with concurrent accesses management
//
//
//	Error codes convention:
//	=======================
//		+--------+-----------------------------------------------------+
//		| Codes  | Description                                         |
//		+--------+-----------------------------------------------------+
//		|      0 | Generic error                                       |
//		|      1 | Success                                             |
//		|  32-63 | Information (it is associated to a success status)  |
//		| 64-127 | Warning                                             |
//		|   >127 | Specific error                                      |
//		+--------+-----------------------------------------------------+
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

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include "esp_err.h"
#include <esp_log.h>

#include <moduleDB.h>


static iInputItem_t      intDb[MODULEDB_MAXITEMSNUMB];
static uint8_t           DBsize = 0;
static SemaphoreHandle_t mtx;
static bool              initFlag = false;

//
// ERROR messages in not-MOCKed mode
//
#if MOCK == 0
#define LOGERR   ESP_LOGE(__FILE__, "ERROR(%d)! in %s()", __LINE__, __FUNCTION__);
#define LOGMARK  ESP_LOGW(__FILE__, "-------> %s():%d", __FUNCTION__, __LINE__);
#else
#define LOGERR   fprintf(stderr, "ERROR(%d)! in %s()", __LINE__, __FUNCTION__);
#endif

//------------------------------------------------------------------------------------------------------------------------------
//                                      P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
werror moduleDB_rw (uint8_t inputID, iInputItem_t *item, moduleDBopType_t op) {
	//
	// Description:
	//	This private function allows you to read or update an item in/by the internal database
	//
	// Arguments
	//	inputID                    The id of the object you are looking for information of it
	//	item                       The item you want to update, or the memory address where the found item will be copied
	//	op                         Operation type reading or writing one
	//
	// Returned value:
	//	WERRCODE_SUCCESS           OK
	//	WERRCODE_WARNING_RESBUSY   The resource was busy
	//	WERRCODE_ERROR_INITFAILED  Module has not been initialized
	//	WERRCODE_ERROR_ILLEGALARG  The object-id does not belong to the internal DB
	//	WERRCODE_ERROR_MUTEXOP     The mutex has been created in wrong way
	//
	werror ec = WERRCODE_SUCCESS;

	if (initFlag == false) {
		// ERROR!
		LOGERR
		ec = WERRCODE_ERROR_INITFAILED;
	
	} else if (DBsize < inputID) {
		// ERROR!
		ec = WERRCODE_ERROR_ILLEGALARG;
		
	} else if (xSemaphoreTake(mtx, MODULEDB_TIMEOUT) != pdTRUE) {
		// WARNING!
		ESP_LOGE(__FUNCTION__, "WARNING! resource busy");
		ec = WERRCODE_WARNING_RESBUSY;
			
	} else {
		if (item != NULL) {
			if (op == MODULEDB_READ) {
				*item = intDb[inputID];
				//ESP_LOGI(
				//	__FUNCTION__, "PIN=%d  status=%s", 
				//	intDb[inputID].pinID, intDb[inputID].status ? "ACTIVE" : "NOT-ACTIVE"
				//);
			} else {
				intDb[inputID] = *item;
			}
		}
		
		if (xSemaphoreGive(mtx) != pdTRUE) {
			// ERROR!
			ESP_LOGE(__FUNCTION__, "ERROR! I cannot release the db-access' mutex");
			ec = WERRCODE_ERROR_MUTEXOP;
		}
	}
	
	return(ec);
}

werror moduleDB_add (uint8_t *inputID, iInputItem_t item) {
	//
	// Description:
	//	
	//	
	// Returned value:
	//	WERRCODE_SUCCESS               Arg defined item has been successfully added to the managed ones list
	//	WERRCODE_ERROR_INITFAILED      Mutex creation failed
	//	WERRCODE_ERROR_DATAOVERFLOW    No further memory space to add the item
	//	WERRCODE_WARNING_RESBUSY       The internal db was already in-use by someonelse
	//	WERRCODE_ERROR_MUTEXOP         The mutex has been created in wrong way
	//
	werror      ec = WERRCODE_SUCCESS;
	
	if (initFlag == false) {
		if ((mtx = xSemaphoreCreateMutex()) == NULL) {
			ESP_LOGE(__FUNCTION__, "Mutex creation failed");
			ec = WERRCODE_ERROR_INITFAILED;
		} else {
			ESP_LOGI(__FUNCTION__, "OK! moduleDB has been initialized");
			DBsize = 0;
			initFlag = true;
		}
	}

	if (initFlag) {
		if ( DBsize == MODULEDB_MAXITEMSNUMB) {
			// ERROR!
			LOGERR
			ec = WERRCODE_ERROR_DATAOVERFLOW;
	
		} else {
			if (xSemaphoreTake(mtx, MODULEDB_TIMEOUT) == pdTRUE) {
				*inputID = DBsize;
				intDb[*inputID] = item;
				DBsize++;
			
				ESP_LOGI(__FUNCTION__, "OK! The object-%d has been correctly regitered", *inputID);
				
				if (xSemaphoreGive(mtx) != pdTRUE) {
					// ERROR!
					ESP_LOGE(__FUNCTION__, "ERROR! I cannot release the db-access' mutex");
					ec = WERRCODE_ERROR_MUTEXOP;
				
				} else
					// Small delay after the mutex has been released
					vTaskDelay(1 / portTICK_PERIOD_MS);
					
			} else {
				// WARNING!
				ESP_LOGE(__FUNCTION__, "WARNING! resource busy");
				ec = WERRCODE_WARNING_RESBUSY;
			}
		}
	}
	
	return(ec);
}


werror moduleDB_iter (uint8_t *inputID, iInputItem_t *item) {
	//
	// Description:
	//	This function is an iterator and allows you to read the sored items, sequentially.
	//	
	// Arguments:
	//	inputID  The ID of the current pointed object
	//	item     The object currently pointed by the iterator-reference
	//
	// Returned value:
	//	WERRCODE_SUCCESS              The operation has terminated with success
	//	WERRCODE_ERROR_DATAOVERFLOW   The iterator has already browsed all items
	//	The retuned by moduleDB_rw() code
	//
	static uint8_t it = 0;
	werror         ec = WERRCODE_SUCCESS;
	
	// Iterator resetting...
	if (inputID == NULL || item == NULL) {
		it = 0;
	
	} else if (it <= DBsize) {
		*inputID = it;
		ec = moduleDB_rw(*inputID, item, MODULEDB_READ);
		if (wErrCode_isSuccess(ec)) it++;
		
	} else {
		// ERROR!
		ec = WERRCODE_ERROR_DATAOVERFLOW;
	}
	
	return(ec);
}	
