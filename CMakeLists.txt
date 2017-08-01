cmake_minimum_required(VERSION 2.6)

set(LOLOSWITCHER_INSTALL_PREFIX "/usr/sbin")

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

set(PROJECT loloswitcher)

# set(CMAKE_CXX_FLAGS "-O0")
set_source_files_properties(EventQueue.cpp PROPERTIES COMPILE_FLAGS "-O0")

project(${PROJECT})

set(HEADERS
	Main.h
	KeyLayout.h
	Config.h
	ParseHelper.h
	EventQueue.h
	AccessControl.h)
set(SOURCES 
	Main.cpp
	KeyLayout.cpp
	Config.cpp
	ParseHelper.cpp
	EventQueue.cpp
	AccessControl.cpp)

link_libraries(
	X11
	pcre
	pthread)

add_executable (${PROJECT} ${HEADERS} ${SOURCES})

install(TARGETS loloswitcher DESTINATION ${LOLOSWITCHER_INSTALL_PREFIX})

# install(SCRIPT postinstall.cmake)
install(CODE "EXEC_PROGRAM(chown ARGS root:root ${LOLOSWITCHER_INSTALL_PREFIX}/loloswitcher)") 
install(CODE "EXEC_PROGRAM(chmod ARGS u+s ${LOLOSWITCHER_INSTALL_PREFIX}/loloswitcher)")  
