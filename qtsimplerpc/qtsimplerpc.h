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

    template<class QObjectSubclass> static void registerEnumsOfClass() { registerEnumsOfMetaObject(&QObjectSubclass::staticMetaObject); }
    static void registerEnumsOfMetaObject(const QMetaObject *metaObject);

public slots:
    void setPeerDevice(QIODevice *peerDevice);
    QIODevice *peerDevice() const;
    
    void bindObjectAllMembers(QObject *object);
    void bindObjectAllSlotsIncoming(QObject *object);
    void bindObjectAllSignalsOutgoing(QObject *object);

    void bindSlotAsCustomIncomingCommand(QObject *object, const char *member, QByteArray commandName);
    void bindSignalAsCustomOutgoingCommand(QObject *object, const char *signal, QByteArray commandName);

    QVariant remoteCall(QByteArray commandName, QVariantList arguments, int *errorCode = 0);
    void remoteCallAsync(QByteArray commandName, QVariantList arguments);

private:
    RpcConnection *connection;
};

#endif // QTSIMPLERPC_H
