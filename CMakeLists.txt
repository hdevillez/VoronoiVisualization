cmake_minimum_required(VERSION 3.6)
project(Draw_tools C)

set(EXEC_NAME exec)
SET(CMAKE_CXX_FLAGS -lpthread)
add_executable(${EXEC_NAME} src/draw_tools.c src/alt/geometry src/alt/bbst.c src/alt/event.c src/alt/pq.c src/alt/draw.c src/alt/fortune.c src/alt/main.c )

set(CMAKE_C_FLAGS "-g -O0 -std=c99 -Wall")


set_property(TARGET ${EXEC_NAME} PROPERTY C_STANDARD 99)
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)


# add dependencies: glfw, glad, -lm
add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
target_include_directories(${EXEC_NAME} PRIVATE ${GLIB_INCLUDE_DIRS})

#target_link_libraries(${EXEC_NAME} INTERFACE ${GLIB_LDFLAGS} glad glfw m)
# set exec as the startup project in visual studio
if ( MSVC )
	target_link_libraries(${EXEC_NAME} glad glfw)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXEC_NAME})
else()
    target_link_libraries(${EXEC_NAME} glad glfw m ${GLIB_LDFLAGS}) 
endif()

