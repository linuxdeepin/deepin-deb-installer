# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)

if(NOT DEFINED VERSION)
    message(WARNING "Not defined version ,about dialog version set 5.3.9")
    set(VERSION 5.3.9)
endif()

if (QT_DESIRED_VERSION MATCHES 6)
    # Qt6支持的qapt已基于3.0.5.2-1-deepin3版本变为libqapt-qt6-dev
    # execute_process(COMMAND dpkg -s libqapt-qt6-dev
    #     COMMAND grep Version
    #     TIMEOUT 5
    #     OUTPUT_VARIABLE QAPT_VERSION_STR)
    # string(REPLACE "Version: " "" QAPT_VERSION_STR ${QAPT_VERSION_STR})
    message(STATUS "enable virtual package enhance")
    add_definitions(-DENABLE_VIRTUAL_PACKAGE_ENHANCE)
else()
    # 检查qapt版本
    execute_process(COMMAND dpkg -s libqapt-dev
        COMMAND grep Version
        TIMEOUT 5
        OUTPUT_VARIABLE QAPT_VERSION_STR)
    string(REPLACE "Version: " "" QAPT_VERSION_STR ${QAPT_VERSION_STR})
    message(STATUS "BUILD QAPT VERSION ${QAPT_VERSION_STR}")

    if(${QAPT_VERSION_STR} VERSION_LESS "3.0.5-1")
        if(${QAPT_VERSION_STR} VERSION_LESS "3.0.4.8-1+deepin2")
            message(STATUS "disable virtual package enhance")
        else()
            message(STATUS "enable virtual package enhance")
            add_definitions(-DENABLE_VIRTUAL_PACKAGE_ENHANCE)
        endif()
    else()
        # 部分包名后可能添加后缀，后续更新的deepin分支libqapt包应包含虚包处理
        if(${QAPT_VERSION_STR} VERSION_GREATER_EQUAL "3.0.5.1-1-deepin1" AND ${QAPT_VERSION_STR} MATCHES ".*deepin.*")
            message(STATUS "enable virtual package enhance")
            add_definitions(-DENABLE_VIRTUAL_PACKAGE_ENHANCE)
            # Qt5环境且QApt版本>=3.0.5.1-1-deepin1，支持setEnvVariable方法
            add_definitions(-DENABLE_QAPT_SETENV)
        else()
            message(STATUS "disable virtual package enhance")
        endif()
    endif()
endif()

# common resource names
set(APP_RES_DIR "${CMAKE_SOURCE_DIR}/assets")
set(APP_BIN_NAME "deepin-deb-installer")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-deb-installer.desktop")
set(APP_MIME "${APP_RES_DIR}/deepin-deb-installer.xml")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")

project(${APP_BIN_NAME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "-Wl,--as-need -fPIE")
set(CMAKE_EXE_LINKER_FLAGS "-pie")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()

# 安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

# compile flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

    # Enable Qt builtin debug mode
    add_definitions("-DQT_MESSAGELOGCONTEXT")

    # 添加宏定义只在debug模式下输出性能打点日志
    add_definitions("-DDEBUG")

    if(CMAKE_SAFETYTEST)
        # 安全测试选项
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined,address -O2")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g -fsanitize=undefined,address -O2")
        set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -g -fsanitize=undefined,address -O2")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -fsanitize=undefined,address -O2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address -O2")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -fsanitize=undefined,address -O2")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -g -fsanitize=undefined,address -O2")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}  -g -fsanitize=undefined,address -O2")
    endif()

else()
    # -Wl, -O2 Enable linker optimizations
    # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
    # -ffunction-sections
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2 -Wl,-O1 -Wl,--gc-sections")
endif()

configure_file(${APP_RES_DIR}/environments.h.in environments.h @ONLY)

add_definitions(-DUSE_POLKIT -DENABLE_INACTIVE_DISPLAY)

# Qt5环境不支持兼容模式（QThreadPool::start不支持lambda）
if (NOT QT_DESIRED_VERSION MATCHES 6)
    add_definitions(-DDISABLE_COMPATIBLE)
    message(STATUS "Qt5 environment detected, disabling compatible mode")
endif()

# Find the library
find_package(PkgConfig REQUIRED)

find_package(PolkitQt${QT_DESIRED_VERSION}-1)
set(qt_required_components Core DBus Gui Widgets Concurrent)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND qt_required_components Core5Compat)
endif()

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS ${qt_required_components})
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core Gui Widget REQUIRED)

set(LINK_LIBS
    Qt${QT_DESIRED_VERSION}::Core
    Qt${QT_DESIRED_VERSION}::DBus
    Qt${QT_DESIRED_VERSION}::Gui
    Qt${QT_DESIRED_VERSION}::Widgets
    Qt${QT_DESIRED_VERSION}::Concurrent
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    PolkitQt${QT_DESIRED_VERSION}-1::Agent
)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND LINK_LIBS Qt${QT_DESIRED_VERSION}::Core5Compat)
endif()

macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")

    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()

    set(${result} ${dirlist})
endmacro()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SUBDIRLIST(all_src ${CMAKE_CURRENT_SOURCE_DIR})

# Include all app own subdirectorys
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}${subdir})
endforeach()

include_directories(${CMAKE_CURRENT_LIST_DIR}/utils)
file(GLOB_RECURSE APP_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

# Tell CMake to create the executable
add_executable(${PROJECT_NAME} ${APP_SRCS} ${APP_QRC})

target_include_directories(${APP_BIN_NAME}
    PUBLIC ${OBJECT_BINARY_DIR})

target_link_libraries(${APP_BIN_NAME}
    ${LINK_LIBS}
    ${QAPT_LIB}
)
set(CMAKE_INSTALL_PREFIX /usr)

# translation files
TRANSLATION_GENERATE(QM_FILES ${CMAKE_SOURCE_DIR}/translations)
add_custom_target(${PROJECT_NAME}_qm_files DEPENDS ${QM_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_qm_files)

# Install translations
install(FILES ${QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)

# Install files
install(DIRECTORY ${APP_RES_DIR}/deepin-deb-installer DESTINATION /usr/share/deepin-manual/manual-assets/application/)
install(TARGETS deepin-deb-installer RUNTIME DESTINATION bin)
install(FILES ${APP_CONFIG} DESTINATION share/deepin-deb-installer)
install(FILES ${APP_DESKTOP} DESTINATION share/applications)
install(FILES ${APP_MIME} DESTINATION share/mime/packages)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/com.deepin.DebInstaller.service DESTINATION share/dbus-1/services/)
