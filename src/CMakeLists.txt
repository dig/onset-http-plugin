set(PLUGIN_CONFIG_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/config_headers")

configure_file(
	"version.hpp.in"
	"${PLUGIN_CONFIG_INCLUDE_DIR}/version.hpp"
	@ONLY
)

add_library(onset-http-plugin MODULE
	Plugin.cpp
	Plugin.hpp
	PluginInterface.cpp
	Singleton.hpp
)

target_include_directories(onset-http-plugin PRIVATE
	${PLUGIN_CONFIG_INCLUDE_DIR}
	${HORIZONSDK_INCLUDE_DIR}
	${CURL_INCLUDE_DIRS}
)

set_property(TARGET onset-http-plugin PROPERTY PREFIX "")

set_property(TARGET onset-http-plugin PROPERTY CXX_STANDARD 17)
set_property(TARGET onset-http-plugin PROPERTY CXX_STANDARD_REQUIRED ON)

if(MSVC)
	target_compile_definitions(onset-http-plugin PRIVATE
		_CRT_SECURE_NO_WARNINGS
		NOMINMAX
	)
endif()

# increase warning levels
if (MSVC)
	target_compile_options(onset-http-plugin PRIVATE
		/W4
		/WX
		/permissive-
		/wd4505
	)
elseif(UNIX)
	target_compile_options(onset-http-plugin PRIVATE
		-Wall
		-Wextra
		-pedantic
		-Werror
		-Wno-unused-function
	)
endif()

target_link_libraries(onset-http-plugin ${HORIZONSDK_LIBRARY} ${CURL_LIBRARIES})

if(UNIX)
	if(NOT APPLE)
		target_link_libraries(onset-http-plugin rt)
	endif()
	
	set_target_properties(onset-http-plugin PROPERTIES
		INSTALL_RPATH "$ORIGIN/..:$ORIGIN/")
endif()

install(TARGETS onset-http-plugin DESTINATION "plugins/")

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
if(WIN32)
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_GENERATOR TGZ)
endif()

include(CPack)
