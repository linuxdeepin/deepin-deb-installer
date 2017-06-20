cmake_minimum_required(VERSION 3.7)

project(deepin-deb-installer)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS -Wall)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS -O3)
endif ()

file(GLOB SRCS "*.h" "*.cpp")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(QApt REQUIRED)

pkg_check_modules(DtkBase REQUIRED dtkbase)
pkg_check_modules(DtkWidget REQUIRED dtkwidget)

# Tell CMake to create the executable
add_executable(deepin-deb-installer ${SRCS} resources/resources.qrc)
target_include_directories(deepin-deb-installer PUBLIC ${DtkBase_INCLUDE_DIRS} ${DtkWidget_INCLUDE_DIRS})
target_link_libraries(deepin-deb-installer QApt ${DtkBase_LIBRARIES} ${DtkWidget_LIBRARIES} ${Qt5Widgets_LIBRARIES})

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

install(TARGETS deepin-deb-installer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES deepin-deb-installer.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
