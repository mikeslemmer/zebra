#ifndef ZEBRAAPP_H
#define ZEBRAAPP_H

#include "globals.h"

class NetServer;
class SortedSet;
class ZebraApp : public QCoreApplication
{
    Q_OBJECT
public:
    explicit ZebraApp(int &argc, char **argv);

signals:

public slots:

private:
    NetServer *m_pServer;
    SortedSet *m_pSortedSet;

};

#endif // ZEBRAAPP_H
