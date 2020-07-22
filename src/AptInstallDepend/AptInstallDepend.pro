QT += core
QT -= gui

CONFIG += c++11

TARGET = deepin-deb-installer-dependsInstall
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    installDebThread.cpp
DEFINES += QT_DEPRECATED_WARNINGS

target.path = /usr/bin

INSTALLS += target

DISTFILES +=

HEADERS += \
    installDebThread.h
