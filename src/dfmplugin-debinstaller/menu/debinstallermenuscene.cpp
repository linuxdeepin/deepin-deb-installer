// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debinstallermenuscene.h"
#include "debinstallermenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QUrl>
#include <QProcess>
#include <QStandardPaths>

inline constexpr char kCompatibleInstall[] { "compatible-install" };
inline constexpr char kOpenWithActionID[] { "open-with" };

using namespace dfmplugin_debinstaller;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *DebInstallerMenuCreator::create()
{
    return new DebInstallerMenuScene();
}

DebInstallerMenuScenePrivate::DebInstallerMenuScenePrivate(DebInstallerMenuScene *qq)
    : q(qq)
{
}

DebInstallerMenuScene::DebInstallerMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new DebInstallerMenuScenePrivate(this))
{
    d->predicateName[kCompatibleInstall] = tr("Install in compatible mode");
}

DebInstallerMenuScene::~DebInstallerMenuScene()
{
}

QString DebInstallerMenuScene::name() const
{
    return DebInstallerMenuCreator::name();
}

bool DebInstallerMenuScene::initialize(const QVariantHash &params)
{
    if (QStandardPaths::findExecutable("deepin-compatible-ctl").isEmpty())
        return false;

    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    // only show for single deb file selection, not empty area
    if (d->isEmptyArea || d->selectFiles.size() != 1)
        return false;

    const auto &url = d->selectFiles.first();
    if (!url.isLocalFile() || !url.toLocalFile().endsWith(".deb"))
        return false;

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *DebInstallerMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<DebInstallerMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool DebInstallerMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    auto act = parent->addAction(d->predicateName.value(kCompatibleInstall));
    d->predicateAction[kCompatibleInstall] = act;
    act->setProperty(ActionPropertyKey::kActionID, kCompatibleInstall);

    return AbstractMenuScene::create(parent);
}

void DebInstallerMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    auto targetAction = d->predicateAction.value(kCompatibleInstall);
    if (!targetAction)
        return AbstractMenuScene::updateState(parent);

    // disable if deb installer is already running
    QProcess proc;
    proc.start("pidof", { "deepin-deb-installer" });
    proc.waitForFinished(1000);
    targetAction->setEnabled(proc.exitCode() != 0);

    // remove our action first
    parent->removeAction(targetAction);

    // find "open-with" action and insert our action after it
    auto actions = parent->actions();
    bool inserted = false;

    for (auto it = actions.begin(); it != actions.end(); ++it) {
        auto actId = (*it)->property(ActionPropertyKey::kActionID).toString();
        if (actId == kOpenWithActionID) {
            ++it;   // advance past "open-with"
            // insert before the next action (or append if open-with was last)
            if (it != actions.end()) {
                parent->insertAction(*it, targetAction);
            } else {
                parent->addAction(targetAction);
            }
            inserted = true;
            break;
        }
    }

    // fallback: if "open-with" not found, append at end
    if (!inserted)
        parent->addAction(targetAction);

    AbstractMenuScene::updateState(parent);
}

bool DebInstallerMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (!d->predicateAction.contains(actionId))
        return AbstractMenuScene::triggered(action);

    if (actionId == kCompatibleInstall) {
        QString debPath = d->selectFiles.first().toLocalFile();
        return QProcess::startDetached("deepin-deb-installer", { "--compatible", debPath });
    }

    return true;
}
