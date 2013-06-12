# This configuration is for the aloe++ skeleton

# set-up the program libraries here
set(LIBRARIES m rt)

# set-up program includes here
include_directories(/usr/local/include/)

############## DO NOT NEED TO MODIFY BEYOND HERE

get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME) 

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
	cmake_minimum_required (VERSION 2.6)
	project (${module})

	# The version number.
	set (OECORE_VERSION_MAJOR 1)
	set (OECORE_VERSION_MINOR 0)
	set(MODULE_REPOS_NAME "default")
	
else()
	include_directories(${OESR_INCLUDE})
endif()


file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE TEST_SOURCES "test/*.c")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# aloe module
add_library(${module}-aloe SHARED ${SOURCES})
set_target_properties(${module}-aloe PROPERTIES OUTPUT_NAME ${module})
set_target_properties(${module}-aloe PROPERTIES COMPILE_FLAGS "-D_COMPILE_ALOE")
target_link_libraries(${module}-aloe oesrapi skeleton ${LIBRARIES})
install(TARGETS ${module}-aloe DESTINATION lib/${MODULE_REPOS_NAME}/)





