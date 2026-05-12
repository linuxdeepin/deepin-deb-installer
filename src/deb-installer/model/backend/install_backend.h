// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTALL_BACKEND_H
#define INSTALL_BACKEND_H

#include <QObject>

class QString;

class DebListModel;

/**
 * @brief Abstract interface for install backends.
 *
 * Each backend (apt, compatible, immutable) implements this interface
 * to provide its own install/uninstall lifecycle and signature verification.
 */
class InstallBackend : public QObject
{
    Q_OBJECT

public:
    explicit InstallBackend(DebListModel *model, QObject *parent = nullptr);
    ~InstallBackend() override = default;

    virtual bool verifySignature(int index) = 0;
    virtual bool install(int index) = 0;
    virtual bool uninstall(int index) = 0;

protected:
    DebListModel *m_model{nullptr};

    Q_DISABLE_COPY(InstallBackend)
};

#endif  // INSTALL_BACKEND_H
