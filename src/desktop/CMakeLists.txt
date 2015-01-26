project(NoteVault)
cmake_minimum_required(VERSION 2.8)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
endif()

set(SRC_LIST
	main.cpp
	ui/MainWindow.cpp
	ui/MainWindow.h
	notes/IdFactory.h
	notes/IdFactory.cpp
	notes/Note.h
	notes/NoteSet.cpp
	notes/NoteSet.h
	)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})
