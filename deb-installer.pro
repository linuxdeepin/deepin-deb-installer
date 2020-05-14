#-------------------------------------------------
#
# Project created by QtCreator 2019-07-31T11:03:10
#
#-------------------------------------------------

QT += core gui widgets concurrent dtkwidget dtkgui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deepin-deb-installer
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += libqapt

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    debinstaller.cpp \
    deblistmodel.cpp \
    filechoosewidget.cpp \
    infocontrolbutton.cpp \
    main.cpp \
    multipleinstallpage.cpp \
    packagelistview.cpp \
    packageslistdelegate.cpp \
    packagesmanager.cpp \
    singleinstallpage.cpp \
    uninstallconfirmpage.cpp \
    workerprogress.cpp \
    utils.cpp \
    droundbgframe.cpp \
    installprocessinfoview.cpp \
    choosefilebutton.cpp \
    debinfolabel.cpp

HEADERS += \
    debinstaller.h \
    deblistmodel.h \
    filechoosewidget.h \
    infocontrolbutton.h \
    multipleinstallpage.h \
    packagelistview.h \
    packageslistdelegate.h \
    packagesmanager.h \
    result.h \
    singleinstallpage.h \
    uninstallconfirmpage.h \
    workerprogress.h \
    utils.h \
    droundbgframe.h \
    installprocessinfoview.h \
    choosefilebutton.h \
    debinfolabel.h

DISTFILES += \
    deepin-deb-installer.applications \
    deepin-deb-installer.desktop \
    translations/deepin-deb-installer.ts \
    translations/deepin-deb-installer_am_ET.ts \
    translations/deepin-deb-installer_ar.ts \
    translations/deepin-deb-installer_ast.ts \
    translations/deepin-deb-installer_bg.ts \
    translations/deepin-deb-installer_ca.ts \
    translations/deepin-deb-installer_cs.ts \
    translations/deepin-deb-installer_da.ts \
    translations/deepin-deb-installer_de.ts \
    translations/deepin-deb-installer_en_AU.ts \
    translations/deepin-deb-installer_es.ts \
    translations/deepin-deb-installer_es_419.ts \
    translations/deepin-deb-installer_et.ts \
    translations/deepin-deb-installer_fi.ts \
    translations/deepin-deb-installer_fr.ts \
    translations/deepin-deb-installer_gl_ES.ts \
    translations/deepin-deb-installer_he.ts \
    translations/deepin-deb-installer_hi_IN.ts \
    translations/deepin-deb-installer_hr.ts \
    translations/deepin-deb-installer_hu.ts \
    translations/deepin-deb-installer_id.ts \
    translations/deepin-deb-installer_it.ts \
    translations/deepin-deb-installer_ko.ts \
    translations/deepin-deb-installer_lt.ts \
    translations/deepin-deb-installer_mn.ts \
    translations/deepin-deb-installer_ms.ts \
    translations/deepin-deb-installer_ne.ts \
    translations/deepin-deb-installer_nl.ts \
    translations/deepin-deb-installer_pa.ts \
    translations/deepin-deb-installer_pl.ts \
    translations/deepin-deb-installer_pt.ts \
    translations/deepin-deb-installer_pt_BR.ts \
    translations/deepin-deb-installer_ru.ts \
    translations/deepin-deb-installer_sk.ts \
    translations/deepin-deb-installer_sl.ts \
    translations/deepin-deb-installer_sr.ts \
    translations/deepin-deb-installer_tr.ts \
    translations/deepin-deb-installer_uk.ts \
    translations/deepin-deb-installer_zh_TW.ts \
    translations/desktop/desktop.ts \
    translations/desktop/desktop_ast.ts \
    translations/desktop/desktop_bg.ts \
    translations/desktop/desktop_ca.ts \
    translations/desktop/desktop_cs.ts \
    translations/desktop/desktop_da.ts \
    translations/desktop/desktop_de.ts \
    translations/desktop/desktop_en_AU.ts \
    translations/desktop/desktop_es.ts \
    translations/desktop/desktop_es_419.ts \
    translations/desktop/desktop_et.ts \
    translations/desktop/desktop_fi.ts \
    translations/desktop/desktop_fr.ts \
    translations/desktop/desktop_hu.ts \
    translations/desktop/desktop_id.ts \
    translations/desktop/desktop_it.ts \
    translations/desktop/desktop_ko.ts \
    translations/desktop/desktop_lt.ts \
    translations/desktop/desktop_mn.ts \
    translations/desktop/desktop_ms.ts \
    translations/desktop/desktop_ne.ts \
    translations/desktop/desktop_nl.ts \
    translations/desktop/desktop_pa.ts \
    translations/desktop/desktop_pl.ts \
    translations/desktop/desktop_pt.ts \
    translations/desktop/desktop_pt_BR.ts \
    translations/desktop/desktop_ru.ts \
    translations/desktop/desktop_sk.ts \
    translations/desktop/desktop_sq.ts \
    translations/desktop/desktop_sr.ts \
    translations/desktop/desktop_tr.ts \
    translations/desktop/desktop_uk.ts \
    translations/desktop/desktop_zh_CN.ts \
    translations/desktop/desktop_zh_TW.ts \
    translations/deepin-deb-installer_zh_CN.ts

RESOURCES += \
    resources/resources.qrc

TRANSLATIONS += $$PWD/translations/deepin-deb-installer_zh_CN.ts

# Automating generation .qm files from .ts files
!system($$PWD/translate_generation.sh): error("Failed to generate translation")

isEmpty(BINDIR):BINDIR=/usr/bin
isEmpty(APPDIR):APPDIR=/usr/share/applications
isEmpty(APPREGDIR):APPREGDIR=/usr/share/application-registry
isEmpty(DSRDIR):DSRDIR=/usr/share/deepin-deb-installer

target.path = $$INSTROOT$$BINDIR
desktop.path = $$INSTROOT$$APPDIR
desktop.files = $$PWD/deepin-deb-installer.desktop

applications.path = $$INSTROOT$$APPREGDIR
applications.files = $$PWD/deepin-deb-installer.applications

translations.path = /usr/share/deepin-deb-installer/translations
translations.files = $$PWD/translations/*.qm

icon_files.path = /usr/share/icons/hicolor/scalable/apps
icon_files.files = $$PWD/resources/images/deepin-deb-installer.svg


INSTALLS += target desktop applications translations icon_files

CONFIG(release, debug|release) {
    TRANSLATIONS = $$files($$PWD/translations/*.ts)
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    dtk_translations.path = /usr/share/$$TARGET/translations
    dtk_translations.files = $$PWD/translations/*.qm
    INSTALLS += dtk_translations
}
