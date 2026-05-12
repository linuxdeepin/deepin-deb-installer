// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLE_INSTALL_BACKEND_H
#define COMPATIBLE_INSTALL_BACKEND_H

#include "install_backend.h"

#ifndef DISABLE_COMPATIBLE

#include "compatible/compatible_process_controller.h"

#include <QScopedPointer>

class CompatibleInstallBackend : public InstallBackend
{
    Q_OBJECT
    friend class DebListModel;

public:
    explicit CompatibleInstallBackend(DebListModel *model, QObject *parent = nullptr);
    ~CompatibleInstallBackend() override;

    bool verifySignature(int index) override;
    bool install(int index) override;
    bool uninstall(int index) override;

    // Expose for DebListModel::slotConfigInputWrite
    [[nodiscard]] bool isRunning() const;
    void writeConfigData(const QString &configData);
    [[nodiscard]] bool needTemplates() const;

private:
    void ensureProcessor();

    QScopedPointer<Compatible::CompatibleProcessController> m_compProcessor;

    Q_DISABLE_COPY(CompatibleInstallBackend)
};

#endif  // DISABLE_COMPATIBLE

#endif  // COMPATIBLE_INSTALL_BACKEND_H
