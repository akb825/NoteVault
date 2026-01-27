# Install dependent DLLs

# Qt
find_file(Qt6Core Qt6Core.dll)
find_file(Qt6Gui Qt6Gui.dll)
find_file(Qt6Widgets Qt6Widgets.dll)
find_file(Qt6Svg Qt6Svg.dll)
install(FILES ${Qt6Core} ${Qt6Gui} ${Qt6Widgets} ${Qt6Svg} DESTINATION bin)

# Also the platform.
get_filename_component(qt6Base ${Qt6Core} DIRECTORY)
get_filename_component(qt6Base ${qt6Base} DIRECTORY)
find_file(qwindows qwindows.dll PATHS ${qt6Base}/plugins/platforms)
install(FILES ${qwindows} DESTINATION bin/platforms)

# OpenSSL
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\..*" "\\1_\\2" version ${OPENSSL_VERSION})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(platformSuffix "-x64")
else()
	set(platformSuffix)
endif()
set(cryptoName libcrypto-${version}${platformSuffix}.dll)
find_file(libcrypto ${cryptoName})
install(FILES ${libcrypto} DESTINATION bin)

set(CPACK_GENERATOR WIX)
set(CPACK_WIX_UPGRADE_GUID 5F45FB31-9A30-4501-9E65-6AB9E8FDAF34)
set(CPACK_WIX_PRODUCT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows/icon.ico)
set(CPACK_WIX_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/install/windows/FileAssociation.wxs)
include(CPack)
