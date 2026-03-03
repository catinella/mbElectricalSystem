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
#-----------------------------------------------------------------------------------------------------------------------------------
include(${CMAKE_CURRENT_LIST_DIR}/../../configUtils.cmake)

idf_component_register(
	SRCS 
		"debugConsoleAPI.c"
	INCLUDE_DIRS
		"include"
		"../../tools/debugConsole"
	REQUIRES
		esp_driver_gpio
)

# Configuration file reading...
parse_config("${CMAKE_SOURCE_DIR}/CMakeLists.conf" "debugConsoleAPI")

if(DEFINED DBGCON_KEEPTRACK)
	if(${DBGCON_KEEPTRACK} EQUAL 1)
		message(WARNING "[WARNING!] Debug console support has been activated")
		target_compile_definitions(${COMPONENT_LIB} PRIVATE DBGCON_KEEPTRACK)
	endif()
endif()

target_compile_definitions(${COMPONENT_LIB} PRIVATE TARGET_ESP32)
