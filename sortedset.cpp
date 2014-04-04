#include "sortedset.h"


class ScoreLimit
{
public:
    ScoreLimit(QByteArray baInput) :
        bIsClosed(false),
        bSuccess(true)
    {
        // Infinity is just the biggest or smallest number possible, combined with a closed interval.
        if (baInput == "-inf")
        {
            dLimit = std::numeric_limits<qreal>::min();
            bIsClosed = true;
            return;
        }
        else if (baInput == "+inf")
        {
            dLimit = std::numeric_limits<qreal>::max();
            bIsClosed = true;
            return;
        }

        if (baInput.at(0) == '(')
        {
            bIsClosed = true;
            baInput = baInput.mid(1);
        }

        dLimit = baInput.toDouble(&bSuccess);
    }
    qreal dLimit;
    bool bIsClosed;
    bool bSuccess;
};


SortedSet::SortedSet(QObject *parent) :
    QObject(parent)
{
}

/// Triages a command and passes it along to the appropriate execution function.
void SortedSet::slotCommand(QTcpSocket *pSocket, const QByteArray &baCommand, const QByteArray &baCommandUpper, const QList<QByteArray> &listParams)
{
    if (baCommandUpper == "ZADD")
    {
        zAdd(pSocket, listParams);
    }
    else if (baCommandUpper == "ZCARD")
    {
        zCard(pSocket, listParams);
    }
    else if (baCommandUpper == "ZRANGE")
    {
        zRange(pSocket, listParams);
    }
    else if (baCommandUpper == "ZREM")
    {
        zRem(pSocket, listParams);
    }
    else if (baCommandUpper == "ZSCORE")
    {
        zScore(pSocket, listParams);
    }
    else
    {
        LOGDEBUG("Unrecognized command" << baCommand);
        for (int i = 0; i < listParams.length(); i++)
        {
            LOGDEBUG("Arg" << i << listParams[i]);
        }
        pSocket->write(RESP_ERROR "unknown command '" + baCommand + "'" RESP_EOL);
    }


}


/// ZADD key score member [score member ...]
///
/// Adds all the specified members with the specified scores to the sorted set stored at key.
/// It is possible to specify multiple score/member pairs. If a specified member is already
/// a member of the sorted set, the score is updated and the element reinserted at the right
/// position to ensure the correct ordering. If key does not exist, a new sorted set with the
/// specified members as sole members is created, like if the sorted set was empty.
/// If the key exists but does not hold a sorted set, an error is returned.
///
/// The score values should be the string representation of a numeric value, and accepts qreal
/// precision floating point numbers.
void SortedSet::zAdd(QTcpSocket *pSocket, const QList<QByteArray> &listParams)
{
    if (listParams.count() < 3)
    {
        pSocket->write(RESP_ERROR "wrong number of arguments for 'zadd' command" RESP_EOL);
        return;
    }
    else if (listParams.count() % 2 == 0) // # of arguments must be odd.
    {
        pSocket->write(RESP_ERROR "syntax error");
        return;
    }

    QList<QByteArray> listKeys;
    QList<qreal> listScores;
    for (int i = 1; i + 1 < listParams.count(); i += 2)
    {
        bool bOk;
        const qreal dScore = listParams.value(i).toDouble(&bOk);
        if (!bOk)
        {
            pSocket->write(RESP_ERROR "value is not a valid float");
            return;
        }
        listScores << dScore;
        listKeys << listParams.value(i + 1);
    }

    int nAdded = 0;
    QPair<QHash<QByteArray, qreal>, QMultiMap<qreal, QByteArray> > &pairData = m_hashData[listParams.first()];
    for (int i = 0; i < listKeys.count(); i++)
    {
        const QByteArray baKey = listKeys.value(i);
        const qreal dScore = listScores.value(i);

        if (pairData.first.contains(baKey))
        {
            pairData.second.remove(dScore, baKey);
        }
        else
        {
            nAdded++;
        }
        pairData.first.insert(baKey, dScore);
        pairData.second.insert(dScore, baKey);
    }

    pSocket->write(RESP_INTEGER + QByteArray::number(nAdded) + RESP_EOL);
}



/// ZCARD key
///
/// Returns the sorted set cardinality (number of elements) of the sorted set stored at key.
void SortedSet::zCard(QTcpSocket *pSocket, const QList<QByteArray> &listParams)
{
    if (listParams.isEmpty())
    {
        pSocket->write(RESP_ERROR "wrong number of arguments for 'zcard' command" RESP_EOL);
        return;
    }

    QHash<QByteArray, QPair<QHash<QByteArray, qreal>, QMultiMap<qreal, QByteArray> > >::const_iterator it = m_hashData.find(listParams.first());
    if (it == m_hashData.end())
    {
        pSocket->write(RESP_INTEGER "0" RESP_EOL);
    }
    else
    {
        pSocket->write(RESP_INTEGER + QByteArray::number(it->second.count()) + RESP_EOL);
    }
}



/// ZCOUNT key min max
///
/// Returns the number of elements in the sorted set at key with a score between min and max.
void SortedSet::zCount(QTcpSocket *pSocket, const QList<QByteArray> &listParams)
{
    if (listParams.count() != 3)
    {
        pSocket->write(RESP_ERROR "wrong number of arguments for 'zcount' command" RESP_EOL);
        return;
    }

    const ScoreLimit limitMin(listParams.value(1));
    const ScoreLimit limitMax(listParams.value(2));

    if (!limitMax.bSuccess || !limitMin.bSuccess)
    {
        pSocket->write(RESP_ERROR "min or max is not a float" RESP_EOL);
        return;
    }

    int nCount = 0;
    QPair<QHash<QByteArray, qreal>, QMultiMap<qreal, QByteArray> > &pairData = m_hashData[listParams.first()];
    for (QMultiMap<qreal, QByteArray>::iterator it = pairData.second.lowerBound(limitMin.dLimit); it.key() <= limitMax.dLimit; it++)
    {
        // This if statement can be optimized in obvious ways. This handles closed/open intervals.
        if ((!limitMin.bIsClosed && it.key() == limitMin.dLimit) ||
            (!limitMax.bIsClosed && it.key() == limitMax.dLimit))
        {
            continue;
        }

        nCount++;
    }

    pSocket->write(RESP_INTEGER + QByteArray::number(nCount) + RESP_EOL);

}

/// ZRANGE key start stop [WITHSCORES]
///
/// Returns the specified range of elements in the sorted set stored at key. The elements are considered
/// to be ordered from the lowest to the highest score. Lexicographical order is used for elements with equal score.
///
/// It is possible to pass the WITHSCORES option in order to return the scores of the elements together with the elements.
/// The returned list will contain value1,score1,...,valueN,scoreN instead of value1,...,valueN.
void SortedSet::zRange(QTcpSocket *pSocket, const QList<QByteArray> &listParams)
{
    if (listParams.count() < 3 || listParams.count() > 4)
    {
        pSocket->write(RESP_ERROR "wrong number of arguments for 'zrange' command" RESP_EOL);
        return;
    }

    bool bWithScores = false;
    if (listParams.count() == 4)
    {
        if (listParams.value(3).toUpper() != "WITHSCORES")
        {
            pSocket->write(RESP_ERROR "syntax error" RESP_EOL);
            return;
        }
        bWithScores = true;
    }

    QPair<QHash<QByteArray, qreal>, QMultiMap<qreal, QByteArray> > &pairData = m_hashData[listParams.first()];
    bool bOkMin, bOkMax;
    int nMin = listParams.value(1).toInt(&bOkMin);
    int nMax = listParams.value(2).toInt(&bOkMax);
    if (!bOkMin || !bOkMax)
    {
        pSocket->write(RESP_ERROR "value is not an integer or out of range" RESP_EOL);
        return;
    }

    const int nMapCount = pairData.second.count();
    if (nMin < 0)
    {
        nMin = nMapCount + nMin;
    }

    if (nMax < nMapCount)
    {
        nMax = nMapCount + nMax;
    }

    if (nMax < nMin || nMax < 0 || nMin > nMapCount - 1)
    {
        pSocket->write(RESP_EMPTY RESP_EOL);
        return;
    }

    // Note that this is linear time given how QMap works. Might not be too hard to fix that with an
    // additional list or something.
    int nOutputPos = 1;
    QMultiMap<qreal, QByteArray>::iterator it;
    int nPos;
    for (it = pairData.second.begin(), nPos = 0;
         it != pairData.second.end() && nPos <= nMax;
         it++, nPos++)
    {
        // Note: need to sort by value if same key!
        if (nPos >= nMin)
        {
            pSocket->write(QByteArray::number(nOutputPos) + RESP_ARRAY_PAREN RESP_QUOTE + it.value() + RESP_QUOTE + RESP_EOL);
            nOutputPos++;
            if (bWithScores)
            {
                pSocket->write(QByteArray::number(nOutputPos) + RESP_ARRAY_PAREN RESP_QUOTE + QByteArray::number(it.key()) + RESP_QUOTE + RESP_EOL);
                nOutputPos++;
            }
        }
    }

    if (!nOutputPos)
    {
        pSocket->write(RESP_EMPTY RESP_EOL);
        return;
    }

}


/// ZREM key member [member ...]
///
/// Removes the specified members from the sorted set stored at key.
/// Non existing members are ignored. An error is returned when key exists and does not hold a sorted set.
void SortedSet::zRem(QTcpSocket *pSocket, const QList<QByteArray> &listParams)
{
    if (listParams.count() < 2)
    {
        pSocket->write(RESP_ERROR "wrong number of arguments for 'zrem' command" RESP_EOL);
        return;
    }

    int nCount = 0;

    QPair<QHash<QByteArray, qreal>, QMultiMap<qreal, QByteArray> > &pairData = m_hashData[listParams.first()];
    for (int i = 1; i < listParams.count(); i++)
    {
        const QByteArray baKey = listParams.value(i);
        QHash<QByteArray, qreal>::iterator it = pairData.first.find(baKey);
        if (it != pairData.first.end())
        {
            const qreal dScore = it.value();
            pairData.second.remove(dScore, baKey);
            pairData.first.remove(baKey);
            nCount++;
        }
    }

    pSocket->write(RESP_INTEGER + QByteArray::number(nCount) + RESP_EOL);
}


/// ZSCORE key member
///
/// Returns the score of member in the sorted set at key.
/// If member does not exist in the sorted set, or key does not exist, nil is returned.
void SortedSet::zScore(QTcpSocket *pSocket, const QList<QByteArray> &listParams)
{
    if (listParams.count() != 2)
    {
        pSocket->write(RESP_ERROR "wrong number of arguments for 'zscore' command" RESP_EOL);
        return;
    }

    QPair<QHash<QByteArray, qreal>, QMultiMap<qreal, QByteArray> > &pairData = m_hashData[listParams.first()];
    QHash<QByteArray, qreal>::iterator it = pairData.first.find(listParams.value(1));
    if (it != pairData.first.end())
    {
        pSocket->write(RESP_QUOTE + QByteArray::number(it.value()) + RESP_QUOTE RESP_EOL);
    }
    else
    {
        pSocket->write(RESP_NIL);
    }
}

