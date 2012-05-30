/****************************************************************************
**
** Copyright (C) 2012 Sebastian Lehmann
** Contact: contact@l3.ms
**
**
** This file is part of QtSimpleRPC.
**
** QtSimpleRPC is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** QtSimpleRPC is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "rpcsignalmapperhelper.h"
#include <QVariant>

RpcSignalMapperHelper::RpcSignalMapperHelper(QMetaMethod mappedMethod, QObject *parent) :
    QObject(parent),
    method(mappedMethod)
{
}

void RpcSignalMapperHelper::map()
{
    qWarning("RpcSignalMapperHelper::map() has been called directly.");
}

void RpcSignalMapperHelper::internalSignalHandler(void **_a)
{
    QVariantList args;
    int i = 0;
    foreach(QByteArray typeName, method.parameterTypes())
    {
        int type = QMetaType::type(typeName.constData());
        QVariant arg(type, _a[++i]); // precrement will start with 1
        args << arg;
    }
    emit mapped(sender(), method, args);
}




//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



/****************************************************************************
**  THIS IS THE META OBJECT CODE AS GENERATED BY MOC, BUT CUSTOMIZED.
**
**  THIS CODE IS COMPATIBLE WITH:
**   * Q_MOC_OUTPUT_REVISION 62 (QT 4.7.2)
**   * Q_MOC_OUTPUT_REVISION 63 (QT 4.8.0)
**  OTHER VERSIONS UNTESTED.
**
**  SEE QT_METACALL FOR THE CHANGES IN Q_MOC_OUTPUT_REVISION 62.
**  SEE QT_STATIC_METACALL FOR THE CHANGES IN Q_MOC_OUTPUT_REVISION 63.
*****************************************************************************/

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RpcSignalMapperHelper[] = {

 // content:
#if Q_MOC_OUTPUT_REVISION == 62
       5,       // revision
#elif Q_MOC_OUTPUT_REVISION == 63
       6,       // revision
#endif
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      47,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      89,   22,   22,   22, 0x0a,

       0        // eod
};


static const char qt_meta_stringdata_RpcSignalMapperHelper[] = {
    "RpcSignalMapperHelper\0\0sender,signal,arguments\0"
    "mapped(QObject*,QMetaMethod,QVariantList)\0"
    "map()\0"
};

#if Q_MOC_OUTPUT_REVISION == 63
void RpcSignalMapperHelper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RpcSignalMapperHelper *_t = static_cast<RpcSignalMapperHelper *>(_o);
        switch (_id) {
        case 0: _t->mapped((*reinterpret_cast< QObject*(*)>(_a[1])),(*reinterpret_cast< QMetaMethod(*)>(_a[2])),(*reinterpret_cast< QVariantList(*)>(_a[3]))); break;
        case 1: _t->internalSignalHandler(_a); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RpcSignalMapperHelper::staticMetaObjectExtraData = {
    0,  qt_static_metacall
};
#endif

const QMetaObject RpcSignalMapperHelper::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_RpcSignalMapperHelper,
#if Q_MOC_OUTPUT_REVISION == 62
      qt_meta_data_RpcSignalMapperHelper, 0 }
#elif Q_MOC_OUTPUT_REVISION == 63
      qt_meta_data_RpcSignalMapperHelper, &staticMetaObjectExtraData }
#endif
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RpcSignalMapperHelper::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RpcSignalMapperHelper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RpcSignalMapperHelper::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RpcSignalMapperHelper))
        return static_cast<void*>(const_cast< RpcSignalMapperHelper*>(this));
    return QObject::qt_metacast(_clname);
}

int RpcSignalMapperHelper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
#if Q_MOC_OUTPUT_REVISION == 62
        switch (_id) {
        case 0: mapped((*reinterpret_cast< QObject*(*)>(_a[1])),(*reinterpret_cast< QMetaMethod(*)>(_a[2])),(*reinterpret_cast< QVariantList(*)>(_a[3]))); break;
        case 1: internalSignalHandler(_a); break;
        default: ;
        }
#elif Q_MOC_OUTPUT_REVISION == 63
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
#endif
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void RpcSignalMapperHelper::mapped(QObject * _t1, QMetaMethod _t2, QVariantList _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
