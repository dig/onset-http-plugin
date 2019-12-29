cmake_minimum_required(VERSION 3.1)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(onset-http-plugin VERSION 1.0.0)


list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(UNIX)
	# hide non-exported symbols
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(HORIZONSDK_ROOT_DIR "${PROJECT_SOURCE_DIR}/libs/sdk")
find_package(HorizonPluginSDK REQUIRED)


#option(CURL_STATICLIB "Set to ON to build libcurl with static linking."  ON)
if(WIN32)
    add_definitions("-DCURL_STATICLIB")
endif()
set(CURL_LIBRARY "-lcurl")

if(WIN32)
	find_package(CURL CONFIG REQUIRED)
elseif(UNIX)
	find_package(CURL REQUIRED)
endif()

add_subdirectory(src)
