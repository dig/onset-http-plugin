cmake_minimum_required(VERSION 3.1)
project(onset-http-plugin VERSION 1.0)


list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(UNIX)
	# hide non-exported symbols
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(HORIZONSDK_ROOT_DIR "${PROJECT_SOURCE_DIR}/libs/sdk")
find_package(HorizonPluginSDK REQUIRED)

set(JSON_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/libs/json/src PARENT_SCOPE)

if(WIN32)
    option(CMAKE_USE_OPENSSL "Use OpenSSL code. Experimental" OFF)
endif()

set(BUILD_CPR_TESTS OFF CACHE BOOL "Don't build cpr tests")
find_package(CURL)
if(CURL_FOUND)
    set(USE_SYSTEM_CURL ON CACHE BOOL "Use the system curl for faster builds")
endif()
add_subdirectory("${PROJECT_SOURCE_DIR}/libs/cpr")

add_subdirectory(src)
