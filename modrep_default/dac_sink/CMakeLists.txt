# This configuration is for the aloe++ skeleton

# set-up the program libraries here
set(LIBRARIES m gen_libs_base rt)

# set-up program includes here
include_directories(/usr/local/include/ ${CMAKE_CURRENT_SOURCE_DIR}/../gen_libs)

############## DO NOT NEED TO MODIFY BEYOND HERE

get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME) 

include_directories(${OESR_INCLUDE})
include_directories(${CMAKE_SOURCE_DIR}/rtdal_lnx/include)


file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE TEST_SOURCES "test/*.c")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# aloe module
add_library(${module}-aloe SHARED ${SOURCES})
set_target_properties(${module}-aloe PROPERTIES OUTPUT_NAME ${module})
set_target_properties(${module}-aloe PROPERTIES COMPILE_FLAGS "-D_COMPILE_ALOE")
target_link_libraries(${module}-aloe oesrapi skeleton ${LIBRARIES})
install(TARGETS ${module}-aloe DESTINATION lib/${MODULE_REPOS_NAME}/)












