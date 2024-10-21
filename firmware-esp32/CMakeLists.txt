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
#	Configuration process:
#	======================
#	To configure the cource code, set the following values in the configuration file (CMakeLists.txt).
#		FIRMWARETEST=<source file> It allows you to build a test instead the prod-firmware.
#		DBGCON_KEEPTRACK={0|1}     If it is set to 1, then It enable the support for the debug console tool
#
#	Configuration data propagation:
#	===============================
#	At the beginning, in order to configure the building process, I used enviroments variables. But it is a very boring
#	solution, and everytime you forgot to execute the init script, you got a firmware that did not function as expected.
#	So, I modified this file to make it able to read a configuration file, directly. It has been a very useful enhancement,
#	but soon I discovered the variable setting propagation is full of problems!!!! For this reason I created the configUtils
#	library so every components can load the configuration data directly.
#
#-----------------------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)    # Include the IDF framework
include(${CMAKE_SOURCE_DIR}/configUtils.cmake)       # Including the custom library file

project(firmware-esp32)

# Inside tools sub-folder there are software packages for PC
set(EXCLUDE_DIR "${CMAKE_SOURCE_DIR}/tools")
set(IDF_TARGET "esp32s2")

add_custom_target(check_config ALL DEPENDS "${CMAKE_SOURCE_DIR}/CMakeLists.conf")
add_dependencies(app check_config)

# Configuration file reading...
parse_config("${CMAKE_SOURCE_DIR}/CMakeLists.conf")

if(DEFINED FIRMWARETEST)
	message(WARNING "[!] TEST mode has been enabled")
	set(myexec ${FIRMWARETEST})
else()
	message(STATUS "Normal mode")
	set(myexec prod.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main/${myexec}")
	if(NOT "${myexec}" STREQUAL "prod.c")
		message(STATUS "Testing firmware: ${CMAKE_CURRENT_SOURCE_DIR}/main/${myexec}")
	endif()

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_SOURCE_DIR}/main" ./clink.sh ${myexec}
	)
else()
	message(FATAL_ERROR "Firmware file '[ERROR!] ${CMAKE_CURRENT_SOURCE_DIR}/main/${myexec}' does not exist.")
endif()

