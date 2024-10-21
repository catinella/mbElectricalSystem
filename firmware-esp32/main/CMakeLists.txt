include(${CMAKE_CURRENT_LIST_DIR}/../configUtils.cmake)

idf_component_register(
	SRCS	
		"firmware-esp32.c" 
	INCLUDE_DIRS
		"." 
		"../components/iInputInterface/include"
		"../components/werror/include"
		"../components/debugConsoleAPI/include"
	PRIV_REQUIRES
		esp_driver_gpio
		esp_adc
		hal
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
