install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/linux/icons/
	DESTINATION share/icons/hicolor)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/install/Linux/notevault.desktop
	DESTINATION share/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/install/Linux/secure-note.xml
	DESTINATION share/mime/packages)