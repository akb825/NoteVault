project(notevault)
cmake_minimum_required(VERSION 2.8)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
endif()

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

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets REQUIRED)
find_package(OpenSSL REQUIRED)

qt5_wrap_ui(UI_HEADERS ui/AboutDialog.ui)
qt5_wrap_ui(UI_HEADERS ui/ConfirmCloseDialog.ui)
qt5_wrap_ui(UI_HEADERS ui/OpenPasswordDialog.ui)
qt5_wrap_ui(UI_HEADERS ui/MainWindow.ui)
qt5_wrap_ui(UI_HEADERS ui/SavePasswordDialog.ui)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${UI_HEADERS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${OPENSSL_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/linux/icons/
	DESTINATION share/icons/hicolor)
install(FILES notevault.desktop DESTINATION share/applications)
install(FILES secure-note.xml DESTINATION share/mime/packages)
