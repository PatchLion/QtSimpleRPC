#include "qtsimplerpc.h"
#include "rpcconnection.h"


QtSimpleRpc::QtSimpleRpc(QObject *parent) :
    QObject(parent),
    connection(new RpcConnection(this))
{
}

void QtSimpleRpc::registerEnumsOfMetaObject(const QMetaObject *metaObject)
{
    RpcConnection::registerEnums(metaObject);
}

void QtSimpleRpc::setPeerDevice(QIODevice *peerDevice)
{
    connection->setPeerDevice(peerDevice);
}

QIODevice *QtSimpleRpc::peerDevice() const
{
    return connection->peerDevice();
}

void QtSimpleRpc::bindObjectAllMembers(QObject *object)
{
    connection->mapAllCommandsToSlots(object);
    connection->mapAllSignalsToCommands(object);
}

void QtSimpleRpc::bindObjectAllSlotsIncoming(QObject *object)
{
    connection->mapAllCommandsToSlots(object);
}

void QtSimpleRpc::bindObjectAllSignalsOutgoing(QObject *object)
{
    connection->mapAllSignalsToCommands(object);
}

void QtSimpleRpc::bindSlotAsCustomIncomingCommand(QObject *object, const char *member, QByteArray commandName)
{
    connection->mapCommandToSlot(commandName, object, member);
}

void QtSimpleRpc::bindSignalAsCustomOutgoingCommand(QObject *object, const char *signal, QByteArray commandName)
{
    connection->mapSignalToCommand(object, signal, commandName);
}

QVariant QtSimpleRpc::remoteCall(QByteArray commandName, QVariantList arguments, int *errorCode)
{
    return connection->remoteCall(commandName, arguments, errorCode);
}

void QtSimpleRpc::remoteCallAsync(QByteArray commandName, QVariantList arguments)
{
    connection->remoteCallAsync(commandName, arguments);
}

