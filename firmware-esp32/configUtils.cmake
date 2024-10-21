#-----------------------------------------------------------------------------------------------------------------------------------
#    __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
#   |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
#   | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
#   | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
#   |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
#                                                                                                   |___/
#
# File name: configUtils.cmake
#
# Author: Silvano Catinella <catinella@yahoo.com>
#
# Description:
#	C-Make library for the configuration file parsing. The files want to use this library have to incude the following
#	statements:
#
#	include(${CMAKE_SOURCE_DIR}/config_utils.cmake)      # Including the library file
#	parse_config("${CMAKE_SOURCE_DIR}/CMakeLists.conf")  # It calls the parser and creates the variables
#
#-----------------------------------------------------------------------------------------------------------------------------------

function(parse_config CONFIG_FILE)
	message(STATUS "\n")
	message(STATUS "------------------------------------------------------------------------------")
	message(STATUS "             B U I L D I N G   C O N F I G U R A T I O N "                     )
	message(STATUS "------------------------------------------------------------------------------")
	file(READ "${CONFIG_FILE}" filecontent)
	string(REGEX MATCHALL "^[ \t]*([A-Za-z][^=]+)=([^\n]+)" rowsarray "${filecontent}")
	foreach(row ${rowsarray})
		string(REGEX REPLACE "\n" "" match  "${row}")
		string(REGEX REPLACE "^[ \t]*([^=]+)[\t ]*=[\t ]*(.+)$" "\\1" varname  "${row}")
		string(REGEX REPLACE "^[ \t]*([^=]+)[\t ]*=[\t ]*(.+)$" "\\2" varvalue "${row}")
		
		# No blank spaces
		string(STRIP "${varname}" varname)
		string(STRIP "${varvalue}" varvalue)
		
		# Global vars creation....
		set(${varname} "${varvalue}" PARENT_SCOPE)
		
		message(STATUS "Configuration data: ${varname}=${varvalue}")
	endforeach()
	message(STATUS "\n")
endfunction()


