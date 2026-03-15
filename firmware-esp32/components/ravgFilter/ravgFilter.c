/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   ravgFilter.c
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
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

#include <ravgFilter.h>
#include <stddef.h>

werror ravg_init (ravg_t *item) {
	//
	// Description:
	//	It initializes the argument defined structure
	//
	werror err = WERRCODE_SUCCESS;
	if (item == NULL)
		// ERROR!
		err = WERRCODE_ERROR_ILLEGALARG;
	else {
		item->bufferIndx = 0;
		item->ready = false;
	}
	
	return(err);
}

werror ravg_update (ravg_t *item, ravgData_t *filteredValue, ravgData_t newValue) {
	//
	// Description:
	//	This function adds the argument defined new value to its internal ring buffer and set the argument defined filtered
	//	value with the averrage one. Before to fill up the structure's internal ring buffer, the function cannot calculate 
	//	the averrage value, and it returns a warning message to inform you filtered value is not yet available.
	//
	// Returned value:
	//	WERROR_SUCCESS              Success
	//	WERRCODE_ERROR_ILLEGALARG   NULL pointers are not allowed
	//	WERRCODE_WARNING_RESNOTAV   The averrage value cannot yet be calculated
	//
	werror err = WERRCODE_SUCCESS;

	if (item == NULL || filteredValue == NULL)
		// ERROR!
		err = WERRCODE_ERROR_ILLEGALARG;
	else {
		if (item->bufferIndx == RAVG_DEEPLEVEL) {
			// Filered data is available
			if (item->ready == false) item->ready = true;
			
			item->bufferIndx = 0;
		}
		
		// New data storing...
		item->buffer[item->bufferIndx] = newValue;
		item->bufferIndx++;
		
		if (item->ready) {
			*filteredValue = 0;
			for (uint8_t t=0; t<RAVG_DEEPLEVEL; t++)
				*filteredValue += item->buffer[t];
			*filteredValue = *filteredValue / RAVG_DEEPLEVEL;
		
		} else
			// WARNING!
			err = WERRCODE_WARNING_RESNOTAV;
	}

	return(err);
}
