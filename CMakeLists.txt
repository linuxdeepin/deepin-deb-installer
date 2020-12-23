cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED VERSION)
    set(VERSION 5.3.9)
endif ()

project(deepin_deb_installer)
option(DMAN_RELEAE OFF "Install dman resources to system or not")
add_subdirectory(src/AptInstallDepend)
add_subdirectory(src/deb_installer)
add_subdirectory(test)
#add_subdirectory(FuzzyTest)

