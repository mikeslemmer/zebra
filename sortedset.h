#ifndef SORTEDSET_H
#define SORTEDSET_H

#include <QObject>

class SortedSet : public QObject
{
    Q_OBJECT
public:
    explicit SortedSet(QObject *parent = 0);

signals:

public slots:
    void slotCommand(const QByteArray &baCommand, const QList<QByteArray> &listParams);

};

#endif // SORTEDSET_H
