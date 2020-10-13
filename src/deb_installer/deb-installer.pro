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
    model/packageslistdelegate.cpp \
    model/deblistmodel.cpp \
    model/packagelistview.cpp \
    view/pages/debinstaller.cpp \
    view/pages/multipleinstallpage.cpp \
    view/pages/singleinstallpage.cpp \
    view/pages/uninstallconfirmpage.cpp \
    view/pages/AptConfigMessage.cpp \
    view/widgets/choosefilebutton.cpp \
    view/widgets/coloredprogressbar.cpp \
    view/widgets/debinfolabel.cpp \
    view/widgets/droundbgframe.cpp \
    view/widgets/filechoosewidget.cpp \
    view/widgets/InfoCommandLinkButton.cpp \
    view/widgets/infocontrolbutton.cpp \
    view/widgets/installprocessinfoview.cpp \
    view/widgets/TitleBarFocusMonitor.cpp \
    view/widgets/workerprogress.cpp \
    utils/utils.cpp \
    manager/DealDependThread.cpp \
    manager/PackageDependsStatus.cpp \
    manager/packagesmanager.cpp \
    main.cpp \
    singleInstallerApplication.cpp

HEADERS += \
    manager/packagesmanager.h \
    manager/DealDependThread.h \
    manager/PackageDependsStatus.h \
    model/packageslistdelegate.h \
    model/deblistmodel.h \
    model/packagelistview.h \
    view/pages/debinstaller.h \
    view/pages/multipleinstallpage.h \
    view/pages/singleinstallpage.h \
    view/pages/uninstallconfirmpage.h \
    view/pages/AptConfigMessage.h \
    view/widgets/choosefilebutton.h \
    view/widgets/coloredprogressbar.h \
    view/widgets/debinfolabel.h \
    view/widgets/droundbgframe.h \
    view/widgets/filechoosewidget.h \
    view/widgets/InfoCommandLinkButton.h \
    view/widgets/infocontrolbutton.h \
    view/widgets/installprocessinfoview.h \
    view/widgets/TitleBarFocusMonitor.h \
    view/widgets/workerprogress.h \
    utils/accessibledefine.h \
    utils/result.h \
    utils/accessible.h \
    utils/utils.h \
    singleInstallerApplication.h

DISTFILES += \
    deepin-deb-installer.applications \
    deepin-deb-installer.desktop \
    ../../translations/deepin-deb-installer.ts \
    ../../translations/deepin-deb-installer_am_ET.ts \
    ../../translations/deepin-deb-installer_ar.ts \
    ../../translations/deepin-deb-installer_ast.ts \
    ../../translations/deepin-deb-installer_bg.ts \
    ../../translations/deepin-deb-installer_ca.ts \
    ../../translations/deepin-deb-installer_cs.ts \
    ../../translations/deepin-deb-installer_da.ts \
    ../../translations/deepin-deb-installer_de.ts \
    ../../translations/deepin-deb-installer_en_AU.ts \
    ../../translations/deepin-deb-installer_es.ts \
    ../../translations/deepin-deb-installer_es_419.ts \
    ../../translations/deepin-deb-installer_et.ts \
    ../../translations/deepin-deb-installer_fi.ts \
    ../../translations/deepin-deb-installer_fr.ts \
    ../../translations/deepin-deb-installer_gl_ES.ts \
    ../../translations/deepin-deb-installer_he.ts \
    ../../translations/deepin-deb-installer_hi_IN.ts \
    ../../translations/deepin-deb-installer_hr.ts \
    ../../translations/deepin-deb-installer_hu.ts \
    ../../translations/deepin-deb-installer_id.ts \
    ../../translations/deepin-deb-installer_it.ts \
    ../../translations/deepin-deb-installer_ko.ts \
    ../../translations/deepin-deb-installer_lt.ts \
    ../../translations/deepin-deb-installer_mn.ts \
    ../../translations/deepin-deb-installer_ms.ts \
    ../../translations/deepin-deb-installer_ne.ts \
    ../../translations/deepin-deb-installer_nl.ts \
    ../../translations/deepin-deb-installer_pa.ts \
    ../../translations/deepin-deb-installer_pl.ts \
    ../../translations/deepin-deb-installer_pt.ts \
    ../../translations/deepin-deb-installer_pt_BR.ts \
    ../../translations/deepin-deb-installer_ru.ts \
    ../../translations/deepin-deb-installer_sk.ts \
    ../../translations/deepin-deb-installer_sl.ts \
    ../../translations/deepin-deb-installer_sr.ts \
    ../../translations/deepin-deb-installer_tr.ts \
    ../../translations/deepin-deb-installer_uk.ts \
    ../../translations/deepin-deb-installer_zh_TW.ts \
    ../../translations/desktop/desktop.ts \
    ../../translations/desktop/desktop_ast.ts \
    ../../translations/desktop/desktop_bg.ts \
    ../../translations/desktop/desktop_ca.ts \
    ../../translations/desktop/desktop_cs.ts \
    ../../translations/desktop/desktop_da.ts \
    ../../translations/desktop/desktop_de.ts \
    ../../translations/desktop/desktop_en_AU.ts \
    ../../translations/desktop/desktop_es.ts \
    ../../translations/desktop/desktop_es_419.ts \
    ../../translations/desktop/desktop_et.ts \
    ../../translations/desktop/desktop_fi.ts \
    ../../translations/desktop/desktop_fr.ts \
    ../../translations/desktop/desktop_hu.ts \
    ../../translations/desktop/desktop_id.ts \
    ../../translations/desktop/desktop_it.ts \
    ../../translations/desktop/desktop_ko.ts \
    ../../translations/desktop/desktop_lt.ts \
    ../../translations/desktop/desktop_mn.ts \
    ../../translations/desktop/desktop_ms.ts \
    ../../translations/desktop/desktop_ne.ts \
    ../../translations/desktop/desktop_nl.ts \
    ../../translations/desktop/desktop_pa.ts \
    ../../translations/desktop/desktop_pl.ts \
    ../../translations/desktop/desktop_pt.ts \
    ../../translations/desktop/desktop_pt_BR.ts \
    ../../translations/desktop/desktop_ru.ts \
    ../../translations/desktop/desktop_sk.ts \
    ../../translations/desktop/desktop_sq.ts \
    ../../translations/desktop/desktop_sr.ts \
    ../../translations/desktop/desktop_tr.ts \
    ../../translations/desktop/desktop_uk.ts \
    ../../translations/desktop/desktop_zh_CN.ts \
    ../../translations/desktop/desktop_zh_TW.ts \
    ../../translations/deepin-deb-installer_zh_CN.ts \
    com.deepin.pkexec.aptInstallDepend.policy

RESOURCES += \
    ../../assets/resources.qrc

policy.path = /usr/share/polkit-1/actions
policy.files = $$PWD/com.deepin.pkexec.aptInstallDepend.policy

TRANSLATIONS += ../../translations/deepin-deb-installer_zh_CN.ts

# Automating generation .qm files from .ts files
!system($$PWD/translate_generation.sh): error("Failed to generate translation")

isEmpty(BINDIR):BINDIR=/usr/bin
isEmpty(APPDIR):APPDIR=/usr/share/applications
isEmpty(APPREGDIR):APPREGDIR=/usr/share/application-registry
isEmpty(DSRDIR):DSRDIR=/usr/share/deepin-deb-installer

!system(deepin-policy-ts-convert policy2ts com.deepin.pkexec.aptInstallDepend.policy.tmp ../../translations/policy): message("Failed policy to ts")
!system(deepin-policy-ts-convert ts2policy com.deepin.pkexec.aptInstallDepend.policy.tmp policy-install-translation com.deepin.pkexec.aptInstallDepend.policy) {
    system(cp com.deepin.pkexec.aptInstallDepend.policy.tmp com.deepin.pkexec.aptInstallDepend.policy)
}

target.path = $$INSTROOT$$BINDIR
desktop.path = $$INSTROOT$$APPDIR
desktop.files = $$PWD/deepin-deb-installer.desktop

applications.path = $$INSTROOT$$APPREGDIR
applications.files = $$PWD/deepin-deb-installer.applications

translations.path = /usr/share/deepin-deb-installer/translations
translations.files = ../../translations/*.qm

icon_files.path = /usr/share/icons/hicolor/scalable/apps
icon_files.files = ../../assets/images/deepin-deb-installer.svg


INSTALLS += target desktop applications translations icon_files policy

CONFIG(release, debug|release) {
    TRANSLATIONS = $$files(../../translations/*.ts)
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    dtk_translations.path = /usr/share/$$TARGET/translations
    dtk_translations.files = ../../translations/*.qm
    INSTALLS += dtk_translations
}
