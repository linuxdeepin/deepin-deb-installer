/****************************************************************************
** Meta object code from reading C++ file 'appendpackagethread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "appendpackagethread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'appendpackagethread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AppendPackageThread_t {
    QByteArrayData data[9];
    char stringdata0[98];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AppendPackageThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AppendPackageThread_t qt_meta_stringdata_AppendPackageThread = {
    {
QT_MOC_LITERAL(0, 0, 19), // "AppendPackageThread"
QT_MOC_LITERAL(1, 20, 7), // "refresh"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 3), // "idx"
QT_MOC_LITERAL(4, 33, 17), // "packageAlreadyAdd"
QT_MOC_LITERAL(5, 51, 15), // "addSingleFinish"
QT_MOC_LITERAL(6, 67, 6), // "enable"
QT_MOC_LITERAL(7, 74, 14), // "addMultiFinish"
QT_MOC_LITERAL(8, 89, 8) // "addStart"

    },
    "AppendPackageThread\0refresh\0\0idx\0"
    "packageAlreadyAdd\0addSingleFinish\0"
    "enable\0addMultiFinish\0addStart"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AppendPackageThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       1,    0,   47,    2, 0x26 /* Public | MethodCloned */,
       4,    0,   48,    2, 0x06 /* Public */,
       5,    1,   49,    2, 0x06 /* Public */,
       7,    1,   52,    2, 0x06 /* Public */,
       8,    1,   55,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,

       0        // eod
};

void AppendPackageThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AppendPackageThread *_t = static_cast<AppendPackageThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->refresh((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->refresh(); break;
        case 2: _t->packageAlreadyAdd(); break;
        case 3: _t->addSingleFinish((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->addMultiFinish((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->addStart((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AppendPackageThread::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppendPackageThread::refresh)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AppendPackageThread::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppendPackageThread::packageAlreadyAdd)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (AppendPackageThread::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppendPackageThread::addSingleFinish)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (AppendPackageThread::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppendPackageThread::addMultiFinish)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (AppendPackageThread::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppendPackageThread::addStart)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AppendPackageThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_AppendPackageThread.data,
      qt_meta_data_AppendPackageThread,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *AppendPackageThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AppendPackageThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AppendPackageThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int AppendPackageThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void AppendPackageThread::refresh(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 2
void AppendPackageThread::packageAlreadyAdd()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void AppendPackageThread::addSingleFinish(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AppendPackageThread::addMultiFinish(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void AppendPackageThread::addStart(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
