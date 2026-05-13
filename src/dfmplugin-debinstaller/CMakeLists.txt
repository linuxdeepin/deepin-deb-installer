cmake_minimum_required(VERSION 3.13)

project(dfmplugin-debinstaller)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

if(NOT DEFINED DFM_PLUGIN_DIR)
    set(DFM_PLUGIN_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/dde-file-manager/plugins)
endif()
if(NOT DEFINED DFM_PLUGIN_COMMON_EDGE_DIR)
    set(DFM_PLUGIN_COMMON_EDGE_DIR ${DFM_PLUGIN_DIR}/common-edge)
endif()

FILE(GLOB PLUGIN_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)
find_package(dfm${DTK_VERSION_MAJOR}-framework REQUIRED)
find_package(dfm${DTK_VERSION_MAJOR}-base REQUIRED)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    dfm${DTK_VERSION_MAJOR}-framework
    dfm${DTK_VERSION_MAJOR}-base
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${DFM_PLUGIN_COMMON_EDGE_DIR}
)
