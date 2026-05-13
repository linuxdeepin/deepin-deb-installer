// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APT_INSTALL_BACKEND_H
#define APT_INSTALL_BACKEND_H

#include "install_backend.h"

#include <QApt/Transaction>

#include <QPointer>

class AptInstallBackend : public InstallBackend
{
    Q_OBJECT
    friend class DebListModel;

public:
    explicit AptInstallBackend(DebListModel *model, QObject *parent = nullptr);
    ~AptInstallBackend() override;

    bool verifySignature(int index) override;
    bool verifySignature(const QString &packagePath);
    bool install(int index) override;
    bool uninstall(int index) override;

private slots:
    void slotTransactionOutput();
    void slotTransactionFinished();
    void slotDependsInstallTransactionFinished();
    void slotTransactionErrorOccurred();
    void slotTransactionStatusChanged(QApt::TransactionStatus transactionStatus);
    void slotUninstallFinished();

private:
    void setEndEnable();
    void checkBoxStatus();

    QPointer<QApt::Transaction> m_currentTransaction;

    Q_DISABLE_COPY(AptInstallBackend)
};

#endif  // APT_INSTALL_BACKEND_H
