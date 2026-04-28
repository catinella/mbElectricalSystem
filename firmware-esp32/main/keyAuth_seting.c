/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   keyAuth_seting.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This firmware read the two electric points created by the authentication-key and prints the values on screen.
//	If you want to enable that key to turn-on the motorbike, then use those printed numbers as references in the
//	key authentication process. Please, read the ../README.md file for further details
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

// Platform dependent libraries
#include "esp_timer.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"

// Higher level libraries
#include "esp_log.h"
#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include <math.h>

// Project's libraries
#include <werror.h>
#include <ravgFilter.h>
#include <mbesPinsMap.h>

uint16_t errCalcule (ravgData_t min, ravgData_t max) {
	return(abs((max - min)));
};

int app_main(void) {
	ravg_t     X1_obj,     X2_obj;
	ravgData_t X1_val = 0, X2_val = 0;
	ravgData_t X1_max = 0, X1_min = 0;
	ravgData_t X2_max = 0, X2_min = 0;
	int        X1_tmp = 0, X2_tmp = 0;
	werror     X1_err,     X2_err;
	
	adc_oneshot_unit_handle_t adc_handle;
	
	//
	// Running averrage initialization
	//
	ravg_init(&X1_obj);
	ravg_init(&X2_obj);
	
	//
	// A/D converter configuration
	//
	{
		adc_oneshot_chan_cfg_t config = {
			.bitwidth = ADC_BITWIDTH_DEFAULT,
			.atten    = ADC_ATTEN_DB_12
		};
		adc_oneshot_unit_init_cfg_t init_config1 = {
			.unit_id  = ADC_UNIT_1,               // ADC unit selection
			.clk_src  = ADC_DIGI_CLK_SRC_DEFAULT, // Clock's source
			.ulp_mode = ADC_ULP_MODE_DISABLE      // Ultra Low Power FSM coprocessor
		};

		// A/D converter initialization
		if (adc_oneshot_new_unit(&init_config1, &adc_handle) != ESP_OK) {
			ESP_LOGE("MAIN", "A/D converter initialization failed");
	
		// Channel configuration
		} else if (
			adc_oneshot_config_channel(adc_handle, i_VX1, &config) != ESP_OK ||
			adc_oneshot_config_channel(adc_handle, i_VX2, &config) != ESP_OK 
		) {
			ESP_LOGE("MAIN", "A/D channel configuration failed");
		}
	}

	while (1) {

		if (
			adc_oneshot_read(adc_handle, i_VX1, &X1_tmp) != ESP_OK ||
			adc_oneshot_read(adc_handle, i_VX2, &X2_tmp) != ESP_OK 
		) {
			// ERROR!
			ESP_LOGE("MAIN", "ERROR! adc_oneshot_read() failed");
			
		} else {
			X1_err = ravg_update (&X1_obj, &X1_val, X1_tmp);
			X2_err = ravg_update (&X2_obj, &X2_val, X2_tmp);
			
			if (wErrCode_isSuccess(X1_err) && wErrCode_isSuccess(X2_err)) {
				if (X1_max < X1_val)                 X1_max = X1_val;
				if (X1_min > X1_val || X1_min == 0)  X1_min = X1_val;
				if (X2_max < X2_val)                 X2_max = X2_val;
				if (X2_min > X2_val || X2_min == 0)  X2_min = X2_val;

				ESP_LOGI(__FILE__, "-----------------------------------------");
				ESP_LOGI(
					__FILE__, "X1 = %d -- (%d) -- %d \t Err = %d",
					X1_max, X1_val, X1_min, errCalcule(X1_max, X1_min)
				);
				ESP_LOGI(
					__FILE__, "X2 = %d -- (%d) -- %d \t Err = %d",
					X2_max, X2_val, X2_min, errCalcule(X2_max, X2_min)
				);
				
			} else if (wErrCode_isWarning(X1_err) || wErrCode_isWarning(X2_err))
				ESP_LOGW(__FILE__, "The filtered values are not yet avasilable");
				
			else
				ESP_LOGE(__FILE__, "ravg_update() call failed");
		}
		
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}

	return(0);
}
