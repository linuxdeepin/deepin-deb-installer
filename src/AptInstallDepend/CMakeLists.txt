cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED VERSION)
    set(VERSION 5.3.9)
endif ()

project(AptInstallDepend)
set (EXE_NAME deepin-deb-installer-dependsInstall)
set(LINK_LIBS
    Qt5::Core
)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


if(CMAKE_SAFETYTEST)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O2")
 endif()

file(GLOB_RECURSE AUTH_CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_executable (${EXE_NAME}
    ${AUTH_CPP_FILES}
)

# Find the library
find_package(Qt5Core REQUIRED)
set(CMAKE_INSTALL_PREFIX /usr)
# Install files
install(TARGETS ${EXE_NAME} DESTINATION bin)
target_link_libraries(${EXE_NAME} ${LINK_LIBS} )
