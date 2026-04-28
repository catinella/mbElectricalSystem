#-----------------------------------------------------------------------------------------------------------------------------------
#    __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
#   |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
#   | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
#   | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
#   |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
#                                                                                                   |___/
#
# File name: main/CMakeLists.txt
#
# Author: Silvano Catinella <catinella@yahoo.com>
#
# Description:
#	CMAKE building software cofiguration file
#
#	To build the rimware use the framework command "idf.by build" or type cmake <CMakeLists.txt path> in a proper path.
#	
#-----------------------------------------------------------------------------------------------------------------------------------
include(${CMAKE_CURRENT_LIST_DIR}/../configUtils.cmake)

idf_component_register(
	SRCS	
		"firmware-esp32.c" 
	INCLUDE_DIRS
		"." 
		"../components/iInputInterface/include"
		"../components/werror/include"
		"../components/debugConsoleAPI/include"
		"../components/authKey/include"
		"../components/ravgFilter/include"
	PRIV_REQUIRES
		esp_driver_gpio
		esp_adc
		hal
		esp_timer
)

add_custom_target(clink ALL DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/firmware-esp32.c")
add_dependencies(app clink)

# Configuration file reading...
parse_config("${CMAKE_SOURCE_DIR}/CMakeLists.conf" "main")

if(DEFINED FIRMWARETEST)
	message(WARNING "[!] TEST mode has been enabled")
	set(myexec ${FIRMWARETEST})
else()
	message(STATUS "Normal mode")
	set(myexec prod.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${myexec}")
	if(NOT "${myexec}" STREQUAL "prod.c")
		message(STATUS "Testing firmware: ${CMAKE_CURRENT_SOURCE_DIR}/${myexec}")
	endif()

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_SOURCE_DIR}" ./clink.sh ${myexec}
	)
else()
	message(FATAL_ERROR "Firmware file '[ERROR!] ${CMAKE_CURRENT_SOURCE_DIR}/${myexec}' does not exist.")
endif()

target_compile_definitions(${COMPONENT_LIB} PRIVATE TARGET_ESP32)

if(DEFINED DEBUG)
	if(${DEBUG} GREATER 0)
		message(WARNING "[WARNING!] Debug support has been enabled")
		target_compile_definitions(${COMPONENT_LIB} PRIVATE DEBUG=${DEBUG})
	endif()
endif()

if(DEFINED NOAUTH)
	if(${NOAUTH} GREATER 0)
		message(WARNING "[WARNING!] Authentication procedure has been disabled")
		target_compile_definitions(${COMPONENT_LIB} PRIVATE NOAUTH)
		#target_compile_definitions(${COMPONENT_LIB} PRIVATE NOAUTH=${NOAUTH})
	endif()
endif()

if(DEFINED NODLSWITCH)
	if(${NODLSWITCH} GREATER 0)
		message(WARNING "[WARNING!] Firmware for a device without the low beam lights control")
		target_compile_definitions(${COMPONENT_LIB} PRIVATE NODLSWITCH)
	endif()
endif()

