# Install dependent DLLs

# Qt
get_target_property(Qt6Core Qt6::Core IMPORTED_LOCATION)
get_target_property(Qt6Gui Qt6::Gui IMPORTED_LOCATION)
get_target_property(Qt6Widgets Qt6::Widgets IMPORTED_LOCATION)
get_target_property(Qt6Svg Qt6::Svg IMPORTED_LOCATION)
install(FILES ${Qt6Core} ${Qt6Gui} ${Qt6Widgets} ${Qt6Svg} DESTINATION bin)

# Also the platform and image plugins.
get_filename_component(qt6Base ${Qt6Core} DIRECTORY)
get_filename_component(qt6Base ${qt6Base} DIRECTORY)
install(FILES ${qt6Base}/plugins/platforms/qwindows.dll DESTINATION bin/platforms)
install(FILES ${qt6Base}/plugins/imageformats/qsvg.dll DESTINATION bin/imageformats)

# OpenSSL
string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" version ${OPENSSL_VERSION})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(platformSuffix "-x64")
else()
	set(platformSuffix)
endif()
get_filename_component(opensslBase ${OPENSSL_INCLUDE_DIR} DIRECTORY)
set(libcrypto ${opensslBase}/bin/libcrypto-${version}${platformSuffix}.dll)
install(FILES ${libcrypto} DESTINATION bin)

set(CPACK_GENERATOR WIX)
set(CPACK_WIX_UPGRADE_GUID 5F45FB31-9A30-4501-9E65-6AB9E8FDAF34)
set(CPACK_WIX_PRODUCT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/windows/icon.ico)
set(CPACK_WIX_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/install/windows/FileAssociation.wxs)
include(CPack)
