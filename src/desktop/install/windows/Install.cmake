# Install dependent DLLs
get_filename_component(sslBase ${OPENSSL_INCLUDE_DIR} DIRECTORY)
list(GET Qt5Widgets_INCLUDE_DIRS 0 qtInclude)
get_filename_component(qtBase ${qtInclude} DIRECTORY)

install(FILES ${qtBase}/bin/Qt5Core.dll DESTINATION bin)
install(FILES ${qtBase}/bin/Qt5Gui.dll DESTINATION bin)
install(FILES ${qtBase}/bin/Qt5Widgets.dll DESTINATION bin)
install(FILES ${qtBase}/bin/Qt5Svg.dll DESTINATION bin)
install(FILES ${sslBase}/bin/libeay32.dll DESTINATION bin)
install(FILES C:/Windows/System32/msvcp120.dll DESTINATION bin)
install(FILES C:/Windows/System32/msvcr120.dll DESTINATION bin)

set(CPACK_GENERATOR WIX)
set(CPACK_WIX_UPGRADE_GUID 5F45FB31-9A30-4501-9E65-6AB9E8FDAF34)
set(CPACK_WIX_PRODUCT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows/icon.ico)
set(CPACK_WIX_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/install/windows/FileAssociation.wxs)
include(CPack)