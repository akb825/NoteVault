# Install dependent DLLs

# Qt
find_file(Qt5Core Qt5Core.dll)
find_file(Qt5Gui Qt5Gui.dll)
find_file(Qt5Widgets Qt5Widgets.dll)
find_file(Qt5Svg Qt5Svg.dll)
install(FILES ${Qt5Core} ${Qt5Gui} ${Qt5Widgets} ${Qt5Svg} DESTINATION bin)

# Also the platform.
get_filename_component(qt5Base ${Qt5Core} DIRECTORY)
get_filename_component(qt5Base ${qt5Base} DIRECTORY)
find_file(qwindows qwindows.dll PATHS ${qt5Base}/plugins/platforms)
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

# OpenSSL currently also uses VS2013 redistributable.
# Make it optional in case this breaks in the future.
find_file(msvcr120 msvcr120.dll)
if (msvcr120)
	install(FILES ${msvcr120} DESTINATION bin)
endif()

set(CPACK_GENERATOR WIX)
set(CPACK_WIX_UPGRADE_GUID 5F45FB31-9A30-4501-9E65-6AB9E8FDAF34)
set(CPACK_WIX_PRODUCT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows/icon.ico)
set(CPACK_WIX_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/install/windows/FileAssociation.wxs)
include(CPack)