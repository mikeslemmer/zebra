#include "netserver.h"

NetServer::NetServer(QObject *parent) :
    QObject(parent)
{
    m_pServer = new QTcpServer(this);
    m_pServer->listen(QHostAddress::Any, SERVER_PORT);

    connect(m_pServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    LOGDEBUG("Server listening on port" << SERVER_PORT);
}


/// Handler for a new connection coming in
void NetServer::slotNewConnection()
{
    LOGDEBUG("Got a new connection");
    while(m_pServer->hasPendingConnections())
    {
        QTcpSocket * const pSocket = m_pServer->nextPendingConnection();
        m_hashSocketData.insert(pSocket, SocketData());
        connect(pSocket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
        connect(pSocket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyRead()));
    }
}


/// Called when data is available on the socket
void NetServer::slotSocketReadyRead()
{
    QTcpSocket *pSocket = qobject_cast<QTcpSocket *>(sender());
    SocketData &socketData = m_hashSocketData[pSocket];
    QByteArray &baInput = socketData.baSocketBuffer;
    baInput += pSocket->readAll();


    while (true)
    {
        if (socketData.nNumParams == 0)
        {
            // Waiting for an EOL to do anything. If no EOL is found, just wait for more input.
            const int nEOLPos = baInput.indexOf(RESP_EOL);
            if (nEOLPos < 0)
            {
                break;
            }

            // Grab the relevant piece out of the input.
            const QByteArray baData = baInput.left(nEOLPos);
            baInput = baInput.mid(nEOLPos + strlen(RESP_EOL));

            // "*0" is technically legal. Ignore it and move to the next input.
            if (baData.length() == 0)
            {
                LOGDEBUG("Length 0 input received")
                continue;
            }

            // If it doesn't start with a "*" it better be a "QUIT" (at least for now).
            if (baData.at(0) != RESP_ARRAY)
            {
                if (baData.toUpper() == CMD_QUIT)
                {
                    // This is where you'd put the ability to make
                    // direct commands.
                    LOGDEBUG("Closing socket");
                    pSocket->close();
                    return;
                }
                else
                {
                    pSocket->write(RESP_ERROR "unknown command '" + baInput + "'" RESP_EOL);
                    continue;
                }
            }

            // Convert the input to a number.
            bool bOk;
            socketData.nNumParams = baData.mid(1).toInt(&bOk);
            if (!bOk || socketData.nNumParams < 0)
            {
                pSocket->write(RESP_ERROR "Protocol error: invalid multibulk length" RESP_EOL);
                pSocket->close();
                return;
            }

//            LOGDEBUG("Num params:" << socketData.nNumParams);
        }
        else
        {
            if (socketData.nCurrParamLength == -1)
            {
                // Waiting for an EOL to do anything. If no EOL is found, just wait for more input.
                const int nEOLPos = baInput.indexOf(RESP_EOL);
                if (nEOLPos < 0)
                {
                    break;
                }

                // Grab the relevant piece out of the input.
                const QByteArray baData = baInput.left(nEOLPos);
                baInput = baInput.mid(nEOLPos + 2);

                // If it doesn't start with a "$" it's not going to work.
                if (baData.at(0) != RESP_STRING)
                {
                    pSocket->write(RESP_ERROR + QByteArray("Protocol error: expected '$', got '") + baData.at(0) + "'" RESP_EOL);
                    pSocket->close();
                    return;
                }

                // Convert the input to a number.
                bool bOk;
                socketData.nCurrParamLength = baData.mid(1).toInt(&bOk);
                if (!bOk || socketData.nCurrParamLength < -1)
                {
                    pSocket->write(RESP_ERROR "Protocol error: invalid multibulk length" RESP_EOL);
                    pSocket->close();
                    return;
                }
                else if (socketData.nCurrParamLength == -1) // This means "null"
                {
                    socketData.listParams.append(QByteArray());
                }


//                LOGDEBUG("Param length:" << socketData.nCurrParamLength);
                continue;
            }
            else
            {
                const int nInputLen = baInput.length();

                // If there's not enough input, just wait for more input
                if (nInputLen < socketData.nCurrParamLength + (int)strlen(RESP_EOL))
                {
                    break;
                }
                else
                {
                    socketData.listParams.append(baInput.left(socketData.nCurrParamLength));
                    baInput = baInput.mid(socketData.nCurrParamLength + strlen(RESP_EOL));
                    socketData.nCurrParamLength = -1;
                    LOGDEBUG("Received string:" << socketData.listParams.last());
                    // Fall through
                }
            }

            // If we have all the data we need, go ahead and send off the command.
            if (socketData.nCurrParamLength == -1 && socketData.listParams.count() == socketData.nNumParams)
            {
                const QByteArray baCommand = socketData.listParams.takeFirst();
                const QByteArray baCommandUpper = baCommand.toUpper();

                switch(baCommandUpper.at(0))
                {
                    case 'Z':
                        {
                            emit signalSortedSet(pSocket, baCommand, baCommandUpper, socketData.listParams);
                        }
                        break;

                    default:
                        {
                            LOGDEBUG("Unrecognized command" << baCommand);
                            for (int i = 0; i < socketData.listParams.length(); i++)
                            {
                                LOGDEBUG("Arg" << i << socketData.listParams[i]);
                            }
                            pSocket->write(RESP_ERROR "unknown command '" + baCommand + "'" RESP_EOL);
                        }
                        break;
                }


                // Reset the list and required param count.
                socketData.listParams = QList<QByteArray>();
                socketData.nNumParams = 0;
            }

        }
    }
}


/// A socket disconnected (either because they hung up or because we told them to go away)
/// Remove it from the hash and from memory.
void NetServer::slotSocketDisconnected()
{
    LOGDEBUG("Socket disconnected");
    QTcpSocket *pSocket = qobject_cast<QTcpSocket *>(sender());
    m_hashSocketData.remove(pSocket);
    pSocket->deleteLater();
}




/*


/// Handles textual input mode.
void NetServer::processTextInput(QByteArray &baInput)
{
    int nIdx;
    while (baInput.at(0) != '*' && (nIdx = baInput.indexOf("\n")) >= 0)
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


*/
