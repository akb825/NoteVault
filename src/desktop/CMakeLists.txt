project(NoteVault)
cmake_minimum_required(VERSION 2.8)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
endif()

set(SRC_LIST
	main.cpp
	io/Crypto.cpp
	io/Crypto.h
	io/CryptoIStream.cpp
	io/CryptoIStream.h
	io/CryptoOStream.cpp
	io/CryptoOStream.h
	io/FileIStream.cpp
	io/FileIStream.h
	io/FileOStream.cpp
	io/FileOStream.h
	io/IStream.h
	io/NoteFile.cpp
	io/NoteFile.h
	io/OStream.h
	notes/IdFactory.h
	notes/IdFactory.cpp
	notes/Note.h
	notes/NoteSet.cpp
	notes/NoteSet.h
	ui/MainWindow.cpp
	ui/MainWindow.h
	)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(OpenSSL REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories(${wxWidgets_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} ${OPENSSL_LIBRARIES})
