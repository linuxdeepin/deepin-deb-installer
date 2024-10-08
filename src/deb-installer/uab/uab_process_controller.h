// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABPROCESSCONTROLLER_H
#define UABPROCESSCONTROLLER_H

#include <QObject>

#include "uab_package.h"

class QProcess;

namespace Uab {

class UabProcessController : public QObject
{
    Q_OBJECT

public:
    explicit UabProcessController(QObject *parent = nullptr);
    ~UabProcessController() override = default;

    enum ProcFlag {
        Prepare = 1 << 0,
        Uninstalling = 1 << 1,
        Installing = 1 << 2,
        Processing = 1 << 3,
        Finish = 1 << 4,
        Error = 1 << 5,
    };
    Q_DECLARE_FLAGS(ProcFlags, ProcFlag)
    Q_FLAG(ProcFlags)
    ProcFlags procFlag() const;
    bool isRunning() const;

    bool reset();
    bool markInstall(const UabPackage::Ptr &installPtr);
    bool markUninstall(const UabPackage::Ptr &unisntallPtr);
    bool commitChanges();

    Q_SIGNAL void processStart();
    Q_SIGNAL void processFinished(bool success);
    Q_SIGNAL void processOutput(const QString &output);
    Q_SIGNAL void progressChanged(float progress);

private:
    bool ensureProcess();
    void parseProgressFromJson(const QByteArray &jsonData);
    void parseProgressFromRawOutput(const QByteArray &output);
    Q_SLOT void onReadOutput();
    Q_SLOT void onFinished(int exitCode, int exitStatus);

    bool nextProcess();

    bool installImpl(const UabPackage::Ptr &installPtr);
    bool uninstallImpl(const UabPackage::Ptr &uninstallPtr);

    bool checkIndexValid();
    UabPackage::Ptr currentPackagePtr();

    void commitCurrentChangeToBackend();

private:
    QProcess *m_process{nullptr};

    ProcFlags m_procFlag{Prepare};
    int m_currentIndex{-1};
    QList<QPair<ProcFlag, UabPackage::Ptr>> m_procList;  // install/uninstall package list

    Q_DISABLE_COPY(UabProcessController);
};

}  // namespace Uab

#endif  // UABPROCESSCONTROLLER_H
