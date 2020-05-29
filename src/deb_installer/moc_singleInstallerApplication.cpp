/****************************************************************************
** Meta object code from reading C++ file 'singleInstallerApplication.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "singleInstallerApplication.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'singleInstallerApplication.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SingleInstallerApplication_t {
    QByteArrayData data[6];
    char stringdata0[93];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SingleInstallerApplication_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SingleInstallerApplication_t qt_meta_stringdata_SingleInstallerApplication = {
    {
QT_MOC_LITERAL(0, 0, 26), // "SingleInstallerApplication"
QT_MOC_LITERAL(1, 27, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 43, 23), // "com.deepin.DebInstaller"
QT_MOC_LITERAL(3, 67, 12), // "InstallerDeb"
QT_MOC_LITERAL(4, 80, 0), // ""
QT_MOC_LITERAL(5, 81, 11) // "debPathList"

    },
    "SingleInstallerApplication\0D-Bus Interface\0"
    "com.deepin.DebInstaller\0InstallerDeb\0"
    "\0debPathList"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SingleInstallerApplication[] = {

 // content:
       7,       // revision
       0,       // classname
       1,   14, // classinfo
       1,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
       1,    2,

 // slots: name, argc, parameters, tag, flags
       3,    1,   21,    4, 0x4a /* Public | isScriptable */,

 // slots: parameters
    QMetaType::Void, QMetaType::QStringList,    5,

       0        // eod
};

void SingleInstallerApplication::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SingleInstallerApplication *_t = static_cast<SingleInstallerApplication *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->InstallerDeb((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SingleInstallerApplication::staticMetaObject = {
    { &DApplication::staticMetaObject, qt_meta_stringdata_SingleInstallerApplication.data,
      qt_meta_data_SingleInstallerApplication,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SingleInstallerApplication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SingleInstallerApplication::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SingleInstallerApplication.stringdata0))
        return static_cast<void*>(this);
    return DApplication::qt_metacast(_clname);
}

int SingleInstallerApplication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
