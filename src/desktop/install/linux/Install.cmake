configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install/linux/notevault.desktop.in
	${CMAKE_CURRENT_BINARY_DIR}/notevault.desktop @ONLY)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/linux/icons/
	DESTINATION share/icons/hicolor)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/notevault.desktop DESTINATION share/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/install/linux/secure-note.xml
	DESTINATION share/mime/packages)
