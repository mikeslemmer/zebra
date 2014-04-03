#ifndef NETCONNECTION_H
#define NETCONNECTION_H

#include "globals.h"

class NetServer : public QObject
{
    Q_OBJECT
public:
    explicit NetServer(QObject *parent = 0);

signals:
    void signalSortedSet(const QByteArray &baCommand, const QList<QByteArray> &listParams);

protected slots:
    void slotNewConnection();
    void slotSocketReadyRead();
    void slotSocketDisconnected();

private:
    QList<QByteArray> splitParams(const QByteArray &baParams);

private:
    QTcpServer *m_pServer;
    QHash<QTcpSocket *, QByteArray> m_hashSocketBuffer;
    QList<QTcpSocket *> m_listSocketsToClose;
};

#endif // NETCONNECTION_H
