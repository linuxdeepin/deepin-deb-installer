cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    set(VERSION 1.2.2)
endif ()

project(deepin-deb-installer)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

    # generate qm files
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    execute_process(COMMAND bash "translate_ts2desktop.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else ()
    set(CMAKE_CXX_COMPILER "clang++")
endif ()

configure_file(environments.h.in environments.h @ONLY)

# Sources files
file(GLOB SRCS "*.h" "*.cpp" "widgets/*.h" "widgets/*.cpp")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(QApt REQUIRED)
find_package(DtkWidget REQUIRED)

#find_package(Qt5LinguistTools)
#SET(Lang_files ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/XXX.ts)
#QT5_CREATE_TRANSLATION(LangSrcs ${Lang_files})
#add_executable(${target}  WIN32 ${sources} ${headers} ${LangSrcs} )


# Tell CMake to create the executable
add_executable(deepin-deb-installer ${SRCS} resources/resources.qrc)
target_include_directories(deepin-deb-installer PUBLIC ${DtkWidget_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})
target_link_libraries(deepin-deb-installer
    QApt
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# Install qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES}
        DESTINATION share/deepin-deb-installer/translations)

# Install icons
foreach (ICON_SIZE 16 24 32 48 64 96 128 256)
    install(FILES resources/images/deepin-deb-installer-${ICON_SIZE}px.svg
            RENAME deepin-deb-installer.svg
            DESTINATION share/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps)
endforeach ()

install(TARGETS deepin-deb-installer DESTINATION bin)
install(FILES deepin-deb-installer.desktop DESTINATION share/applications)
install(FILES deepin-deb-installer.applications DESTINATION share/application-registry)


