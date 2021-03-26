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

cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED VERSION)
    set(VERSION 5.3.9)
endif ()
set(APP_BIN_NAME "libdeepin-deb-installer")

project(deepin-deb-installer-dev)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5DBus REQUIRED)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    QApt
    Qt5::Concurrent
)
include_directories(${CMAKE_CURRENT_LIST_DIR})
file(GLOB_RECURSE APP_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/manager/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/installer/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/package/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/status/*.cpp
    )

file(GLOB_RECURSE APP_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/deepin-deb-installer-lib_global.h
    ${CMAKE_CURRENT_LIST_DIR}/DeepinDebInstallerLib.h
    )

add_library( ${APP_BIN_NAME} SHARED ${APP_SRCS})

# Find the library
target_link_libraries(${APP_BIN_NAME}
    Qt5::Core
    Qt5::DBus
    QApt
    Qt5::Concurrent
)

set(CMAKE_INSTALL_PREFIX /usr)
# Install files

SET_TARGET_PROPERTIES(${APP_BIN_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS ${APP_BIN_NAME} LIBRARY DESTINATION lib)

install(FILES ${APP_INCLUDE} DESTINATION  include/deepin-deb-installer)

execute_process(COMMAND "ldconfig")
