idf_component_register(
	SRCS         "iInputInterface.c"
	INCLUDE_DIRS "include"
	PRIV_REQUIRES esp_driver_gpio debugConsoleAPI esp_timer
)
