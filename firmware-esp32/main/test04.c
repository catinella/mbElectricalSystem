/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   test04.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	
//
//	This software has been developed for ESP-IDF v5.4 and ESP32-S2-DevKitM-1
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

#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"

#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>

#include <mbesPinsMap.h>

#define DEFAULT_VREF 1100           // Tensione di riferimento predefinita (in mV)

void app_main(void) {
	adc_oneshot_chan_cfg_t      config;
	adc_oneshot_unit_handle_t   adc1_handle;
	adc_oneshot_unit_init_cfg_t init_config1;
	int                         adc_raw, mv;
	
	//
	// ADC1 Initialization
	//
	init_config1.unit_id  = ADC_UNIT_1;               // ADC unit selection
	init_config1.clk_src  = ADC_DIGI_CLK_SRC_DEFAULT; // Clock's source
	init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;     // Ultra Low Power FSM coprocessor
	
	//
	// ADC1 Configuration
	//	[!] Due to a possible BUG you cannot change the bitwidth field value. On the ESP-SP2 the standard is
	//	    ADC_BITWIDTH_13. With any other value MCU returns a runtime error
	//
	config.bitwidth = ADC_BITWIDTH_DEFAULT;
	config.atten    = ADC_ATTEN_DB_12;
	
	
	if (adc_oneshot_new_unit(&init_config1, &adc1_handle) != ESP_OK)
		// ERROR!
		ESP_LOGE(__FUNCTION__, "ERROR! adc_oneshot_new_unit() failed");
	
	else if (adc_oneshot_config_channel(adc1_handle, i_VX1, &config) != ESP_OK)
		// ERROR!
		ESP_LOGE(__FUNCTION__, "ERROR! adc_oneshot_config_channel() failed");
	
	else {

		while (1) {
			if (adc_oneshot_read(adc1_handle, i_VX1, &adc_raw) == ESP_OK) {
				mv = (adc_raw * DEFAULT_VREF) / 8191 * 2.4;
				printf("Valore grezzo ADC: %d\tTensione: %d mV\n", adc_raw, mv);
			} else
				// ERROR!
				ESP_LOGE(__FUNCTION__, "ERROR! adc_oneshot_read() failed");
		
			vTaskDelay(pdMS_TO_TICKS(1000));  // Pausa di 1 secondo
		}
	}
	adc_oneshot_del_unit(adc1_handle);
 }
