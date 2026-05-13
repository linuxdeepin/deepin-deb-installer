// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBINSTALLERMENUSCENE_H
#define DEBINSTALLERMENUSCENE_H

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_debinstaller {

class DebInstallerMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "DebInstallerMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class DebInstallerMenuScenePrivate;
class DebInstallerMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit DebInstallerMenuScene(QObject *parent = nullptr);
    ~DebInstallerMenuScene() override;

    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    QScopedPointer<DebInstallerMenuScenePrivate> d;
};

}   // namespace dfmplugin_debinstaller

#endif   // DEBINSTALLERMENUSCENE_H
