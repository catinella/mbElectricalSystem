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
//		| i_VY1         |  internal link   | first voltage reference                             |
//		| i_VY2         |                  | second  "         "                                 |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_NEUTRAL     | from gearbox     | it is 0 when the gear is in neutral position        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_DECOMPRESS  | from decompress. | it is 0 when decompressor has been pushes           |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_KEEPALIVE   | internal only    | it keeps the system on when you unplug the key      |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LEFTARROW   |                  | left turn blinking indicator switch                 |
//		| i_DOWNLIGHT   |                  | low beam command                                    |
//		| i_UPLIGHT     | left controls    | dazzling beam command                               |
//		| i_RIGHTARROW  |                  | right turn blinking indicator switch                |
//		| i_HORN        |                  | horn button                                         |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_BIKESTAND   | from bikestand   | it is 0 when the bike is placed on the stand        |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_STARTBUTTON |                  | the tipical motorbike engine start button           |
//		| i_BRAKESWITCH | right controls   | the switch on the front brake command               |
//		| i_ENGINEON    |                  | the on/off engine switch in on-position             |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_ADDLIGHT    | from ext. switch | additional high power light                         |
//		+---------------+------------------+-----------------------------------------------------+
//		| i_LIGHTONOFF  | from ext. switch | it turns on the normal lights (low/dazzling beam)   |
//		+---------------+------------------+-----------------------------------------------------+
//		| o_STARTENGINE | rear mtb. side   | it activates the (NO) engine-relay                  |
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
#define i_VX1          "A0"
#define i_VX2          "A1"
#define i_VY1          "A2"
#define i_VY2          "A3"
// ==== available ==== "A4"
// ==== available ==== "A5"
#define o_ENGINEOFF    "A6"
#define i_NEUTRAL      "A7"

#define i_LEFTARROW    "B0"
#define i_DOWNLIGHT    "B1"
#define i_UPLIGHT      "B2"
#define i_RIGHTARROW   "B3"
#define i_HORN         "B4"
#define i_BIKESTAND    "B5"
#define o_ENGINEREADY  "B6"
#define o_NEUTRAL      "B7"

// ==== available ==== "C0"
#define i_STARTBUTTON  "C1"
#define i_ENGINEON     "C2"
#define i_DECOMPRESS   "C3"
#define o_RIGHTARROW   "C4"
#define o_LEFTARROW    "C5"
#define i_ADDLIGHT     "C6"
#define i_LIGHTONOFF   "C7"

// === RXD UART ===    "D0"
// === TXD UART ===    "D1"
#define o_DOWNLIGHT    "D2"
#define o_UPLIGHT      "D3"
#define o_ADDLIGHT     "D4"
#define o_HORN         "D5"
#define o_KEEPALIVE    "D6"
#define o_STARTENGINE  "D7"

#endif