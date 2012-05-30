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

#include "rpcsignalmapper.h"
#include "rpcsignalmapperhelper.h"

RpcSignalMapper::RpcSignalMapper(QObject *parent) :
    QObject(parent)
{
}

void RpcSignalMapper::addMapping(QObject *object, const char *signal, const QByteArray &commandName, bool async)
{
    // remove leading digit of SIGNAL() macro
    QByteArray signalSignature = (signal[0] == '2')
            ? QByteArray(signal + 1)
            : QByteArray(signal);

    const QMetaObject *mo = object->metaObject();
    QMetaMethod method;

    for(int i = QObject::staticMetaObject.methodCount(); i < mo->methodCount(); ++i)
        if(signalSignature == mo->method(i).signature())
            method = mo->method(i);

    if(!method.enclosingMetaObject())
        qWarning("Can't map signal \"%s\": signal not found in class %s.",
                 signalSignature.constData(), mo->className());
    else
    {
        RpcSignalMapperHelper *helper = new RpcSignalMapperHelper(method, this);

        // connect object to helper
        connect(object, QByteArray("2") + signalSignature,
                helper, SLOT(map()));
        // connect helper to my handler
        connect(helper, SIGNAL(mapped(QObject*,QMetaMethod,QVariantList)),
                this,   SLOT(signalHandler(QObject*,QMetaMethod,QVariantList)));

        ObjectSignal objectSignal;
        objectSignal.obj = object;
        objectSignal.signalIndex = method.methodIndex();

        MappedCommand command;
        command.commandName = commandName;
        command.async = async;

        mappings.insert(objectSignal, command);
    }
}

void RpcSignalMapper::signalHandler(QObject *sender, QMetaMethod signal, QVariantList arguments)
{
    ObjectSignal objectSignal;
    objectSignal.obj = sender;
    objectSignal.signalIndex = signal.methodIndex();

    if(mappings.contains(objectSignal))
    {
        MappedCommand command = mappings.value(objectSignal);

        if(command.async)
            emit mappedCommandAsync(command.commandName, arguments);
        else
            emit mappedCommand(command.commandName, arguments);
    }
    else
        qWarning("Received signal but couldn't find correct mapping!");
}
