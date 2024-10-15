idf_component_register(
	SRCS 
		"debugConsoleAPI.c"
	INCLUDE_DIRS
		"include"
		"../../tools/debugConsole"
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE TARGET_ESP32)

