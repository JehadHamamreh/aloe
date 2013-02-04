if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree build. This is bad practice.")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

set(MODULE_REPOS_NAME "modrep_default")

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
	cmake_minimum_required (VERSION 2.6)
	project (modrep_default)
	
else()
	include_directories(${OESR_INCLUDE})
endif()

# The version number.
set (MODREP_DEFAULT_VERSION_MAJOR 1)
set (MODREP_DEFAULT_VERSION_MINOR 0)

set(CMAKE_BINARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${MODULE_REPOS_NAME})

set(CMAKE_C_FLAGS "-Wno-format -std=gnu99 -D_GNU_SOURCE -fPIC -rdynamic")
set(CMAKE_SHARED_LINKER_FLAGS "-u _run_cycle")

file(GLOB modules *) 
set(GENLIBS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen_libs)

add_subdirectory(gen_libs)
foreach (_module ${modules})
	if (IS_DIRECTORY ${_module})
		if (NOT ${_module} STREQUAL ${CMAKE_BINARY_DIR})
			if (NOT ${_module} STREQUAL ${GENLIBS_INCLUDE_DIR})	
				add_subdirectory(${_module})
			endif()
		endif()
	endif()
endforeach()

