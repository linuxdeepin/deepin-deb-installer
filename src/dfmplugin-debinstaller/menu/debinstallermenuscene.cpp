// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debinstallermenuscene.h"
#include "debinstallermenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QUrl>
#include <QStandardPaths>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

inline constexpr char kCompatibleInstall[] { "compatible-install" };
inline constexpr char kOpenWithActionID[] { "open-with" };
inline constexpr char kDebInstallService[] { "com.deepin.DebInstaller" };
inline constexpr char kDebInstallPath[] { "/com/deepin/DebInstaller" };

using namespace dfmplugin_debinstaller;
DFMBASE_USE_NAMESPACE

static bool installerHasPackages()
{
    QDBusInterface iface(kDebInstallService, kDebInstallPath,
                         kDebInstallService, QDBusConnection::sessionBus());
    if (!iface.isValid())
        return false;
    QVariant reply = iface.property("hasPackages");
    return reply.isValid() && reply.toBool();
}

static bool isInstallerRunning()
{
    return QDBusInterface(kDebInstallService, kDebInstallPath,
                          kDebInstallService, QDBusConnection::sessionBus())
        .isValid();
}

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

    if (d->isEmptyArea)
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

    bool running = isInstallerRunning();
    if (running)
        targetAction->setEnabled(!installerHasPackages());
    else
        targetAction->setEnabled(true);

    parent->removeAction(targetAction);

    auto actions = parent->actions();
    bool inserted = false;

    for (auto it = actions.begin(); it != actions.end(); ++it) {
        auto actId = (*it)->property(ActionPropertyKey::kActionID).toString();
        if (actId == kOpenWithActionID) {
            ++it;
            if (it != actions.end()) {
                parent->insertAction(*it, targetAction);
            } else {
                parent->addAction(targetAction);
            }
            inserted = true;
            break;
        }
    }

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
        if (d->selectFiles.isEmpty())
            return QProcess::startDetached("deepin-deb-installer", { "--compatible" });

        QString debPath = d->selectFiles.first().toLocalFile();
        return QProcess::startDetached("deepin-deb-installer", { "--compatible", debPath });
    }

    return true;
}
