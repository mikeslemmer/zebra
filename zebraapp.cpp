#include "zebraapp.h"
#include "netserver.h"
#include "sortedset.h"

ZebraApp::ZebraApp(int &argc, char **argv) :
    QCoreApplication(argc, argv)
{
    m_pServer = new NetServer(this);
    m_pSortedSet = new SortedSet(this);

    connect(m_pServer, SIGNAL(signalSortedSet(const QByteArray &, const QList<QByteArray> &)), m_pSortedSet, SLOT(slotCommand(const QByteArray &, const QList<QByteArray> &)));

}
