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

#ifndef RPCCONNECTION_H
#define RPCCONNECTION_H

#include <QObject>
#include <QVariantList>
#include <QEventLoop>
#include <QMetaObject>
#include <QMetaMethod>

class QIODevice;
class RpcCommandMapper;
class RpcSignalMapper;


class RpcConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QIODevice* peerDevice READ peerDevice WRITE setPeerDevice)

    enum ErrorCode {
        NoError = 0,
        SystemError = 1,
        ParseError = 2
    };

public:
    explicit RpcConnection(QObject *parent = 0);

public slots:
    void setPeerDevice(QIODevice *peerDevice);
    QIODevice *peerDevice() const;

    //! Maps a single command which can then be called by the reomte end
    void mapCommandToSlot(const QByteArray &commandName, QObject *object, const char *member);
    //! Maps all meta methods of the given object (slots and invokable methods,
    //! except QObject's own members) of the given object as commands
    //! which can then be called by the remote end
    void mapAllCommandsToSlots(QObject *object);

    void mapSignalToCommand(QObject *object, const char *signal, const QByteArray &commandName);
    void mapAllSignalsToCommands(QObject *object);

    //! Call command on the remote end
    QVariant remoteCall(QByteArray command, QVariantList arguments, int *errorCode = 0);
    //! Call command asynchronously on the remote end
    void remoteCallAsync(QByteArray command, QVariantList arguments);

signals:
    void deviceFlush(); //!< Emitted when the device should be flushed when supported (for example QTcpSocket::flush())

private slots:
    void device_readyRead();

private:
    QIODevice *device;
    QByteArray readBuf;
    RpcCommandMapper *commandMapper;
    RpcSignalMapper *signalMapper;
    QEventLoop responseLoop;
    QVariant availableResponse;
    int availableErrorCode;

    QVariant waitForResponse(int *errorCode);

    void processRawMessage(QByteArray message);
    void processRawCommand(QByteArray command);
    void processRawResponse(QByteArray response);

    void sendRawMessage(QByteArray message);
    void sendCommand(QByteArray command, QVariantList arguments);
    void sendCommandAsync(QByteArray command, QVariantList arguments);
    void sendResponse(ErrorCode errorCode, QVariant data);
    void sendResponseSuccess(QVariant data);
    void sendResponseParseError(QByteArray commandLine);
    void sendResponseCommandDoesntExistError(QByteArray commandName);
    void sendResponseCommandSignatureMismatchError(QByteArray commandName);
};

#endif // RPCCONNECTION_H
