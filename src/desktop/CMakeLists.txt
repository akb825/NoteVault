project(notevault)
cmake_minimum_required(VERSION 2.8)

set(SRC_LIST
	main.cpp
	Version.h
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
	ui/AboutDialog.cpp
	ui/AboutDialog.h
	ui/AboutDialog.ui
	ui/ConfirmCloseDialog.cpp
	ui/ConfirmCloseDialog.h
	ui/ConfirmCloseDialog.ui
	ui/GeneratePasswordDialog.cpp
	ui/GeneratePasswordDialog.h
	ui/GeneratePasswordDialog.ui
	ui/MainWindow.cpp
	ui/MainWindow.h
	ui/MainWindow.ui
	ui/OpenPasswordDialog.cpp
	ui/OpenPasswordDialog.h
	ui/OpenPasswordDialog.ui
	ui/Resources.qrc
	ui/SavePasswordDialog.cpp
	ui/SavePasswordDialog.h
	ui/SavePasswordDialog.ui
	)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
elseif (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows)
	set(SRC_LIST ${SRC_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows/icon.rc)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets REQUIRED)
find_package(OpenSSL REQUIRED)

qt5_wrap_ui(UI_HEADERS ui/AboutDialog.ui)
qt5_wrap_ui(UI_HEADERS ui/ConfirmCloseDialog.ui)
qt5_wrap_ui(UI_HEADERS ui/GeneratePasswordDialog.ui)
qt5_wrap_ui(UI_HEADERS ui/OpenPasswordDialog.ui)
qt5_wrap_ui(UI_HEADERS ui/MainWindow.ui)
qt5_wrap_ui(UI_HEADERS ui/SavePasswordDialog.ui)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${UI_HEADERS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${OPENSSL_LIBRARIES})
include_directories(${OPENSSL_INCLUDE_DIR})

set(CPACK_PACKAGE_NAME "Note Vault")
set(CPACK_PACKAGE_VENDOR "Aaron Barany")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Store notes securely without relying on an online service.")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Note Vault")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY NoteVault)
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../license.txt)
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};Note Vault")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
if (UNIX)
	include(install/linux/Install.cmake)
elseif (WIN32)
	include(install/windows/Install.cmake)
endif()