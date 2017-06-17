cmake_minimum_required(VERSION 3.8)

project(deepin-deb-installer)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS -Wall)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS -O3)
endif ()

file(GLOB SRCS "*.h" "*.cpp")

# Find the library
find_package(Qt5Widgets)

# Tell CMake to create the executable
add_executable(deepin-deb-installer ${SRCS} resources/resources.qrc)

# Use the module from Qt 5.
target_link_libraries(deepin-deb-installer Qt5::Widgets)
