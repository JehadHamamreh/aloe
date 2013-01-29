if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree build. This is bad practice.")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
	cmake_minimum_required (VERSION 2.6)
	project (gen_libs)
	
else()
	include_directories(${OESR_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(LIBRARY_NAME "gen_libs")

file(GLOB modules *) 

foreach (_module ${modules})
	if (IS_DIRECTORY ${_module})
		if (NOT ${_module} STREQUAL ${CMAKE_BINARY_DIR})
			add_subdirectory(${_module})
		endif()
	endif()
endforeach()

