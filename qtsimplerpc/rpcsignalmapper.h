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

#ifndef RPCSIGNALMAPPER_H
#define RPCSIGNALMAPPER_H

#include <QObject>
#include <QVariantList>
#include <QMetaMethod>

class RpcSignalMapper : public QObject
{
    Q_OBJECT

public:
    explicit RpcSignalMapper(QObject *parent = 0);

signals:
    void mappedCommand(QByteArray commandName, QVariantList arguments);
    void mappedCommandAsync(QByteArray commandName, QVariantList arguments);

public:
    void addMapping(QObject *object, const char *signal, const QByteArray &commandName, bool async = true);

private slots:
    void signalHandler(QObject *sender, QMetaMethod signal, QVariantList arguments);

private:
    struct ObjectSignal {
        QObject *obj;
        int signalIndex;
        bool operator<(const ObjectSignal &o) const {
            return obj < o.obj || (obj == o.obj && signalIndex < o.signalIndex);
        }
    };
    struct MappedCommand {
        QByteArray commandName;
        bool async;
    };
    QMap<ObjectSignal, MappedCommand> mappings;
};


#endif // RPCSIGNALMAPPER_H
