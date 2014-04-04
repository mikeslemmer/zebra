#ifndef SORTEDSET_H
#define SORTEDSET_H

#include "globals.h"

class SortedSet : public QObject
{
    Q_OBJECT
public:
    explicit SortedSet(QObject *parent = 0);

signals:

public slots:
    void slotCommand(QTcpSocket *pSocket, const QByteArray &baCommand, const QByteArray &baCommandUpper, const QList<QByteArray> &listParams);

private:
    void zAdd(QTcpSocket *pSocket, const QList<QByteArray> &listParams);
    void zCard(QTcpSocket *pSocket, const QList<QByteArray> &listParams);
    void zCount(QTcpSocket *pSocket, const QList<QByteArray> &listParams);
    void zRange(QTcpSocket *pSocket, const QList<QByteArray> &listParams);
    void zRem(QTcpSocket *pSocket, const QList<QByteArray> &listParams);
    void zScore(QTcpSocket *pSocket, const QList<QByteArray> &listParams);


private:
    QHash<QByteArray, QPair<QHash<QByteArray, qreal>, QMultiMap<qreal, QByteArray> > > m_hashData;

};

#endif // SORTEDSET_H
