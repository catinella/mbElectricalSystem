include(${CMAKE_CURRENT_LIST_DIR}/../../configUtils.cmake)

idf_component_register(
	SRCS
		"iInputInterface.c"
		"moduleDB.c"
	INCLUDE_DIRS
		"include"
		"../werror/include"
	REQUIRES
		esp_driver_gpio
		debugConsoleAPI
		esp_timer
)

# Configuration file reading...
parse_config("${CMAKE_SOURCE_DIR}/CMakeLists.conf")

if(DEFINED DBGCON_KEEPTRACK)
	if(${DBGCON_KEEPTRACK} EQUAL 1)
		message(WARNING "[WARNING!] Debug console support has been activated")
		target_compile_definitions(${COMPONENT_LIB} PRIVATE DBGCON_KEEPTRACK)
	endif()
endif()
