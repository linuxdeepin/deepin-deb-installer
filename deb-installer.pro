#-------------------------------------------------
#
# Project created by QtCreator 2019-08-16T15:50:02
#
#-------------------------------------------------

QT       += core gui widgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deepin-deb-installer
TEMPLATE = app

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget libqapt

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
    deblistmodel.cpp \
    debinstaller.cpp \
    workerprogress.cpp \
    packagelistview.cpp \
    packageslistdelegate.cpp \
    uninstallconfirmpage.cpp \
    packagesmanager.cpp \
    multipleinstallpage.cpp \
    filechoosewidget.cpp \
    singleinstallpage.cpp \
    infocontrolbutton.cpp \    
    widgets/graybutton.cpp \
    widgets/bluebutton.cpp

HEADERS += \        
    packageslistdelegate.h \
    utils.h \
    infocontrolbutton.h \
    singleinstallpage.h \
    packagesmanager.h \
    result.h \
    workerprogress.h \
    uninstallconfirmpage.h \
    environments.h \
    environments.h.in \
    multipleinstallpage.h \
    packagelistview.h \
    debinstaller.h \
    filechoosewidget.h \
    deblistmodel.h \
    widgets/graybutton.h \
    widgets/bluebutton.h

FORMS += \
#        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    translate_generation.sh \
    translate_ts2desktop.sh

RESOURCES += \
    resources/resources.qrc
