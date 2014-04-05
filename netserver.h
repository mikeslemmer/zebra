#ifndef NETCONNECTION_H
#define NETCONNECTION_H

#include "globals.h"

class NetServer : public QObject
{
    Q_OBJECT
public:
    explicit NetServer(QObject *parent = 0);

signals:
    void signalSortedSet(QTcpSocket *, const QByteArray &, const QByteArray &, const QList<QByteArray> &);

protected slots:
    void slotNewConnection();
    void slotSocketReadyRead();
    void slotSocketDisconnected();

private:

//    void processTextInput(QByteArray &baInput);
//    QList<QByteArray> splitParams(const QByteArray &baParams, bool &bOk);

private:
    class SocketData
    {
    public:
        SocketData() :
            nNumParams(0),
            nCurrParamLength(-1) {}

        QByteArray baSocketBuffer;
        QList<QByteArray> listParams;
        int nNumParams;
        int nCurrParamLength;
    };

    QTcpServer *m_pServer;
    QHash<QTcpSocket *, SocketData> m_hashSocketData;
    QList<QTcpSocket *> m_listSocketsToClose;
};

#endif // NETCONNECTION_H
