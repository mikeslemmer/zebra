#include "netserver.h"

NetServer::NetServer(QObject *parent) :
    QObject(parent)
{
    m_pServer = new QTcpServer(this);
    m_pServer->listen(QHostAddress::Any, SERVER_PORT);

    connect(m_pServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    LOGDEBUG("Server listening on port" << SERVER_PORT);
}


void NetServer::slotNewConnection()
{
    LOGDEBUG("Got a new connection");
    while(m_pServer->hasPendingConnections())
    {
        QTcpSocket * const pSocket = m_pServer->nextPendingConnection();
        m_hashSocketBuffer.insert(pSocket, QByteArray());
        connect(pSocket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
        connect(pSocket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyRead()));
    }
}


void NetServer::slotSocketReadyRead()
{
    QTcpSocket *pSocket = qobject_cast<QTcpSocket *>(sender());
    QByteArray &baInput = m_hashSocketBuffer[pSocket];
    baInput += pSocket->readAll();

    int nIdx;
    while ((nIdx = baInput.indexOf("\n")) >= 0)
    {
        const QByteArray baLine = baInput.left(nIdx).trimmed();
        baInput = baInput.mid(nIdx + 1);

        LOGDEBUG("Line received:" << baLine);

        const int nFirstSpace = baLine.indexOf(" ");
        QByteArray baCommand;
        QByteArray baParams;
        if (nFirstSpace >= 0)
        {
            baCommand = baLine.left(nFirstSpace);
            baParams = baLine.mid(nFirstSpace + 1).trimmed();
        }
        else
        {
            baCommand = baLine;
        }
        const QByteArray baCommandUpper = baCommand.toUpper();

        if (baCommandUpper == CMD_QUIT)
        {
            LOGDEBUG("Closing socket");
            pSocket->close();
            return;
        }

        bool bOk;
        const QList<QByteArray> listParams = splitParams(baParams, bOk);

        if (!bOk)
        {
            pSocket->write("Invalid argument(s)" RESP_EOL);
            continue;
        }

        switch(baCommandUpper.at(0))
        {
            case 'Z':
                {
                    emit signalSortedSet(pSocket, baCommand, baCommandUpper, listParams);
                }
                break;

            default:
                {
                    LOGDEBUG("Unrecognized command" << baCommand);
                    for (int i = 0; i < listParams.length(); i++)
                    {
                        LOGDEBUG("Arg" << i << listParams[i]);
                    }
                    pSocket->write(RESP_ERROR "unknown command '" + baCommand + "'" RESP_EOL);
                }
                break;
        }

    }
}


/// A socket disconnected (either because they hung up or because we told them to go away)
/// Remove it from the hash and from memory.
void NetServer::slotSocketDisconnected()
{
    LOGDEBUG("Socket disconnected");
    QTcpSocket *pSocket = qobject_cast<QTcpSocket *>(sender());
    m_hashSocketBuffer.remove(pSocket);
    pSocket->deleteLater();
}



/// Note that this doesn't work exactly the way Redis does.
QList<QByteArray> NetServer::splitParams(const QByteArray &baParams, bool &bOk)
{
    QList<QByteArray> listParams;

    QByteArray baParam;
    bool inQuote = false;
    for (int i = 0; i < baParams.length(); i++)
    {
        // Ignore multiple spaces
        if (baParam.isEmpty() && baParams[i] == ' ')
        {
            continue;
        }

        // Handle escaped quotes.
        if (baParams[i] == '\\' && baParams[i+1] == '\"')
        {
            baParam += '\"';
            i++;
            continue;
        }

        // Handle escaped backslashes.
        if (baParams[i] == '\\' && baParams[i+1] == '\\')
        {
            baParam += '\\';
            i++;
            continue;
        }

        // Handle actual quotes.
        if (baParams[i] == '\"')
        {
            inQuote = !inQuote;
            continue;
        }

        if (baParams[i] == ' ' && !inQuote)
        {
            listParams.append(baParam);
            baParam = QByteArray();
            continue;
        }
        else
        {
            baParam += baParams[i];
        }
    }

    if (!baParam.isEmpty())
    {
        listParams.append(baParam);
    }

    bOk = !inQuote;

    return listParams;

}
