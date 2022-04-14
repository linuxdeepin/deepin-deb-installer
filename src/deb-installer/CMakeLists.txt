# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#
# Author:     cuizhen <cuizhen@deepin.io>
#
# Maintainer: cuizhen <cuizhen@uniontech.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.7)
if (NOT DEFINED VERSION)
    message(WARNING "Not defined version ,about dialog version set 5.3.9")
    set(VERSION 5.3.9)
endif ()

#common resource names
set(APP_RES_DIR "${CMAKE_SOURCE_DIR}/assets")
set(APP_BIN_NAME "deepin-deb-installer")
set(APP_TRANS_SCRIPT "${APP_RES_DIR}/translate_generation.sh")
set(APP_DESKTOP "${APP_RES_DIR}/deepin-deb-installer.desktop")
set(APP_QRC "${APP_RES_DIR}/resources.qrc")

project(${APP_BIN_NAME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
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

#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

#compile flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")

  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT")

  # 添加宏定义只在debug模式下输出性能打点日志
  add_definitions("-DDEBUG")

  if(CMAKE_SAFETYTEST)
  #安全测试选项
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

#Generate the qm file
execute_process(COMMAND bash ${APP_TRANS_SCRIPT}
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

configure_file(${APP_RES_DIR}/environments.h.in environments.h @ONLY)

add_definitions(-DUSE_POLKIT -DENABLE_INACTIVE_DISPLAY)
# Find the library

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5DBus REQUIRED)

set(LINK_LIBS
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    QApt
    Qt5::Concurrent
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)

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

#Include all app own subdirectorys
foreach(subdir ${all_src})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}${subdir})
endforeach()

include_directories(${CMAKE_CURRENT_LIST_DIR}/utils)

file(GLOB_RECURSE APP_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/manager/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/process/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pages/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/*.cpp

    )

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# Tell CMake to create the executable
add_executable(${PROJECT_NAME} ${APP_SRCS} ${APP_QRC})

target_include_directories(${APP_BIN_NAME}
    PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})

target_link_libraries(${APP_BIN_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::DBus
    QApt
    Qt5::Concurrent
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)
set(CMAKE_INSTALL_PREFIX /usr)

# Install files
install(DIRECTORY ${APP_RES_DIR}/deepin-deb-installer DESTINATION /usr/share/deepin-manual/manual-assets/application/)
install(TARGETS deepin-deb-installer RUNTIME DESTINATION bin)
install(FILES ${APP_CONFIG} DESTINATION share/deepin-deb-installer)
install(FILES ${APP_DESKTOP} DESTINATION share/applications)

