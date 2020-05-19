/****************************************************************************
** Meta object code from reading C++ file 'packagesmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "packagesmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'packagesmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PackagesManagerDependsStatus__dealDependThread_t {
    QByteArrayData data[10];
    char stringdata0[134];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PackagesManagerDependsStatus__dealDependThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PackagesManagerDependsStatus__dealDependThread_t qt_meta_stringdata_PackagesManagerDependsStatus__dealDependThread = {
    {
QT_MOC_LITERAL(0, 0, 46), // "PackagesManagerDependsStatus:..."
QT_MOC_LITERAL(1, 47, 16), // "DealDependFinish"
QT_MOC_LITERAL(2, 64, 0), // ""
QT_MOC_LITERAL(3, 65, 4), // "name"
QT_MOC_LITERAL(4, 70, 7), // "bFinish"
QT_MOC_LITERAL(5, 78, 6), // "iIndex"
QT_MOC_LITERAL(6, 85, 15), // "DependEnableBtn"
QT_MOC_LITERAL(7, 101, 7), // "bEnable"
QT_MOC_LITERAL(8, 109, 10), // "onFinished"
QT_MOC_LITERAL(9, 120, 13) // "on_readoutput"

    },
    "PackagesManagerDependsStatus::dealDependThread\0"
    "DealDependFinish\0\0name\0bFinish\0iIndex\0"
    "DependEnableBtn\0bEnable\0onFinished\0"
    "on_readoutput"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PackagesManagerDependsStatus__dealDependThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   34,    2, 0x06 /* Public */,
       6,    1,   41,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   44,    2, 0x0a /* Public */,
       9,    0,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::Bool,    7,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

       0        // eod
};

void PackagesManagerDependsStatus::dealDependThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        dealDependThread *_t = static_cast<dealDependThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->DealDependFinish((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->DependEnableBtn((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onFinished((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_readoutput(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (dealDependThread::*)(QString , bool , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&dealDependThread::DealDependFinish)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (dealDependThread::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&dealDependThread::DependEnableBtn)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PackagesManagerDependsStatus::dealDependThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_PackagesManagerDependsStatus__dealDependThread.data,
      qt_meta_data_PackagesManagerDependsStatus__dealDependThread,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *PackagesManagerDependsStatus::dealDependThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PackagesManagerDependsStatus::dealDependThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PackagesManagerDependsStatus__dealDependThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int PackagesManagerDependsStatus::dealDependThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void PackagesManagerDependsStatus::dealDependThread::DealDependFinish(QString _t1, bool _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PackagesManagerDependsStatus::dealDependThread::DependEnableBtn(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_PackagesManager_t {
    QByteArrayData data[10];
    char stringdata0[121];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PackagesManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PackagesManager_t qt_meta_stringdata_PackagesManager = {
    {
QT_MOC_LITERAL(0, 0, 15), // "PackagesManager"
QT_MOC_LITERAL(1, 16, 18), // "DeepinWineFinished"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 15), // "DependEnableBtn"
QT_MOC_LITERAL(4, 52, 20), // "DealDependFinishSlot"
QT_MOC_LITERAL(5, 73, 4), // "name"
QT_MOC_LITERAL(6, 78, 7), // "bFinish"
QT_MOC_LITERAL(7, 86, 6), // "iIndex"
QT_MOC_LITERAL(8, 93, 19), // "DealDependEnableBtn"
QT_MOC_LITERAL(9, 113, 7) // "bEnable"

    },
    "PackagesManager\0DeepinWineFinished\0\0"
    "DependEnableBtn\0DealDependFinishSlot\0"
    "name\0bFinish\0iIndex\0DealDependEnableBtn\0"
    "bEnable"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PackagesManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    1,   35,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    3,   38,    2, 0x0a /* Public */,
       8,    1,   45,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool, QMetaType::Int,    5,    6,    7,
    QMetaType::Void, QMetaType::Bool,    9,

       0        // eod
};

void PackagesManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PackagesManager *_t = static_cast<PackagesManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->DeepinWineFinished(); break;
        case 1: _t->DependEnableBtn((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->DealDependFinishSlot((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: _t->DealDependEnableBtn((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PackagesManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PackagesManager::DeepinWineFinished)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PackagesManager::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PackagesManager::DependEnableBtn)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PackagesManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PackagesManager.data,
      qt_meta_data_PackagesManager,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *PackagesManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PackagesManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PackagesManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PackagesManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void PackagesManager::DeepinWineFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PackagesManager::DependEnableBtn(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
