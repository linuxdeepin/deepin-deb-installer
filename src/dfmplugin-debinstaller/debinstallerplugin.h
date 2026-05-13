// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBINSTALLERPLUGIN_H
#define DEBINSTALLERPLUGIN_H

#include <dfm-framework/dpf.h>

namespace dfmplugin_debinstaller {

class DebInstallerPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "debinstallerplugin.json")

    DPF_EVENT_NAMESPACE(dfmplugin_debinstaller)

public:
    void initialize() override;
    bool start() override;

private Q_SLOTS:
    void bindMenuScene();
    void onMenuSceneAdded(const QString &scene);
};

}   // namespace dfmplugin_debinstaller

#endif   // DEBINSTALLERPLUGIN_H
