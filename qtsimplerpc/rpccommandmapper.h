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

#ifndef RPCCOMMANDMAPPER_H
#define RPCCOMMANDMAPPER_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QMetaMethod>
#include <QSet>

class QMutex;


class RpcCommandMapper : public QObject
{
    // no Q_OBJECT since we don't need meta stuff...

public:
    RpcCommandMapper(QObject *parent);

    enum CommandErrorCode {
        //                             // DATA IN VALUE FIELD:
        //                             ----------------------------------------
        Successful,                    // command return value
        CommandDoesntExistError,       // null
        CommandSignatureMismatchError  // null (in future versions maybe possible signatures)
    };
    struct CommandResult {
        CommandErrorCode code;
        QVariant value;
        inline CommandResult(QVariant value = QVariant()) : code(Successful), value(value) {}
        inline CommandResult(bool value) : code(Successful), value(QVariant(value)) {}
        inline CommandResult(int value) : code(Successful), value(QVariant(value)) {}
        inline CommandResult(long long value) : code(Successful), value(QVariant(value)) {}
        inline CommandResult(double value) : code(Successful), value(QVariant(value)) {}
        inline CommandResult(QString value) : code(Successful), value(QVariant(value)) {}
        inline CommandResult(QByteArray value) : code(Successful), value(QVariant(value)) {}
        inline CommandResult(const char * value) : code(Successful), value(QVariant(value)) {}
        template <typename T> inline CommandResult(QList<T> list) : code(Successful), value(QVariant(packList(list))) {}
        template <typename T> inline CommandResult(QMap<QString,T> map) : code(Successful), value(QVariant(packMap(map))) {}
        template <typename T> inline CommandResult(QList<QList<T> > list) : code(Successful), value(QVariant(packListOfLists(list))) {}
        template <typename T> inline CommandResult(QList<QMap<QString,T> > list) : code(Successful), value(QVariant(packListOfMaps(list))) {}
        template <typename T> inline CommandResult(QMap<QString,QList<T> > map) : code(Successful), value(QVariant(packMapOfLists(map))) {}
        template <typename T> inline CommandResult(QMap<QString,QMap<QString,T> > map) : code(Successful), value(QVariant(packMapOfMaps(map))) {}
        inline CommandResult(CommandErrorCode code, QVariant value) : code(code), value(value) {}
    };

    //! Locally calls a previously mapped command
    CommandResult runCommand(const QByteArray &commandName, const QVariantList &arguments);

    //! Maps a command which can then be called using runCommand(). \arg member can be one of
    //! (a) the member name, (b) the method's signature, (c) the C-string returned by
    //! the SLOT() macro (which will prepend a digit to the signature). Independently from
    //! which type you choose, the parameters are ignored by this function. The appropriate
    //! overload of the member is choosen when the method gets called by runCommand() by
    //! looking at the types of the provided arguments.
    void addMapping(const QByteArray &commandName, QObject *object, const char *member);


protected:
    /** Returns a list of commands (without signature) */
    QList<QByteArray> listOfCommands() const;

private:
    struct ObjectSlot {
        QObject *obj;
        QByteArray memberName;
    };
    QHash<QByteArray, ObjectSlot> mappings;

    //meta type stuff:
    static QVariant variantMetacall(QObject *obj, QMetaMethod method, const QVariantList &arguments);

    static bool checkSignature(QMetaMethod method, const QVariantList &arguments);
    static bool checkTypes(const QByteArray &typeDescription, const QVariant &argument);
    static bool checkList(const QByteArray &elementTypeDescription, const QVariantList &argument);
    static bool checkMap(const QByteArray &elementTypeDescription, const QVariantMap &argument);
    static void* newInstanceOfType(const QByteArray &typeDescription, const QVariant &value);
    static void deleteInstanceOfType(const QByteArray &typeDescription, void *instance);
    static QVariant readInstanceOfType(const QByteArray &typeDescription, void *instance);
    static QByteArray normalizeType(const QByteArray &typeDescription);

    template <typename T> static inline QList<T> extractList(const QVariantList &list);
    template <typename T> static inline QMap<QString,T> extractMap(const QVariantMap &addMapping);
    template <typename T> static inline QList<QList<T> > extractListOfLists(const QVariantList &list);
    template <typename T> static inline QList<QMap<QString,T> > extractListOfMaps(const QVariantList &list);
    template <typename T> static inline QMap<QString,QList<T> > extractMapOfLists(const QVariantMap &addMapping);
    template <typename T> static inline QMap<QString,QMap<QString,T> > extractMapOfMaps(const QVariantMap &addMapping);

    template <typename T> static inline QVariantList packList(const QList<T> &list);
    template <typename T> static inline QVariantMap packMap(const QMap<QString,T> &addMapping);
    template <typename T> static inline QVariantList packListOfLists(const QList<QList<T> > &list);
    template <typename T> static inline QVariantList packListOfMaps(const QList<QMap<QString,T> > &list);
    template <typename T> static inline QVariantMap packMapOfLists(const QMap<QString,QList<T> > &addMapping);
    template <typename T> static inline QVariantMap packMapOfMaps(const QMap<QString,QMap<QString,T> > &addMapping);
};


template <typename T>
QVariantList RpcCommandMapper::packList(const QList<T> &list)
{
    QVariantList result;
    foreach(T entry, list)
        result << QVariant(entry);
    return result;
}

template <typename T>
QVariantMap RpcCommandMapper::packMap(const QMap<QString,T> &map)
{
    QVariantMap result;
    QMapIterator<QString,T> i(map);
    while (i.hasNext()) {
        i.next();
        result.insert(i.key(), QVariant(i.value()));
    }
    return result;
}

template <typename T>
QVariantList RpcCommandMapper::packListOfLists(const QList<QList<T> > &list)
{
    QVariantList result;
    foreach(QList<T> entry, list)
        result << packList(entry);
    return result;
}

template <typename T>
QVariantList RpcCommandMapper::packListOfMaps(const QList<QMap<QString,T> > &list)
{
    QVariantList result;
    typedef QMap<QString,T> Map;
    foreach(Map entry, list)
        result << packMap(entry);
    return result;
}

template <typename T>
QVariantMap RpcCommandMapper::packMapOfLists(const QMap<QString,QList<T> > &map)
{
    QVariantMap result;
    QMapIterator<QString,QList<T> > i(map);
    while (i.hasNext()) {
        i.next();
        result.insert(i.key(), packList(i.value()));
    }
    return result;
}

template <typename T>
QVariantMap RpcCommandMapper::packMapOfMaps(const QMap<QString,QMap<QString,T> > &map)
{
    QVariantMap result;
    QMapIterator<QString,QMap<QString,T> > i(map);
    while (i.hasNext()) {
        i.next();
        result.insert(i.key(), packMap(i.value()));
    }
    return result;
}

#endif // RPCCOMMANDMAPPER_H
