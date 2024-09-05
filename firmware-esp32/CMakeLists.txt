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
#	use FIRMWARETEST=<source file> cmake <CMakeLists.txt path>
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
	message(WARNING "Firmware file '${CMAKE_CURRENT_SOURCE_DIR}/main/${myexec}' does not exist.")
endif()
