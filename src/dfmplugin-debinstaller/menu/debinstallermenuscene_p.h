// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBINSTALLERMENUSCENE_P_H
#define DEBINSTALLERMENUSCENE_P_H

#include <QMenu>

namespace dfmplugin_debinstaller {

class DebInstallerMenuScene;
class DebInstallerMenuScenePrivate
{
public:
    explicit DebInstallerMenuScenePrivate(DebInstallerMenuScene *qq);

public:
    DebInstallerMenuScene *q;

    QList<QUrl> selectFiles;
    bool isEmptyArea { false };
    QMap<QString, QAction *> predicateAction;
    QMap<QString, QString> predicateName;
};

}   // namespace dfmplugin_debinstaller

#endif   // DEBINSTALLERMENUSCENE_P_H
