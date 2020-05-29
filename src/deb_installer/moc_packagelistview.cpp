/****************************************************************************
** Meta object code from reading C++ file 'packagelistview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "packagelistview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'packagelistview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PackagesListView_t {
    QByteArrayData data[13];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PackagesListView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PackagesListView_t qt_meta_stringdata_PackagesListView = {
    {
QT_MOC_LITERAL(0, 0, 16), // "PackagesListView"
QT_MOC_LITERAL(1, 17, 15), // "onShowHideTopBg"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 5), // "bShow"
QT_MOC_LITERAL(4, 40, 18), // "onShowHideBottomBg"
QT_MOC_LITERAL(5, 59, 18), // "onClickItemAtIndex"
QT_MOC_LITERAL(6, 78, 11), // "QModelIndex"
QT_MOC_LITERAL(7, 90, 5), // "index"
QT_MOC_LITERAL(8, 96, 17), // "onShowContextMenu"
QT_MOC_LITERAL(9, 114, 19), // "onRemoveItemClicked"
QT_MOC_LITERAL(10, 134, 25), // "onListViewShowContextMenu"
QT_MOC_LITERAL(11, 160, 23), // "onRightMenuDeleteAction"
QT_MOC_LITERAL(12, 184, 22) // "onShortcutDeleteAction"

    },
    "PackagesListView\0onShowHideTopBg\0\0"
    "bShow\0onShowHideBottomBg\0onClickItemAtIndex\0"
    "QModelIndex\0index\0onShowContextMenu\0"
    "onRemoveItemClicked\0onListViewShowContextMenu\0"
    "onRightMenuDeleteAction\0onShortcutDeleteAction"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PackagesListView[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    1,   57,    2, 0x06 /* Public */,
       5,    1,   60,    2, 0x06 /* Public */,
       8,    1,   63,    2, 0x06 /* Public */,
       9,    1,   66,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,   69,    2, 0x08 /* Private */,
      11,    0,   72,    2, 0x08 /* Private */,
      12,    0,   73,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 6,    7,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PackagesListView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PackagesListView *_t = static_cast<PackagesListView *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onShowHideTopBg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->onShowHideBottomBg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onClickItemAtIndex((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 3: _t->onShowContextMenu((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 4: _t->onRemoveItemClicked((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 5: _t->onListViewShowContextMenu((*reinterpret_cast< QModelIndex(*)>(_a[1]))); break;
        case 6: _t->onRightMenuDeleteAction(); break;
        case 7: _t->onShortcutDeleteAction(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PackagesListView::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PackagesListView::onShowHideTopBg)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PackagesListView::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PackagesListView::onShowHideBottomBg)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PackagesListView::*)(QModelIndex );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PackagesListView::onClickItemAtIndex)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PackagesListView::*)(QModelIndex );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PackagesListView::onShowContextMenu)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PackagesListView::*)(QModelIndex );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PackagesListView::onRemoveItemClicked)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PackagesListView::staticMetaObject = {
    { &DListView::staticMetaObject, qt_meta_stringdata_PackagesListView.data,
      qt_meta_data_PackagesListView,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *PackagesListView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PackagesListView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PackagesListView.stringdata0))
        return static_cast<void*>(this);
    return DListView::qt_metacast(_clname);
}

int PackagesListView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DListView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void PackagesListView::onShowHideTopBg(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PackagesListView::onShowHideBottomBg(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PackagesListView::onClickItemAtIndex(QModelIndex _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PackagesListView::onShowContextMenu(QModelIndex _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PackagesListView::onRemoveItemClicked(QModelIndex _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
