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

#include "rpccommandmapper.h"
#include <QMetaMethod>
#include <QDebug>
#include <QThread>
#include <QMutex>


RpcCommandMapper::RpcCommandMapper(QObject *parent) :
    QObject(parent)
{
}

RpcCommandMapper::CommandResult RpcCommandMapper::runCommand(const QByteArray &commandName, const QVariantList &arguments)
{
    if(!mappings.contains(commandName))
        return CommandResult(CommandDoesntExistError, QVariant());

    ObjectSlot objectSlot = mappings.value(commandName);
    QObject *obj = objectSlot.obj;
    const QMetaObject *mo = obj->metaObject();
    QByteArray memberName = objectSlot.memberName;

    qDebug("Found mapping: %s => %s::%s(...)", commandName.constData(),
           mo->className(), memberName.constData());

    // Search for meta methods with given name (no signature check here!)
    QList<QMetaMethod> methods;
    for(int i = 0; i < mo->methodCount(); ++i)
        if(QByteArray(mo->method(i).signature()).startsWith(memberName + "(")) // ignore parameters
            methods << mo->method(i);

    //if no method with this name exist, the command doesn't exist
    if(methods.count() == 0)
        return CommandResult(CommandDoesntExistError, QVariant());

    //we first check if the argument count can match one of the signatures, because this is fast:
    for(int i = 0; i < methods.count(); ++i) {
        qDebug() << methods.at(i).signature();
        if(methods.at(i).parameterTypes().count() != arguments.count())
            methods.removeAt(i--);
    }

    //if no method is left, none can match the signatures available
    if(methods.count() == 0)
        return CommandResult(CommandSignatureMismatchError, QVariant());

    //in the next step, we compare the signatures of the remaining methods with the types in the argument list
    QMetaMethod matchingMethod;
    foreach(QMetaMethod method, methods)
    {
        if(checkSignature(method, arguments))
        {
            //enclosingMetaObject is non-NULL if matchingMethod has been set
            if(matchingMethod.enclosingMetaObject())
            {
                qWarning("Multiple argument type matches found for command \"%s\". Treating as signature mismatch.", commandName.constData());
                return CommandResult(CommandSignatureMismatchError, QVariant());
            }
            matchingMethod = method;
        }
    }

    //enclosingMetaObject is non-NULL if matchingMethod has been set
    if(!matchingMethod.enclosingMetaObject())
        return CommandResult(CommandSignatureMismatchError, QVariant());

    return variantMetacall(obj, matchingMethod, arguments);
}

void RpcCommandMapper::addMapping(const QByteArray &commandName, QObject *object, const char *member)
{
    // remove leading digit of SLOT() / METHOD() macro and arguments if present
    QByteArray memberName = (member[0] == '0' || member[0] == '1')
            ? QByteArray(member + 1)
            : QByteArray(member);
    if(memberName.indexOf('(') != -1)
        memberName = memberName.left(memberName.indexOf('('));

    ObjectSlot slot;
    slot.obj = object;
    slot.memberName = memberName;
    mappings.insertMulti(commandName, slot);
}

bool RpcCommandMapper::checkSignature(QMetaMethod method, const QVariantList &arguments)
{
    //This has been checked before...
    Q_ASSERT(method.parameterTypes().count() == arguments.count());

    for(int i = 0; i < arguments.count(); ++i)
    {
        qDebug("checking argument: %s", method.signature());
        QByteArray type = method.parameterTypes().at(i);
        qDebug() << "against provided type:" << type;
        const QVariant & arg = arguments.at(i);
        if(!checkTypes(type, arg))
            return false;
    }
    return true;
}

bool RpcCommandMapper::checkTypes(const QByteArray &typeDescription, const QVariant &argument)
{
    QByteArray type = normalizeType(typeDescription);
    qDebug("checking type (normalized): \"%s\" vs \"%s\"", type.constData(), argument.typeName());

    if (type == "QVariant")
    {
        return true;
    }
    else if (type.startsWith("QList<"))
    {
        QByteArray entryType = type.mid(6, type.length() - 7).trimmed(); //remove "QList<" and ">"
        if(argument.type() != QVariant::List)
            return false;
        else
            return checkList(entryType, argument.toList());
    }
    else if (type.startsWith("QMap<QString,"))
    {
        QByteArray entryType = type.mid(13, type.length() - 14).trimmed(); //remove "QMap<QString," and ">"
        if(argument.type() != QVariant::Map)
            return false;
        else
            return checkMap(entryType, argument.toMap());
    }
    else if (type == "QString")
    {
        return (argument.type() == QVariant::String);
    }
    else if (type == "qlonglong")
    {
        return (argument.type() == QVariant::LongLong);
    }
    else if (type == "bool")
    {
        return (argument.type() == QVariant::Bool);
    }
    return false;
}

bool RpcCommandMapper::checkList(const QByteArray &elementTypeDescription, const QVariantList &argument)
{
    foreach(QVariant entry, argument)
        if(!checkTypes(elementTypeDescription, entry))
            return false;
    return true;
}

bool RpcCommandMapper::checkMap(const QByteArray &elementTypeDescription, const QVariantMap &argument)
{
    foreach(QVariant entry, argument)
        if(!checkTypes(elementTypeDescription, entry))
            return false;
    return true;
}

void * RpcCommandMapper::newInstanceOfType(const QByteArray &typeDescription, const QVariant &value)
{

#define CHECK_TYPE(typeName) \
    else if (typeDescription == #typeName) \
        return (void*) new typeName(value.value<typeName >()); \
    else if (typeDescription == "QList<"#typeName">") \
        return (void*) new QList<typeName >(extractList<typeName >(value.toList())); \
    else if (typeDescription == "QMap<QString,"#typeName">") \
        return (void*) new QMap<QString,typeName >(extractMap<typeName >(value.toMap())); \
    else if (typeDescription == "QList<QList<"#typeName"> >") \
        return (void*) new QList<QList<typeName > >(extractListOfLists<typeName >(value.toList())); \
    else if (typeDescription == "QList<QMap<QString,"#typeName"> >") \
        return (void*) new QList<QMap<QString,typeName > >(extractListOfMaps<typeName >(value.toList())); \
    else if (typeDescription == "QMap<QString,QList<"#typeName"> >") \
        return (void*) new QMap<QString,QList<typeName > >(extractMapOfLists<typeName >(value.toMap())); \
    else if (typeDescription == "QMap<QString,QMap<QString,"#typeName"> >") \
        return (void*) new QMap<QString,QMap<QString,typeName > >(extractMapOfMaps<typeName >(value.toMap()))

    int type = QMetaType::type(normalizeType(typeDescription).constData());
    if (type) {
        if (value.canConvert((QVariant::Type)type)) {
            QVariant converted = value;
            converted.convert((QVariant::Type)type);
            return QMetaType::construct(type, converted.constData());
        } else {
            return QMetaType::construct(type);
        }
    }
    CHECK_TYPE(QVariant);
    CHECK_TYPE(QVariantList);
    CHECK_TYPE(QVariantMap);
    CHECK_TYPE(QByteArray);
    CHECK_TYPE(QString);
    CHECK_TYPE(int);
    CHECK_TYPE(long long);
    CHECK_TYPE(bool);
    else {
        qWarning("Failed to instanciate an argument of type %s", typeDescription.constData());
        return 0;
    }

#undef CHECK_TYPE
}

void RpcCommandMapper::deleteInstanceOfType(const QByteArray &typeDescription, void *instance)
{
    if (!instance)
        return;

#define CHECK_TYPE(typeName) \
    else if (typeDescription == #typeName) \
        delete (typeName*)instance; \
    else if (typeDescription == "QList<"#typeName">") \
        delete (QList<typeName >*)instance; \
    else if (typeDescription == "QMap<QString,"#typeName">") \
        delete (QMap<QString,typeName >*)instance; \
    else if (typeDescription == "QList<QList<"#typeName">>") \
        delete (QList<QList<typeName > >*)instance; \
    else if (typeDescription == "QList<QMap<QString,"#typeName"> >") \
        delete (QList<QMap<QString,typeName > >*)instance; \
    else if (typeDescription == "QMap<QString,QList<"#typeName">") \
        delete (QMap<QString,QList<typeName > >*)instance; \
    else if (typeDescription == "QMap<QString,QMap<QString,"#typeName">") \
        delete (QMap<QString,QMap<QString,typeName > >*)instance

    int type = QMetaType::type(normalizeType(typeDescription).constData());
    if (type)
        QMetaType::destroy(type, instance);
    CHECK_TYPE(QVariant);
    CHECK_TYPE(QVariantList);
    CHECK_TYPE(QVariantMap);
    CHECK_TYPE(QByteArray);
    CHECK_TYPE(QString);
    CHECK_TYPE(int);
    CHECK_TYPE(long long);
    CHECK_TYPE(bool);
    else
        qWarning("Failed to delete an argument of type %s", typeDescription.constData());

#undef CHECK_TYPE
}

QVariant RpcCommandMapper::readInstanceOfType(const QByteArray &typeDescription, void *instance)
{
    if (!instance)
        return QVariant();

#define CHECK_TYPE(typeName) \
    else if (typeDescription == #typeName) \
        return QVariant(*(typeName*)instance); \
    else if (typeDescription == "QList<"#typeName">") \
        return packList<typeName>(*(QList<typeName >*)instance); \
    else if (typeDescription == "QMap<QString,"#typeName">") \
        return packMap<typeName>(*(QMap<QString,typeName >*)instance); \
    else if (typeDescription == "QList<QList<"#typeName">>") \
        return packListOfLists<typeName>(*(QList<QList<typeName > >*)instance); \
    else if (typeDescription == "QList<QMap<QString,"#typeName"> >") \
        return packListOfMaps<typeName>(*(QList<QMap<QString,typeName > >*)instance); \
    else if (typeDescription == "QMap<QString,QList<"#typeName">") \
        return packMapOfLists<typeName>(*(QMap<QString,QList<typeName > >*)instance); \
    else if (typeDescription == "QMap<QString,QMap<QString,"#typeName">") \
        return packMapOfMaps<typeName>(*(QMap<QString,QMap<QString,typeName > >*)instance)

    int type = QMetaType::type(normalizeType(typeDescription).constData());
    if (type)
        return QVariant(type, instance);
    CHECK_TYPE(QVariant);
    CHECK_TYPE(QVariantList);
    CHECK_TYPE(QVariantMap);
    CHECK_TYPE(QByteArray);
    CHECK_TYPE(QString);
    CHECK_TYPE(int);
    CHECK_TYPE(long long);
    CHECK_TYPE(bool);
    else {
        qWarning("Failed to read return value of type %s", typeDescription.constData());
        return QVariant();
    }

#undef CHECK_TYPE
}

template <typename T>
QList<T> RpcCommandMapper::extractList(const QVariantList &list)
{
    QList<T> result;
    foreach(QVariant entry, list)
        result << entry.value<T>();
    return result;
}

template <typename T>
QMap<QString,T> RpcCommandMapper::extractMap(const QVariantMap &map)
{
    QMap<QString,T> result;
    QMapIterator<QString,QVariant> i(map);
    while (i.hasNext()) {
        i.next();
        result.insert(i.key(), i.value().value<T>());
    }
    return result;
}

template <typename T>
QList<QList<T> > RpcCommandMapper::extractListOfLists(const QVariantList &list)
{
    QList<QList<T> > result;
    foreach(QVariant entry, list)
        result << extractList<T>(entry.toList());
    return result;
}

template <typename T>
QList<QMap<QString,T> > RpcCommandMapper::extractListOfMaps(const QVariantList &list)
{
    QList<QMap<QString,T> > result;
    foreach(QVariant entry, list)
        result << extractMap<T>(entry.toMap());
    return result;
}

template <typename T>
QMap<QString,QList<T> > RpcCommandMapper::extractMapOfLists(const QVariantMap &map)
{
    QMap<QString,QList<T> > result;
    QMapIterator<QString,QVariant> i(map);
    while (i.hasNext()) {
        i.next();
        result.insert(i.key(), extractList<T>(i.value().toList()));
    }
    return result;
}

template <typename T>
QMap<QString,QMap<QString,T> > RpcCommandMapper::extractMapOfMaps(const QVariantMap &map)
{
    QMap<QString,QMap<QString,T> > result;
    QMapIterator<QString,QVariant> i(map);
    while (i.hasNext()) {
        i.next();
        result.insert(i.key(), extractMap<T>(i.value().toMap()));
    }
    return result;
}


QByteArray RpcCommandMapper::normalizeType(const QByteArray &typeDescription)
{
    if (typeDescription == "QVariantList")
    {
        return "QList<QVariant>";
    }
    else if (typeDescription == "QVariantMap")
    {
        return "QMap<QString,QVariant>";
    }
    else if (typeDescription == "QString" || typeDescription == "QByteArray")
    {
        return "QString";
    }
    else if (typeDescription == "int" ||
             typeDescription == "long long")
    {
        return "qlonglong";
    }
    return typeDescription.trimmed();
}

QList<QByteArray> RpcCommandMapper::listOfCommands() const
{
    QList<QByteArray> commands = mappings.keys();
    qSort(commands);
    return commands;
}

QVariant RpcCommandMapper::variantMetacall(QObject *obj, QMetaMethod method, const QVariantList &arguments)
{
    //prepare qt_metacall arguments
    void** metacallArgs = (void**) malloc(sizeof(void*) * (1 + arguments.count()));
    QByteArray returnTypeName = QByteArray(method.typeName());
    metacallArgs[0] = newInstanceOfType(returnTypeName, QVariant());
    for(int i = 0; i < arguments.count(); ++i)
        metacallArgs[i+1] = newInstanceOfType(method.parameterTypes().at(i), arguments.at(i));

    //perform qt_metacall
    obj->qt_metacall(QMetaObject::InvokeMetaMethod, method.methodIndex(), metacallArgs);
    QVariant returnVal = readInstanceOfType(returnTypeName, metacallArgs[0]);

    //cleanup qt_metacall arguments
    deleteInstanceOfType(returnTypeName, metacallArgs[0]);
    for(int i = 0; i < arguments.count(); ++i)
        deleteInstanceOfType(method.parameterTypes().at(i), metacallArgs[i+1]);

    return returnVal;
}
