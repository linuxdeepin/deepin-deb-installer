// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debinstallerplugin.h"
#include "menu/debinstallermenuscene.h"

#include <QTranslator>

inline constexpr char kParentScene[] { "ExtendMenu" };

using namespace dfmbase;
using namespace dfmplugin_debinstaller;

void DebInstallerPlugin::initialize()
{
    auto translator = new QTranslator(this);
    Q_UNUSED(translator->load(QLocale(), "deepin-deb-installer", "_",
                              "/usr/share/deepin-deb-installer/translations"));
    QCoreApplication::installTranslator(translator);

    if (DPF_NAMESPACE::LifeCycle::isAllPluginsStarted()) {
        bindMenuScene();
    } else {
        connect(dpfListener, &DPF_NAMESPACE::Listener::pluginsStarted,
                this, &DebInstallerPlugin::bindMenuScene, Qt::DirectConnection);
    }
}

bool DebInstallerPlugin::start()
{
    return true;
}

void DebInstallerPlugin::bindMenuScene()
{
    dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_RegisterScene",
                         DebInstallerMenuCreator::name(), new DebInstallerMenuCreator);

    bool ret = dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_Contains",
                                    QString(kParentScene))
                       .toBool();
    if (ret) {
        dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_Bind",
                             DebInstallerMenuCreator::name(), QString(kParentScene));
    } else {
        dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                       this, &DebInstallerPlugin::onMenuSceneAdded);
    }
}

void DebInstallerPlugin::onMenuSceneAdded(const QString &scene)
{
    if (scene == kParentScene) {
        dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_Bind",
                             DebInstallerMenuCreator::name(), QString(kParentScene));
        dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                         this, &DebInstallerPlugin::onMenuSceneAdded);
    }
}
