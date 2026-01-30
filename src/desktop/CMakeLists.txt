cmake_minimum_required(VERSION 3.16)
project(notevault)

set(CMAKE_CXX_STANDARD 11)

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
	add_compile_options(-Wall -Werror)
elseif (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows)
	set(SRC_LIST ${SRC_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows/icon.rc)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Svg OpenSSL::Crypto)
include_directories(${OPENSSL_INCLUDE_DIR})

set(CPACK_PACKAGE_NAME "Note Vault")
set(CPACK_PACKAGE_VENDOR "Aaron Barany")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Store notes securely without relying on an online service.")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Note Vault")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY NoteVault)
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/install/License.txt)
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};Note Vault")

if (WIN32)
	include(InstallRequiredSystemLibraries)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
if (UNIX)
	include(install/linux/Install.cmake)
elseif (WIN32)
	include(install/windows/Install.cmake)
endif()
