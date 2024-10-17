/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   prod.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This file contains all software needed by the ESP32 to manage your motorbike's services (eg. start, stop, lights...)
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

// Platform dependent libs
#include "esp_timer.h"
#include "driver/gpio.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"

// Higher level libs
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>

// Progect's sub-modules
#include <mbesPinsMap.h>
#include <iInputInterface.h>
#include <debugConsoleAPI.h>


#define OUTPUTPINS_LIST { \
	o_KEEPALIVE,        \
	o_STARTENGINE,      \
	o_ENGINEON,         \
	o_ENGINEREADY,      \
	o_NEUTRAL,          \
	o_RIGHTARROW,       \
	o_LEFTARROW,        \
	o_DOWNLIGHT,        \
	o_UPLIGHT,          \
	o_ADDLIGHT          \
}

#define BLINK_PERIOD pdMS_TO_TICKS(200)

#define V_TOLERANCE  10

typedef enum {
	RKEY_EVALUATION,
	HW_FAILURE,
	MAIN_LOOP,
	PARCKING_STATUS
}  fsmStates_t;

typedef enum {
	MTB_STOPPED_ST,
	MTB_WFR_ST,
	MTB_ELSTARTING_ST,
	MTB_RUNNIG_ST
} mtbStates_t;

typedef enum {
	BLINKER_TICK,
	BLINKER_GET
} blinkerCmd_t;

//------------------------------------------------------------------------------------------------------------------------------
//                                                 F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
void setPinValue (gpio_num_t pin, uint32_t value) {
	//
	// Description:
	//	It is just a wrapper to gpio_set_level()
	//
	gpio_set_level(pin, value);
	KEEPTRACK_numID(pin, value);
	return;
}

bool blinker (blinkerCmd_t cmd) {
	static volatile bool     status = false;
	static SemaphoreHandle_t mtx;
	bool                     out = true;
	
	xSemaphoreTake(mtx, (10/portTICK_PERIOD_MS));
	if (cmd == BLINKER_TICK)
		status = !status;
	else
		out = status;
	xSemaphoreGive(mtx);
		
	return(out);
}

void tickerCB(TimerHandle_t xTimer) {
	//
	// Software-timer callback
	//
	blinker(BLINKER_TICK);
}

/*
uint16_t normalizz (uint16_t raw) {
	//
	// Description:
	//	It convert the read-from-ADC raw data in millivolts. This function is useful when you are setting the key and
	//	for debug
	//
	return((raw * DEFAULT_VREF) / 8191 * 2.4);
}
*/
//------------------------------------------------------------------------------------------------------------------------------
//                                                      M A I N
//------------------------------------------------------------------------------------------------------------------------------

int main(void) {
	bool          loop         = true;              // It enables the main loop (Just for future applications)
	bool          decompPushed = false;             // Flag true, means motorbike is ready to accept start commands
	fsmStates_t   FSM          = RKEY_EVALUATION;
	mtbStates_t   mtbState     = MTB_STOPPED_ST;
	uint8_t       leftArr_sel, rightArr_sel, uLight_sel, addLight_sel, light_sel;           // Lights
	bool	        leftArr_value, rightArr_value, uLight_value, addLight_value, light_value;
	uint8_t       engStart_sel, decomp_sel, engOn_sel;                                      // Engine controls
	bool          engStart_value, decomp_value, engOn_value;
	uint8_t       neutral_sw, bykestand_sw, clutch_sw;                                      // Motorbyke int switches
	bool          neutral_value, bykestand_value, clutch_value;
	uint8_t       value = 0;
	TimerHandle_t xBlinkTimer;
	
	adc_oneshot_unit_handle_t adcX1_handle, adcX2_handle, adcY1_handle, adcY2_handle;
	int                       adc_rawX1, adc_rawX2, adc_rawY1, adc_rawY2;
	
	
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
			.ulp_mode = ADC_ULP_MODE_DISABLE     // Ultra Low Power FSM coprocessor
		};
	
		if (
			adc_oneshot_new_unit(&init_config1, &adcX1_handle)       != ESP_OK ||
			adc_oneshot_config_channel(adcX1_handle, i_VX1, &config) != ESP_OK ||
			adc_oneshot_new_unit(&init_config1, &adcX2_handle)       != ESP_OK ||
			adc_oneshot_config_channel(adcX2_handle, i_VX2, &config) != ESP_OK ||
			adc_oneshot_new_unit(&init_config1, &adcY1_handle)       != ESP_OK ||
			adc_oneshot_config_channel(adcY1_handle, i_VY1, &config) != ESP_OK ||
			adc_oneshot_new_unit(&init_config1, &adcY2_handle)       != ESP_OK ||
			adc_oneshot_config_channel(adcY2_handle, i_VY1, &config) != ESP_OK
		) {
			// ERROR!
			ESP_LOGE("MAIN", "A/D converter initialization failed");
			FSM =  HW_FAILURE;
		};
	}


	//
	// Output pins configuration
	//
	if (FSM != HW_FAILURE) {
		uint64_t outputBitMasksList[] = OUTPUTPINS_LIST ;
		gpio_config_t outputConfTemplate = {
			.intr_type    = GPIO_INTR_DISABLE,
			.mode         = GPIO_MODE_OUTPUT,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.pull_up_en   = GPIO_PULLDOWN_DISABLE
		};

		for (uint8_t t=0; t<4; t++) {
			outputConfTemplate.pin_bit_mask = (1ULL << outputBitMasksList[t]);
			if (gpio_config(&outputConfTemplate) != ESP_OK) {
				// ERROR!
				ESP_LOGE("MAIN", "ERROR! Output %ld-pin configuration failed", (unsigned long int)outputBitMasksList[t]);
				FSM =  HW_FAILURE;
				break;
			}
		}
	}

	
	//
	// Input pin/controls configuration
	//
	if (
		FSM == HW_FAILURE || iInputInterface_init() != WERRCODE_SUCCESS ||
		
		iInputInterface_new(&engStart_sel, BUTTON, i_STARTBUTTON) != WERRCODE_SUCCESS ||
		iInputInterface_new(&decomp_sel,   BUTTON, i_DECOMPRESS)  != WERRCODE_SUCCESS ||
		iInputInterface_new(&engOn_sel,    SWITCH, i_ENGINEON)    != WERRCODE_SUCCESS ||
	
		iInputInterface_new(&leftArr_sel,  SWITCH, i_LEFTARROW)   != WERRCODE_SUCCESS ||
		iInputInterface_new(&rightArr_sel, SWITCH, i_RIGHTARROW)  != WERRCODE_SUCCESS ||
		iInputInterface_new(&uLight_sel,   SWITCH, i_UPLIGHT)     != WERRCODE_SUCCESS ||
		iInputInterface_new(&addLight_sel, SWITCH, i_ADDLIGHT)    != WERRCODE_SUCCESS ||
		iInputInterface_new(&light_sel,    SWITCH, i_LIGHTONOFF)  != WERRCODE_SUCCESS ||
	
		iInputInterface_new(&neutral_sw,   SWITCH, i_NEUTRAL)     != WERRCODE_SUCCESS ||
		iInputInterface_new(&bykestand_sw, SWITCH, i_BYKESTAND)   != WERRCODE_SUCCESS ||
		iInputInterface_new(&clutch_sw,    SWITCH, i_CLUTCH)      != WERRCODE_SUCCESS 
	) {
		// ERROR!
		ESP_LOGE("MAIN", "ERROR! input pins configuration failed");
		FSM =  HW_FAILURE;
	
	} else {
		//
		// Important output-pins initial values
		//
		setPinValue(o_KEEPALIVE,   0);
		setPinValue(o_STARTENGINE, 0);
		setPinValue(o_ENGINEON,    0);
		setPinValue(o_ENGINEREADY, 0);

		xBlinkTimer = xTimerCreate("BlinkTimer", BLINK_PERIOD, pdTRUE, (void *)0, tickerCB);

		if (xBlinkTimer != NULL)
			xTimerStart(xBlinkTimer, 0);
	}

	while (loop) {
		if (FSM == HW_FAILURE) {
			//
			// Critic hardware failure state
			// [!] In order to go out from this state you can just turn-off and turn-on your motorbike
			//
			value = value ? false : true;
			setPinValue(o_LEFTARROW,  value);
			setPinValue(o_RIGHTARROW, value);
			setPinValue(o_NEUTRAL,   value);
			
			// Are you paranoying??
			setPinValue(o_STARTENGINE, 0);
			setPinValue(o_ENGINEON,    0);
			setPinValue(o_ENGINEREADY, 0);
			
			vTaskDelay(200 / portTICK_PERIOD_MS);
		
		
		} else if (FSM == RKEY_EVALUATION) {
			//
			// Resistor keys evaluation
			//
			if (
				adc_oneshot_read(adcX1_handle, i_VX1, &adc_rawX1) == ESP_OK &&
				adc_oneshot_read(adcX2_handle, i_VX2, &adc_rawX2) == ESP_OK &&
				adc_oneshot_read(adcY1_handle, i_VY1, &adc_rawY1) == ESP_OK &&
				adc_oneshot_read(adcY2_handle, i_VY2, &adc_rawY2) == ESP_OK &&
				abs(adc_rawX1 - adc_rawY1) < V_TOLERANCE                    &&
				abs(adc_rawX2 - adc_rawY2) < V_TOLERANCE
			) {
				// The keyword has been authenicated, you can unplug it
				setPinValue(o_KEEPALIVE, 1);
				FSM = MAIN_LOOP;
				ESP_LOGI("MAIN", "[ OK ] key has been accepted");
			
			} else { 
				setPinValue(o_RIGHTARROW,  0);
				setPinValue(o_LEFTARROW,   0);
				setPinValue(o_DOWNLIGHT,   0);
				setPinValue(o_UPLIGHT,     0);
				setPinValue(o_ADDLIGHT,    0);
			}
			
			// [!] The following delay is used to prevent brutal-force attack (when ready_flag == 0) and to allow
			//    the MCP23008 to boot
			vTaskDelay(100 / portTICK_PERIOD_MS);
		
			
		} else if (FSM == HW_FAILURE) {
			ESP_LOGE("MAIN", "HW failure event detected");
			// TODO: Neutral led blinking
			
			
		} else if (FSM == MAIN_LOOP) {

			// 

			if (
				wErrCode_isError(iInputInterface_get(leftArr_sel,  &leftArr_value))   ||
				wErrCode_isError(iInputInterface_get(rightArr_sel, &rightArr_value))  ||
				wErrCode_isError(iInputInterface_get(uLight_sel,   &uLight_value))    ||
				wErrCode_isError(iInputInterface_get(addLight_sel, &addLight_value))  ||
				wErrCode_isError(iInputInterface_get(light_sel,    &light_value))     ||
				wErrCode_isError(iInputInterface_get(engStart_sel, &engStart_value))  ||
				wErrCode_isError(iInputInterface_get(decomp_sel,   &decomp_value))    ||
				wErrCode_isError(iInputInterface_get(engOn_sel,    &engOn_value))     ||
				wErrCode_isError(iInputInterface_get(neutral_sw,   &neutral_value))   ||
				wErrCode_isError(iInputInterface_get(bykestand_sw, &bykestand_value)) ||
				wErrCode_isError(iInputInterface_get(clutch_sw,    &clutch_value))
			) {
				// === for future enhancements ===
				// ERROR!
				ESP_LOGE("MAIN", "Unexpected error while I was reading the pin status");
				FSM =  HW_FAILURE;
				
			} else {

				//
				// Lights
				//
				if (light_value == true) {
					setPinValue(o_DOWNLIGHT, 1);
					setPinValue(o_UPLIGHT,  uLight_value   ? 1 : 0);
					setPinValue(o_ADDLIGHT, addLight_value ? 1 : 0);
						
				} else {
					setPinValue(o_DOWNLIGHT, 0);
					setPinValue(o_UPLIGHT,   0);
					setPinValue(o_ADDLIGHT,  0);
				}


				//
				// Blinking lights
				//
				if (leftArr_value) {
					setPinValue(o_LEFTARROW, blinker(BLINKER_GET));
					setPinValue(o_RIGHTARROW, 0);
	
				} else if (rightArr_value) {
					// TODO: timer enabling
					setPinValue(o_RIGHTARROW, blinker(BLINKER_GET));
					setPinValue(o_LEFTARROW,  0);
	
				} else {
					// TODO: timer disabling
					setPinValue(o_RIGHTARROW, 0);
					setPinValue(o_LEFTARROW,  0);
				}
				
				
				// Decompressor sensor management
				if (decomp_value) decompPushed = true;
				
				
				//
				// SECURITY policy #1
				// 	Protection by motorcycle stand down while the vehicle is running
				//
				if (neutral_value == false && bykestand_value == false && mtbState != MTB_STOPPED_ST) {
					ESP_LOGW("MAIN", "WARNING! bike stand is down!!");
					mtbState = MTB_STOPPED_ST;
					setPinValue(o_ENGINEON, 0);            // Engine locked by CDI
				}
				
				
				// 
				// Master rule #1
				//	If the engine-on button is disabled then the mtb's engine must be stopped and locked, always
				//
				if (engOn_value == false) {
					mtbState = MTB_STOPPED_ST;
					setPinValue(o_ENGINEON, 0);            // Engine locked by CDI
				}
	
				
				//
				// Master rule #2
				//	If the start button is not pressed the electric eng must be stopped, in any situation
				//
				if (engStart_value == false)  // i_STARTBUTTON 
					setPinValue(o_STARTENGINE, 0);


				switch (mtbState) {
					case MTB_STOPPED_ST: {
						//
						// In this state the mtb is stopped and it CANNOT be started by the electric engine and
						// manually too
						//
						setPinValue(o_ENGINEON,    0);   // Engine locked by CDI
						setPinValue(o_ENGINEREADY, 0);   // LED: mtb is not yet ready to start
						setPinValue(o_STARTENGINE, 0);
						
						ESP_LOGI("MAIN", "MTB_STOPPED_ST");
						if ((neutral_value || clutch_value) && decompPushed && engOn_value)
							mtbState = MTB_WFR_ST;
					} break;
	
	
					case MTB_WFR_ST: {
						//
						// In tis state driver can choise to start the mtb using the electric eng or manually
						//
						ESP_LOGI("MAIN", "MTB_WFRST");
						setPinValue(o_ENGINEON,    1);                 // Engine no more locked by CDI
						setPinValue(o_ENGINEREADY, 1);                 // LED: mtb is ready to start
						
						if (neutral_value == false && clutch_value == false) {
							decompPushed = false;                    // The mtb has been started manually
							mtbState = MTB_RUNNIG_ST;
							setPinValue(o_ENGINEREADY, 0);
							ESP_LOGI("MAIN", "MTB started manually");
						
						} else if (engStart_value) {                  // i_STARTBUTTON
							setPinValue(o_ENGINEREADY, 0);
							ESP_LOGI("MAIN", "OK electric starter is running");
							decompPushed = false;
							mtbState = MTB_ELSTARTING_ST;
						}
					} break;
				
	
					case MTB_ELSTARTING_ST: {
						//
						// The electric start engine is running
						//
						ESP_LOGI("MAIN", "MTB_ELSTARTING_ST");
						setPinValue(o_STARTENGINE, engStart_value);  // i_STARTBUTTON
						if (engStart_value == false) 
							mtbState = MTB_RUNNIG_ST;
					} break;
	
	
					case MTB_RUNNIG_ST: {
						//
						// The motorbike's engine is running....
						// Unfortunately, because the MCU does not know the real eng status (by RPM signal), to
						// come back in the MTB_STOPPED_ST status, the driver MUST set the engine-on switch to off
						//
						// If the ebgine stops to run for so,e reason, ad the driver will push the decompressor
						// control, then the engine will be immediately ready to be started again. Also if the
						// driver want to stop the engine using the decompressor (but he should not do this!)
						//
						setPinValue(o_ENGINEREADY, 0);
						ESP_LOGI("MAIN", "MTB_RUNNIG_ST");
	
						if (decomp_value) {
							mtbState = MTB_STOPPED_ST;
							setPinValue(o_ENGINEON, 0);           // Engine locking request to CDI
							decompPushed = false;
							vTaskDelay(1000 / portTICK_PERIOD_MS); // I wait (1s) for the engine stop
						}
					} break;
				} // === mtbstate switch ===
			}
/*
			// Parcking mode
			if (
				mbesSelector_get(engOn_sel)  == false &&
				mbesSelector_get(light_sel)  == false &&
				mbesSelector_get(uLight_sel) == false
			) 
				FSM = PARCKING_STATUS;
*/
			
			
		} else if (FSM == PARCKING_STATUS) {
			//
			// Parcking status
			//
			ESP_LOGI("MAIN", "Parking mode");
			value = blinker(BLINKER_GET);
			setPinValue(o_LEFTARROW,  value);
			setPinValue(o_RIGHTARROW, value);
			setPinValue(o_DOWNLIGHT,  1);

			// [!] The lonely way to exit by the parcking state, is to turning off the motorbike
		}


		// delay
		#if DEBUG > 0
		vTaskDelay(200 / portTICK_PERIOD_MS);
		#else
		vTaskDelay(10 / portTICK_PERIOD_MS);
		#endif
	}

	return(0);
}
