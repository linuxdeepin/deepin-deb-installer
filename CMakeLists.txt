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

#安全测试的开关
set(CMAKE_SAFETYTEST OFF)

project(deepin_deb_installer)
option(DMAN_RELEAE OFF "Install dman resources to system or not")

find_package(Qt5LinguistTools REQUIRED)
set(POLICY_FILE com.deepin.pkexec.aptInstallDepend.policy)
add_subdirectory(src/AptInstallDepend)
add_subdirectory(src/deb-installer)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
endif()
add_subdirectory(src/deepin-deb-installer-dev)
add_subdirectory(translations)
#add_subdirectory(tests/FuzzyTest)

