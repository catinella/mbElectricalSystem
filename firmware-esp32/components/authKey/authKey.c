/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   authKey.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	The authorized key's value is stored in the ESP32's NVS partition. This library provides a functions to manage the
//	resistive key storing/reading/authentication process.
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

#include <stddef.h>
#include <stdbool.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include <authKey.h>
#include <ravgFilter.h>

static nvs_handle_t nvsHandle;
static bool         initFlag = false;
static ravg_t       ravgPool[AUTHKEY_DEEPLEV];
static const char   *authKey_labelsList[AUTHKEY_DEEPLEV] = {"refVal_A", "refVal_B"};

werror _authKey_init() {
	//
	// Description:
	//	This function initializes the NVS partition
	//
	werror err = WERRCODE_SUCCESS;

	if (
		nvs_flash_init()                               != ESP_OK ||
		nvs_open("authKey", NVS_READWRITE, &nvsHandle) != ESP_OK
	)
		// ERROR!
		err = WERRCODE_ERROR_INITFAILED;
	else {
		initFlag = true;
		
		for (uint8_t t=0; t<AUTHKEY_DEEPLEV; t++) {
			ravg_init((ravgPool + t));
		}
	}
	
	return(err);
}

/*
werror authKey_save (const authKey_t value) {
	//
	// Description:
	//	This function allows you to save a new key's value in the MCU's NVS partition
	//
	werror err = WERRCODE_SUCCESS;
	
	ESP_ERROR_CHECK(nvs_flash_erase());
	if (initFlag == false) err = nvs_flash_init();
	
	if (wErrCode_isSuccess(err)) {
		nvs_set_u32(handle, "refVal_A", value[0]);
		nvs_set_u32(handle, "refVal_A", value[1]);
	}
	
	return(err);
}
*/

werror authKey_read (authKey_t value) {
	//
	// Description:
	//	It reads the key's value stored in the NVS partition and writes that value in the argument defined memory area
	//
	// Returned code:
	//	WERRCODE_SUCCESS
	//	WERRCODE_ERROR_NVSIOFAILED
	//	WERRCODE_ERROR_INITFAILED
	//
	werror   err = WERRCODE_SUCCESS;

	// NVS initialization
	if (initFlag == false) err = _authKey_init();
	
	if (wErrCode_isSuccess(err)) {
		for (uint8_t t=0; t<AUTHKEY_DEEPLEV; t++) {
			if (nvs_get_u16(nvsHandle, authKey_labelsList[t], &(value[t])) != ESP_OK) {
				// ERROR!
				err = WERRCODE_ERROR_NVSIOFAILED;
				break;
			}
		}
	}
	
	return(err);
}

werror authKey_check (const authKey_t value) {
	//
	// Description:
	//	This function returns WERRCODE_SUCCESS only if the argument defined key's value is almost equal to the stored
	//	one
	//
	// Returned value:
	//	WERRCODE_SUCCESS             The key has been successfully authenticated
	//	WERRCODE_WARNING_RESNOTAV    The filtered value is not yet ready
	//	WERRCODE_WARNING_TESTFAILED  Not authorized key
	//	WERRCODE_ERROR_????          Error code from authKey_read()
	//
	werror    err = WERRCODE_SUCCESS;
	authKey_t filteredValue, storedValue;
	
	err = authKey_read(storedValue);
	//ESP_LOGI(__FUNCTION__, "Reference values = %d:%d", storedValue[0], storedValue[1]);
	
	if (wErrCode_isError(err) == false) {
		//
		// Filtered value getting
		//
		for (uint8_t t=0; t<AUTHKEY_DEEPLEV; t++) {
			*(filteredValue+t) = 0;
			if (ravg_update((ravgPool+t), (filteredValue+t), value[t]) != WERRCODE_SUCCESS) {
				// WARNING!
				ESP_LOGW(__FUNCTION__, "WARNING! filtered values are not yet available");
				err = WERRCODE_WARNING_RESNOTAV;
			}
		}
	
		if (err == WERRCODE_SUCCESS) {
			//
			// Key authentication process
			//
			for (uint8_t t=0; t<AUTHKEY_DEEPLEV; t++) {
				if (abs(storedValue[t] - filteredValue[t]) > AUTHKEY_TOLERANCE) {
					// WARNING!
					err = WERRCODE_WARNING_TESTFAILED;
					break;
				}
			}
			if (err == WERRCODE_SUCCESS) {
				ESP_LOGI(__FUNCTION__, "[ OK ] authentication successfully");
			} else {
				for (uint8_t t=0; t<AUTHKEY_DEEPLEV; t++) {
					ESP_LOGW(
						__FUNCTION__, "key(%d/%d): %d vs. %d",
						t, AUTHKEY_DEEPLEV, storedValue[t], filteredValue[t]
					);
				}
			}
		}
	}
	return(err);
}
