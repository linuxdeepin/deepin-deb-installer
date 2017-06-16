cmake_minimum_required(VERSION 3.8)

project(deepin-deb-installer)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS -Wall)

# source files
set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/debinstaller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filechoosewidget.cpp
   )

# header files
set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/debinstaller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/filechoosewidget.h
   )

# Find the library
find_package(Qt5Widgets)

# Tell CMake to create the executable
add_executable(deepin-deb-installer ${SOURCE} ${HEADERS})

# Use the module from Qt 5.
target_link_libraries(deepin-deb-installer Qt5::Widgets)
