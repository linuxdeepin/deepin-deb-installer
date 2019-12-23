/****************************************************************************
** Meta object code from reading C++ file 'multipleinstallpage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "multipleinstallpage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'multipleinstallpage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MultipleInstallPage_t {
    QByteArrayData data[18];
    char stringdata0[246];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MultipleInstallPage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MultipleInstallPage_t qt_meta_stringdata_MultipleInstallPage = {
    {
QT_MOC_LITERAL(0, 0, 19), // "MultipleInstallPage"
QT_MOC_LITERAL(1, 20, 4), // "back"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 20), // "requestRemovePackage"
QT_MOC_LITERAL(4, 47, 5), // "index"
QT_MOC_LITERAL(5, 53, 16), // "hideAutoBarTitle"
QT_MOC_LITERAL(6, 70, 15), // "onWorkerFinshed"
QT_MOC_LITERAL(7, 86, 17), // "onOutputAvailable"
QT_MOC_LITERAL(8, 104, 6), // "output"
QT_MOC_LITERAL(9, 111, 17), // "onProgressChanged"
QT_MOC_LITERAL(10, 129, 8), // "progress"
QT_MOC_LITERAL(11, 138, 26), // "onRequestRemoveItemClicked"
QT_MOC_LITERAL(12, 165, 11), // "QModelIndex"
QT_MOC_LITERAL(13, 177, 8), // "showInfo"
QT_MOC_LITERAL(14, 186, 8), // "hideInfo"
QT_MOC_LITERAL(15, 195, 23), // "onAutoScrollInstallList"
QT_MOC_LITERAL(16, 219, 7), // "opIndex"
QT_MOC_LITERAL(17, 227, 18) // "hiddenCancelButton"

    },
    "MultipleInstallPage\0back\0\0"
    "requestRemovePackage\0index\0hideAutoBarTitle\0"
    "onWorkerFinshed\0onOutputAvailable\0"
    "output\0onProgressChanged\0progress\0"
    "onRequestRemoveItemClicked\0QModelIndex\0"
    "showInfo\0hideInfo\0onAutoScrollInstallList\0"
    "opIndex\0hiddenCancelButton"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MultipleInstallPage[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x06 /* Public */,
       3,    1,   70,    2, 0x06 /* Public */,
       5,    0,   73,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   74,    2, 0x08 /* Private */,
       7,    1,   75,    2, 0x08 /* Private */,
       9,    1,   78,    2, 0x08 /* Private */,
      11,    1,   81,    2, 0x08 /* Private */,
      13,    0,   84,    2, 0x08 /* Private */,
      14,    0,   85,    2, 0x08 /* Private */,
      15,    1,   86,    2, 0x08 /* Private */,
      17,    0,   89,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, 0x80000000 | 12,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void,

       0        // eod
};

void MultipleInstallPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MultipleInstallPage *_t = static_cast<MultipleInstallPage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->back(); break;
        case 1: _t->requestRemovePackage((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 2: _t->hideAutoBarTitle(); break;
        case 3: _t->onWorkerFinshed(); break;
        case 4: _t->onOutputAvailable((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->onProgressChanged((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 6: _t->onRequestRemoveItemClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 7: _t->showInfo(); break;
        case 8: _t->hideInfo(); break;
        case 9: _t->onAutoScrollInstallList((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->hiddenCancelButton(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MultipleInstallPage::*)() const;
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MultipleInstallPage::back)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MultipleInstallPage::*)(const int ) const;
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MultipleInstallPage::requestRemovePackage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MultipleInstallPage::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MultipleInstallPage::hideAutoBarTitle)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MultipleInstallPage::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MultipleInstallPage.data,
      qt_meta_data_MultipleInstallPage,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *MultipleInstallPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MultipleInstallPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MultipleInstallPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MultipleInstallPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void MultipleInstallPage::back()const
{
    QMetaObject::activate(const_cast< MultipleInstallPage *>(this), &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MultipleInstallPage::requestRemovePackage(const int _t1)const
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(const_cast< MultipleInstallPage *>(this), &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MultipleInstallPage::hideAutoBarTitle()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
