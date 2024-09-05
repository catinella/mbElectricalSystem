/*------------------------------------------------------------------------------------------------------------------------------
//
//  __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesPinsMap.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
//
//	Symbol description:
//		+---------------+------------------+-----------------------------------------------------+
//		|     Symbol    |    Plug/link     |      Description                                    |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_VX1         | resistors-key    | first voltage value                                 |
//		| i_VX2         |                  | second  "       "                                   |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_VREF1       |  internal link   | first voltage reference                             |
//		| i_VREF2       |                  | second  "         "                                 |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_NEUTRAL     | from gearbox     | it is 0 when the gear is in neutral position        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_DECOMPRESS  | from decompress. | it is 0 when decompressor has been pushes           |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_KEEPALIVE   | internal only    | it keeps the system on when you unplug the key      |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_BIKESTAND   | from bikestand   | it is 0 when the bike is placed on the stand        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_ADDLIGHT    | from ext. switch | additional high power light                         |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LIGHTONOFF  | from ext. switch | it turns on the normal lights (low/dazzling beam)   |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_STARTENGINE | rear mtb. side   | it activates the (NO) engine-relay                  |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LEFTARROW   |                  | left turn blinking indicator switch                 |
//		| i_RIGHTARROW  |                  | right turn blinking indicator switch                |
//		| i_DOWNLIGHT   | left controls    |                                                     |
//		| i_UPLIGHT     |                  | dazzling beam command                               |
//		| i_HORN        |                  | horn button                                         |
//		| i_CLUTCH      |                  | 0 means the engine is NOT connected to the wheels   |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_STARTBUTTON |                  | the tipical motorbike engine start button           |
//		| i_BRAKESWITCH | right controls   | the switch on the front brake command               |
//		| i_ENGINEON    |                  | the on/off engine switch in on-position             |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_ENGINEREADY |                  |                                                     |
//		| o_NEUTRAL     |                  |                                                     |
//		| o_RIGHTARROW  |   cockpit LED    |                                                     |
//		| o_LEFTARROW   | indicators and   |                                                     |
//		| o_DOWNLIGHT   |    services      |                                                     |
//		| o_UPLIGHT     |                  |                                                     |
//		| o_ADDLIGHT    |                  |                                                     |
//		| o_HORN        |                  |                                                     |
//		+---------------+------------------+-----------------------------------------------------+
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
#ifndef MBESPINSMAP
#define MBESPINSMAP

//
// PINs declaration
//

// N/A                   (3.3v)                /* 01 */
#define i_LIGHTONOFF     GPIO0                 /* 02 */
#define i_VX1            ADC1_CH0              /* 03 */
#define i_VX2            ADC1_CH1              /* 04 */
#define i_VY1            ADC1_CH2              /* 05 */
#define i_VY2            ADC1_CH3              /* 06 */
// ==== available ====   GPIO5/ADC1_CH4        /* 07 */
// ==== available ====   GPIO6/ADC1_CH5        /* 08 */
// ==== available ====   GPIO7/ADC1_CH6        /* 09 */
// ==== available ====   GPIO8/ADC1_CH7        /* 10 */
#define i_CONF1          GPIO9                 /* 11 */
#define i_CONF2          GPIO_NUM_10           /* 12 */
#define i_NEUTRAL        GPIO_NUM_11           /* 13 */
#define o_KEEPALIVE      GPIO_NUM_12           /* 14 */
#define o_ENGINEREADY    GPIO_NUM_13           /* 15 */
#define o_NEUTRAL        GPIO_NUM_14           /* 16 */
// ==== available ====   GPIO_NUM_15/ADC2_CH4  /* 17 */
#define o_ENGINEON       GPIO_NUM_16           /* 18 */
#define i_HORN           GPIO_NUM_17           /* 19 */
// N/A                   (5v)                  /* 20 */
// N/A                   (GND)                 /* 21 */
#define o_LEFTARROW      GPIO_NUM_18           /* 22 */
// !!!!                  USB/GPIO_NUM_19       /* 23 */
// !!!!                  USB/GPIO_NUM_20       /* 24 */
#define i_ADDLIGHT       GPIO_NUM_21           /* 25 */
#define o_HORN           GPIO_NUM_26           /* 26 */
#define i_RIGHTARROW     GPIO_NUM_33           /* 27 */
#define o_ADDLIGHT       GPIO_NUM_34           /* 28 */
#define i_DOWNLIGHT      GPIO_NUM_35           /* 29 */
#define i_UPLIGHT        GPIO_NUM_36           /* 30 */
#define o_UPLIGHT        GPIO_NUM_37           /* 31 */
#define o_DOWNLIGHT      GPIO_NUM_38           /* 32 */
#define o_RIGHTARROW     GPIO_NUM_39           /* 33 */
#define o_STARTENGINE    GPIO_NUM_40           /* 34 */
#define i_CLUTCH         GPIO_NUM_41           /* 35 */
#define i_BYKESTAND      GPIO_NUM_42           /* 36 */
#define i_DECOMPRESS     GPIO_NUM_43           /* 37 */
#define i_ENGINEON       GPIO_NUM_44           /* 38 */
#define i_STARTBUTTON    GPIO_NUM_45           /* 39 */
#define i_LEFTARROW      GPIO_NUM_46           /* 40 */
// N/A                   (reset)               /* 41 */

#endif
