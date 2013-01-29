
set(LIBRARIES m)

file(GLOB_RECURSE SOURCES "*.c")
file(GLOB_RECURSE INCLUDE_FILES "*.h")

get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME) 

set(FULLNAME ${module})

# aloe module
add_library(${FULLNAME} STATIC ${SOURCES})
target_link_libraries(${FULLNAME} ${LIBRARIES})
install(TARGETS ${FULLNAME} DESTINATION lib)
install(FILES ${INCLUDE_FILES} DESTINATION include/${module})












