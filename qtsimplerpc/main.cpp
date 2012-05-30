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

#include <QCoreApplication>
#include <QStringList>
#include <QTcpSocket>
#include <QHostAddress>

#include <iostream>
using namespace std;

#include "exampleclass.h"
#include "rpcconnection.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Print usage
    if(a.argc() != 2) {
        cerr << "Usage: " << a.arguments()[0].toStdString() << " <port>" << endl;
        return 1;
    }

    // Open TCP socket
    int port = a.arguments()[1].toInt();
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, port);

    // Create example object
    ExampleClass example;

    // Create RPC layer and tell it to use our TCP socket as the communication device
    RpcConnection rpc;
    rpc.setPeerDevice(&socket);

    // Automatic mapping of incoming commands to slots of object
    rpc.mapAllCommandsToSlots(&example);

    // Automatic mapping of signals of object to outgoing (asynchronous) commands
    rpc.mapAllSignalsToCommands(&example);

    return a.exec();
}
