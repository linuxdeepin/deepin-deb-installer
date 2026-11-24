// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QAPTKEEPMODECOMPAT_H
#define QAPTKEEPMODECOMPAT_H

namespace QApt {
class Backend;
}

/**
 * Runtime compatibility shim for libqapt's optional keep mode.
 *
 * The keep mode symbols may be absent when this installer runs against an
 * older libqapt. Resolve them with dlsym so the installer still starts and
 * the normal install / uninstall paths keep working; keep mode is simply
 * reported as unavailable.
 */
namespace QaptKeepModeCompat {

bool available();
void setKeepMode(QApt::Backend *backend, bool enable);
bool keepMode(const QApt::Backend *backend);
bool keepModeFailed(const QApt::Backend *backend);

}  // namespace QaptKeepModeCompat

#endif  // QAPTKEEPMODECOMPAT_H
