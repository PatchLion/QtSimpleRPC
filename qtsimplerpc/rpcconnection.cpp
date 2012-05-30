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

#include "rpcconnection.h"
#include <QIODevice>
#include <QDebug>
#include <QtConcurrentRun>
#include "rpccommandmapper.h"
#include "rpcsignalmapper.h"
#include "qjson.h"


#define MESSAGE_DELIM "\n"


RpcConnection::RpcConnection(QObject *parent) :
    QObject(parent),
    device(NULL),
    commandMapper(new RpcCommandMapper(this)),
    signalMapper(new RpcSignalMapper(this))
{
    connect(signalMapper, SIGNAL(mappedCommand(QByteArray,QVariantList)),
            SLOT(remoteCall(QByteArray,QVariantList)));
    connect(signalMapper, SIGNAL(mappedCommandAsync(QByteArray,QVariantList)),
            SLOT(remoteCallAsync(QByteArray,QVariantList)));
}

void RpcConnection::setPeerDevice(QIODevice *peerDevice)
{
    if(device) {
        device->disconnect(this);
    }
    device = peerDevice;
    if(device) {
        connect(device, SIGNAL(readyRead()), SLOT(device_readyRead()));
    }
}

QIODevice *RpcConnection::peerDevice() const
{
    return device;
}

void RpcConnection::mapCommandToSlot(const QByteArray &commandName, QObject *object, const char *member)
{
    commandMapper->addMapping(commandName, object, member);
}

void RpcConnection::mapAllCommandsToSlots(QObject *object)
{
    const QMetaObject *mo = object->metaObject();
    // skip QObject's members
    for(int i = QObject::staticMetaObject.methodCount(); i < mo->methodCount(); ++i)
    {
        if(mo->method(i).methodType() == QMetaMethod::Slot ||
            mo->method(i).methodType() == QMetaMethod::Method)
        {
            //qDebug("Found method: %s", mo->method(i).signature());
            QByteArray memberName = mo->method(i).signature();
            memberName = memberName.left(memberName.indexOf('(')); // remove arguments
            mapCommandToSlot(memberName, object, memberName.constData());
        }
    }
}

void RpcConnection::mapSignalToCommand(QObject *object, const char *signal, const QByteArray &commandName)
{
    signalMapper->addMapping(object, signal, commandName);
}

void RpcConnection::mapAllSignalsToCommands(QObject *object)
{
    const QMetaObject *mo = object->metaObject();
    // skip QObject's members
    for(int i = QObject::staticMetaObject.methodCount(); i < mo->methodCount(); ++i)
    {
        if(mo->method(i).methodType() == QMetaMethod::Signal)
        {
            //qDebug("Found signal: %s", mo->method(i).signature());
            QByteArray commandName = mo->method(i).signature();
            commandName = commandName.left(commandName.indexOf('(')); // remove arguments to get command name
            mapSignalToCommand(object, mo->method(i).signature(), commandName);
        }
    }
}

QVariant RpcConnection::remoteCall(QByteArray command, QVariantList arguments, int *errorCode)
{
    sendCommand(command, arguments);
    QVariant response = waitForResponse(errorCode);
    return response;
}

void RpcConnection::remoteCallAsync(QByteArray command, QVariantList arguments)
{
    sendCommandAsync(command, arguments);
}

void RpcConnection::device_readyRead()
{
    // read data from device into internal buffer
    readBuf += device->readAll();
    int messageEnd = readBuf.indexOf(MESSAGE_DELIM);

    // is there (at least) one message?
    if(messageEnd != -1)
    {
        QByteArray message = readBuf.left(messageEnd);
        processRawMessage(message);

        if(messageEnd == readBuf.length() - 1)
        {
            // there is no more data
            readBuf.clear();
        }
        else
        {
            // there is more data
            readBuf = readBuf.mid(messageEnd + 1);
            // re-call this slot to process more data:
            device_readyRead();
        }
    }
}

QVariant RpcConnection::waitForResponse(int *errorCode)
{
    //This call should set both availableResponse and availableErrorCode
    responseLoop.exec();

    QVariant result = availableResponse;
    availableResponse = QVariant(); // reset
    if(errorCode)
        *errorCode = availableErrorCode;
    return result;
}

void RpcConnection::processRawMessage(QByteArray message)
{
    if(message.length() == 0)
        return;

    // determine message type (command / response?) by first character
    char first = message.at(0);
    if(first >= '0' && first <= '9')
        processRawResponse(message);
    else
        processRawCommand(message);
}

void RpcConnection::processRawCommand(QByteArray rawData)
{
    //qDebug("Command: %s", rawData.constData());

    bool async = false;
    if(rawData.startsWith("async ")) {
        async = true;
        rawData = rawData.mid(6);
    }

    // parse command
    int split = rawData.indexOf(' ');
    if(split == -1) {
        sendResponseParseError(rawData);
        return;
    }
    QByteArray commandName = rawData.left(split).trimmed();
    QByteArray argumentsData = rawData.mid(split + 1).trimmed();

    // parse arguments
    QVariant argumentsVariant = QJson::decode(argumentsData);
    if(argumentsVariant.type() != QVariant::List) {
        sendResponseParseError(rawData);
        return;
    }
    QVariantList arguments = argumentsVariant.toList();

    if(async)
    {
        // run the command concurrently
        QtConcurrent::run(commandMapper, &RpcCommandMapper::runCommand, commandName, arguments);
    }
    else
    {
        // run the command
        RpcCommandMapper::CommandResult result = commandMapper->runCommand(commandName, arguments);

        // proces result
        switch(result.code)
        {
        case(RpcCommandMapper::Successful):
            sendResponseSuccess(result.value);
            break;
        case(RpcCommandMapper::CommandDoesntExistError):
            sendResponseCommandDoesntExistError(commandName);
            break;
        case(RpcCommandMapper::CommandSignatureMismatchError):
            sendResponseCommandSignatureMismatchError(commandName);
            break;
        default:
            qWarning("Error in implementation of RpcCommandMapper::runCommand().");
            break;
        }
    }
}

void RpcConnection::processRawResponse(QByteArray response)
{
    int space = response.indexOf(' ');
    if(space == -1)
        return;

    ErrorCode errorCode = (ErrorCode)response.left(space).toInt();
    QVariant result = QJson::decode(response.mid(space + 1));

    if(!availableResponse.isNull())
        qWarning("Received response, but I didn't send command! Ignoring.");
    else
    {
        availableResponse = result;
        availableErrorCode = errorCode;
        responseLoop.quit();
    }
}

void RpcConnection::sendRawMessage(QByteArray message)
{
    device->write(message);
    emit deviceFlush();
}

void RpcConnection::sendCommand(QByteArray command, QVariantList arguments)
{
    QJson::Error jsonError;
    QString encodedArguments =
        QJson::encode(arguments, QJson::EncodeOptions(QJson::Compact), &jsonError);
    if(!jsonError.isNull())
        qWarning("JSON error: %s", qPrintable(jsonError.text()));
    else
        sendRawMessage(command + " " + encodedArguments.toUtf8() + "\n");
}

void RpcConnection::sendCommandAsync(QByteArray command, QVariantList arguments)
{
    // little hack but this avoids code duplication: prepend "async " to command
    sendCommand("async " + command, arguments);
}

void RpcConnection::sendResponse(ErrorCode errorCode, QVariant data)
{
    QString json = QJson::encode(data, QJson::EncodeOptions(QJson::Compact | QJson::EncodeUnknownTypesAsNull));
    sendRawMessage(QByteArray::number(errorCode) + " " + json.toUtf8() + "\n");
}

void RpcConnection::sendResponseSuccess(QVariant data)
{
    sendResponse(NoError, data);
}

void RpcConnection::sendResponseParseError(QByteArray commandLine)
{
    sendResponse(ParseError, QVariant("Error parsing command: " + commandLine));
}

void RpcConnection::sendResponseCommandDoesntExistError(QByteArray commandName)
{
    sendResponse(SystemError, QVariant("No such command: " + commandName));
}

void RpcConnection::sendResponseCommandSignatureMismatchError(QByteArray commandName)
{
    sendResponse(SystemError, QVariant("Signature mismatch for command " + commandName));
}

