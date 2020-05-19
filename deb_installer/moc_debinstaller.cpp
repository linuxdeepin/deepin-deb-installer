/****************************************************************************
** Meta object code from reading C++ file 'debinstaller.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "debinstaller.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'debinstaller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DebInstaller_t {
    QByteArrayData data[22];
    char stringdata0[301];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DebInstaller_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DebInstaller_t qt_meta_stringdata_DebInstaller = {
    {
QT_MOC_LITERAL(0, 0, 12), // "DebInstaller"
QT_MOC_LITERAL(1, 13, 18), // "onPackagesSelected"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 8), // "packages"
QT_MOC_LITERAL(4, 42, 24), // "showUninstallConfirmPage"
QT_MOC_LITERAL(5, 67, 19), // "onUninstallAccepted"
QT_MOC_LITERAL(6, 87, 19), // "onUninstallCalceled"
QT_MOC_LITERAL(7, 107, 9), // "onAuthing"
QT_MOC_LITERAL(8, 117, 7), // "authing"
QT_MOC_LITERAL(9, 125, 12), // "onNewAppOpen"
QT_MOC_LITERAL(10, 138, 3), // "pid"
QT_MOC_LITERAL(11, 142, 9), // "arguments"
QT_MOC_LITERAL(12, 152, 23), // "onStartInstallRequested"
QT_MOC_LITERAL(13, 176, 5), // "reset"
QT_MOC_LITERAL(14, 182, 13), // "removePackage"
QT_MOC_LITERAL(15, 196, 5), // "index"
QT_MOC_LITERAL(16, 202, 14), // "changeDragFlag"
QT_MOC_LITERAL(17, 217, 15), // "setEnableButton"
QT_MOC_LITERAL(18, 233, 7), // "bEnable"
QT_MOC_LITERAL(19, 241, 16), // "showHiddenButton"
QT_MOC_LITERAL(20, 258, 22), // "dealDeepinWineFinished"
QT_MOC_LITERAL(21, 281, 19) // "dealDependEnableBtn"

    },
    "DebInstaller\0onPackagesSelected\0\0"
    "packages\0showUninstallConfirmPage\0"
    "onUninstallAccepted\0onUninstallCalceled\0"
    "onAuthing\0authing\0onNewAppOpen\0pid\0"
    "arguments\0onStartInstallRequested\0"
    "reset\0removePackage\0index\0changeDragFlag\0"
    "setEnableButton\0bEnable\0showHiddenButton\0"
    "dealDeepinWineFinished\0dealDependEnableBtn"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DebInstaller[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x08 /* Private */,
       4,    0,   87,    2, 0x08 /* Private */,
       5,    0,   88,    2, 0x08 /* Private */,
       6,    0,   89,    2, 0x08 /* Private */,
       7,    1,   90,    2, 0x08 /* Private */,
       9,    2,   93,    2, 0x08 /* Private */,
      12,    0,   98,    2, 0x08 /* Private */,
      13,    0,   99,    2, 0x08 /* Private */,
      14,    1,  100,    2, 0x08 /* Private */,
      16,    0,  103,    2, 0x08 /* Private */,
      17,    1,  104,    2, 0x08 /* Private */,
      19,    0,  107,    2, 0x08 /* Private */,
      20,    0,  108,    2, 0x08 /* Private */,
      21,    1,  109,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QStringList,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QStringList,   10,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   18,

       0        // eod
};

void DebInstaller::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DebInstaller *_t = static_cast<DebInstaller *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onPackagesSelected((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 1: _t->showUninstallConfirmPage(); break;
        case 2: _t->onUninstallAccepted(); break;
        case 3: _t->onUninstallCalceled(); break;
        case 4: _t->onAuthing((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 5: _t->onNewAppOpen((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 6: _t->onStartInstallRequested(); break;
        case 7: _t->reset(); break;
        case 8: _t->removePackage((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 9: _t->changeDragFlag(); break;
        case 10: _t->setEnableButton((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->showHiddenButton(); break;
        case 12: _t->dealDeepinWineFinished(); break;
        case 13: _t->dealDependEnableBtn((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DebInstaller::staticMetaObject = {
    { &Dtk::Widget::DMainWindow::staticMetaObject, qt_meta_stringdata_DebInstaller.data,
      qt_meta_data_DebInstaller,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DebInstaller::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DebInstaller::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DebInstaller.stringdata0))
        return static_cast<void*>(this);
    return Dtk::Widget::DMainWindow::qt_metacast(_clname);
}

int DebInstaller::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Dtk::Widget::DMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
