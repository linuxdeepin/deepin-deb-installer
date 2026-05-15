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
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDBusConnectionInterface>

inline constexpr char kCompatibleInstall[] { "compatible-install" };
inline constexpr char kOpenWithActionID[] { "open-with" };
inline constexpr char kDebInstallService[] { "com.deepin.DebInstaller" };
inline constexpr char kDebInstallPath[] { "/com/deepin/DebInstaller" };

using namespace dfmplugin_debinstaller;
DFMBASE_USE_NAMESPACE

enum InstallerState : int {
    NotRunning = 0,   // service not on bus, no activation risk
    Ready = 1,        // service running, supports compat, no packages loaded
    Busy = 2,         // service running, supports compat, has packages loaded
    NotSupported = 3  // service running but lacks hasPackages (old version)
};

static InstallerState checkInstallerState()
{
    auto bus = QDBusConnection::sessionBus();
    auto connIface = bus.interface();
    if (!connIface || !connIface->isServiceRegistered(kDebInstallService))
        return NotRunning;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        kDebInstallService, kDebInstallPath,
        "org.freedesktop.DBus.Properties", "Get");
    msg << QString(kDebInstallService) << QString("hasPackages");

    QDBusMessage reply = bus.call(msg, QDBus::Block, 500);
    if (reply.type() == QDBusMessage::ErrorMessage)
        return NotSupported;

    QVariant arg0 = reply.arguments().value(0);
    if (!arg0.canConvert<QDBusVariant>())
        return NotSupported;
    QVariant value = arg0.value<QDBusVariant>().variant();
    return value.toBool() ? Busy : Ready;
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

    switch (checkInstallerState()) {
    case NotRunning:
    case Ready:
    case NotSupported:
        targetAction->setEnabled(true);
        break;
    case Busy:
        targetAction->setEnabled(false);
        break;
    }

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
