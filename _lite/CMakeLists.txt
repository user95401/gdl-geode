cmake_minimum_required(VERSION 3.21)
set(CMAKE_TLS_VERIFY 0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(GDL_Lite VERSION 2.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

include_directories("${CMAKE_CURRENT_LIST_DIR}/../api/")

add_library(${PROJECT_NAME} SHARED "_main.cpp")
setup_geode_mod(${PROJECT_NAME})

if (PROJECT_IS_TOP_LEVEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GDLAPI_EXPORTING)
endif()