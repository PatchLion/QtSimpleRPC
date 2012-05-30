#ifndef QTSIMPLERPC_H
#define QTSIMPLERPC_H

#include <qtsimplerpc_global.h>

#include <QObject>
#include <QIODevice>
#include <QVariantList>

class RpcConnection;

class QTSIMPLERPC_EXPORT QtSimpleRpc : public QObject
{
    Q_OBJECT

public:
    explicit QtSimpleRpc(QObject *parent = 0);

public slots:
    void setPeerDevice(QIODevice *peerDevice);
    QIODevice *peerDevice() const;
    
    void registerObjectAllMembers(QObject *object);
    void registerObjectAllSlotsIncoming(QObject *object);
    void registerObjectAllSignalsOutgoing(QObject *object);

    void registerSlotAsCustomIncomingCommand(QObject *object, const char *member, QByteArray commandName);
    void registerSignalAsCustomOutgoingCommand(QObject *object, const char *signal, QByteArray commandName);

    QVariant remoteCall(QByteArray commandName, QVariantList arguments, int *errorCode = 0);
    void remoteCallAsync(QByteArray commandName, QVariantList arguments);

private:
    RpcConnection *connection;
};

#endif // QTSIMPLERPC_H
