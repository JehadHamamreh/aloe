
# set-up the program libraries here
set(LIBRARIES m )

# set-up program includes here
include_directories(/usr/local/include/)
include_directories(${CMAKE_SOURCE_DIR}/volk/include)
include_directories(${CMAKE_BINARY_DIR}/volk/include)

############## DO NOT NEED TO MODIFY BEYOND HERE

file(GLOB_RECURSE SOURCES "*.c")
file(GLOB_RECURSE INCLUDE_FILES "*.h")

get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME) 

set(FULLNAME ${LIBRARY_NAME}_${module})

# aloe module
add_library(${FULLNAME} STATIC ${SOURCES})
target_link_libraries(${FULLNAME} ${LIBRARIES})
install(TARGETS ${FULLNAME} DESTINATION lib)
install(FILES ${INCLUDE_FILES} DESTINATION include/${module})












