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

if(DEFINED ENV{DBGCON_KEEPTRACK})
	if($ENV{DBGCON_KEEPTRACK} EQUAL 1)
		message(WARNING "[WARNING!] Debug console support has been activated")
		target_compile_definitions(${COMPONENT_LIB} PRIVATE DBGCON_KEEPTRACK=1)
	endif()
endif()
