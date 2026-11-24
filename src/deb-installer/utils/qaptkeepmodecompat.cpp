// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qaptkeepmodecompat.h"

#include <dlfcn.h>

namespace {

using SetKeepModeFn = void (*)(QApt::Backend *, bool);
using ConstBoolFn = bool (*)(const QApt::Backend *);

struct KeepModeFunctions {
    SetKeepModeFn setKeepMode = nullptr;
    ConstBoolFn keepMode = nullptr;
    ConstBoolFn keepModeFailed = nullptr;
};

const KeepModeFunctions &functions()
{
    static const KeepModeFunctions funcs = []() {
        KeepModeFunctions f;
        f.setKeepMode = reinterpret_cast<SetKeepModeFn>(
                dlsym(RTLD_DEFAULT, "qapt_backend_set_keep_mode"));
        f.keepMode = reinterpret_cast<ConstBoolFn>(
                dlsym(RTLD_DEFAULT, "qapt_backend_keep_mode"));
        f.keepModeFailed = reinterpret_cast<ConstBoolFn>(
                dlsym(RTLD_DEFAULT, "qapt_backend_keep_mode_failed"));
        return f;
    }();
    return funcs;
}

}  // namespace

namespace QaptKeepModeCompat {

bool available()
{
    const KeepModeFunctions &f = functions();
    return f.setKeepMode && f.keepMode && f.keepModeFailed;
}

void setKeepMode(QApt::Backend *backend, bool enable)
{
    if (backend && available())
        functions().setKeepMode(backend, enable);
}

bool keepMode(const QApt::Backend *backend)
{
    return backend && available() ? functions().keepMode(backend) : false;
}

bool keepModeFailed(const QApt::Backend *backend)
{
    return backend && available() ? functions().keepModeFailed(backend) : false;
}

}  // namespace QaptKeepModeCompat
