# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED VERSION)
    set(VERSION 5.3.9)
endif()

# 安全测试的开关
set(CMAKE_SAFETYTEST OFF)

project(deepin_deb_installer)
option(DMAN_RELEAE OFF "Install dman resources to system or not")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
# 引入翻译生成
include(translation-generate)

# Find Qt version
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message("   >>> Found Qt version: ${QT_VERSION_MAJOR}")
set(QT_DESIRED_VERSION ${QT_VERSION_MAJOR})

if (QT_VERSION_MAJOR MATCHES 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()
message("   >>> Build with DTK: ${DTK_VERSION_MAJOR}")

add_subdirectory(src/AptInstallDepend)
add_subdirectory(src/deb-installer)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
endif()

add_subdirectory(src/deepin-deb-installer-dev)

set(POLICY_FILE com.deepin.pkexec.aptInstallDepend.policy)
add_subdirectory(translations/policy)

# add_subdirectory(tests/FuzzyTest)
