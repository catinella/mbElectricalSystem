#-----------------------------------------------------------------------------------------------------------------------------------
#    __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
#   |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
#   | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
#   | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
#   |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
#                                                                                                   |___/
#
# File name: CMakeLists.txt
#
# Author: Silvano Catinella <catinella@yahoo.com>
#
# Description:
#	CMAKE building software cofiguration file
#
#	To build the rimware use the framework command "idf.by build" or type cmake <CMakeLists.txt path> in a proper path.
#	
#	Environment vars:
#	=================
#	To allow you to configure the build process, the project provides the following useful environment vars:
#		FIRMWARETEST=<source file> It allows you to build a test instead the prod-firmware.
#		DBGCON_KEEPTRACK={0|1}     If it is set to 1, then It enable the support for the debug console tool
#
#
#-----------------------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(firmware-esp32)

set(IDF_TARGET "esp32s2")

if(DEFINED ENV{FIRMWARETEST})
	message(WARNING "[!] TEST mode has been enabled")
	set(myexec $ENV{FIRMWARETEST})
else()
	message(STATUS "Normal mode")
	set(myexec prod.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main/${myexec}")
	message(STATUS "Testing firmware: ${CMAKE_CURRENT_SOURCE_DIR}/main/${myexec}")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_SOURCE_DIR}/main" ./clink.sh ${myexec}
	)
else()
	message(FATAL_ERROR "Firmware file '[ERROR!] ${CMAKE_CURRENT_SOURCE_DIR}/main/${myexec}' does not exist.")
endif()

# Inside tools sub-folder there are software packages for PC
set(EXCLUDE_DIR "${CMAKE_SOURCE_DIR}/tools")
